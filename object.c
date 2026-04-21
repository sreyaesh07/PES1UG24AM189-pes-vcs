// object.c — Content-addressable object store
//
// Every piece of data (file contents, directory listings, commits) is stored
// as an "object" named by its SHA-256 hash. Objects are stored under
// .pes/objects/XX/YYYYYY... where XX is the first two hex characters of the
// hash (directory sharding).
//
// PROVIDED functions: compute_hash, object_path, object_exists, hash_to_hex, hex_to_hash
// TODO functions:     object_write, object_read

#include "pes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/evp.h>

// ─── PROVIDED ────────────────────────────────────────────────────────────────

void hash_to_hex(const ObjectID *id, char *hex_out) {
    for (int i = 0; i < HASH_SIZE; i++) {
        sprintf(hex_out + i * 2, "%02x", id->hash[i]);
    }
    hex_out[HASH_HEX_SIZE] = '\0';
}

int hex_to_hash(const char *hex, ObjectID *id_out) {
    if (strlen(hex) < HASH_HEX_SIZE) return -1;
    for (int i = 0; i < HASH_SIZE; i++) {
        unsigned int byte;
        if (sscanf(hex + i * 2, "%2x", &byte) != 1) return -1;
        id_out->hash[i] = (uint8_t)byte;
    }
    return 0;
}

void compute_hash(const void *data, size_t len, ObjectID *id_out) {
    unsigned int hash_len;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal_ex(ctx, id_out->hash, &hash_len);
    EVP_MD_CTX_free(ctx);
}

// Get the filesystem path where an object should be stored.
// Format: .pes/objects/XX/YYYYYYYY...
// The first 2 hex chars form the shard directory; the rest is the filename.
void object_path(const ObjectID *id, char *path_out, size_t path_size) {
    char hex[HASH_HEX_SIZE + 1];
    hash_to_hex(id, hex);
    snprintf(path_out, path_size, "%s/%.2s/%s", OBJECTS_DIR, hex, hex + 2);
}

int object_exists(const ObjectID *id) {
    char path[512];
    object_path(id, path, sizeof(path));
    return access(path, F_OK) == 0;
}

// ─── TODO: Implement these ──────────────────────────────────────────────────

int object_write(ObjectType type, const void *data, size_t len, ObjectID *id_out) {
    const char *type_str = NULL;
    if (type == OBJ_BLOB) type_str = "blob";
    else if (type == OBJ_TREE) type_str = "tree";
    else if (type == OBJ_COMMIT) type_str = "commit";
    else return -1;

    char header[64];
    int header_len = snprintf(header, sizeof(header), "%s %zu", type_str, len);
    if (header_len < 0 || (size_t)header_len + 1 > sizeof(header)) return -1;
    header[header_len++] = '\0';

    size_t full_len = (size_t)header_len + len;
    unsigned char *full = malloc(full_len);
    if (!full) return -1;

    memcpy(full, header, (size_t)header_len);
    memcpy(full + header_len, data, len);

    compute_hash(full, full_len, id_out);

    if (object_exists(id_out)) {
        free(full);
        return 0;
    }

    char hex[HASH_HEX_SIZE + 1];
    char dir[512];
    char final_path[512];
    char tmp_path[512];

    hash_to_hex(id_out, hex);
    snprintf(dir, sizeof(dir), "%s/%.2s", OBJECTS_DIR, hex);
    snprintf(final_path, sizeof(final_path), "%s/%s", dir, hex + 2);
    snprintf(tmp_path, sizeof(tmp_path), "%s/.tmp-%d", dir, getpid());

    mkdir(OBJECTS_DIR, 0755);
    if (mkdir(dir, 0755) < 0) {
        struct stat st;
        if (stat(dir, &st) < 0 || !S_ISDIR(st.st_mode)) {
            free(full);
            return -1;
        }
    }

    int fd = open(tmp_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        free(full);
        return -1;
    }

    size_t written = 0;
    while (written < full_len) {
        ssize_t n = write(fd, full + written, full_len - written);
        if (n < 0) {
            close(fd);
            unlink(tmp_path);
            free(full);
            return -1;
        }
        written += (size_t)n;
    }

    if (fsync(fd) < 0) {
        close(fd);
        unlink(tmp_path);
        free(full);
        return -1;
    }

    if (close(fd) < 0) {
        unlink(tmp_path);
        free(full);
        return -1;
    }

    if (rename(tmp_path, final_path) < 0) {
        unlink(tmp_path);
        free(full);
        return -1;
    }

    int dfd = open(dir, O_RDONLY);
    if (dfd >= 0) {
        fsync(dfd);
        close(dfd);
    }

    free(full);
    return 0;
}

int object_read(const ObjectID *id, ObjectType *type_out, void **data_out, size_t *len_out) {
    char path[512];
    object_path(id, path, sizeof(path));

    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }

    long sz = ftell(fp);
    if (sz < 0) {
        fclose(fp);
        return -1;
    }

    rewind(fp);

    unsigned char *buf = malloc((size_t)sz);
    if (!buf) {
        fclose(fp);
        return -1;
    }

    if (fread(buf, 1, (size_t)sz, fp) != (size_t)sz) {
        free(buf);
        fclose(fp);
        return -1;
    }
    fclose(fp);

    ObjectID check_id;
    compute_hash(buf, (size_t)sz, &check_id);
    if (memcmp(check_id.hash, id->hash, HASH_SIZE) != 0) {
        free(buf);
        return -1;
    }

    unsigned char *nul = memchr(buf, '\0', (size_t)sz);
    if (!nul) {
        free(buf);
        return -1;
    }

    char type_str[16];
    size_t obj_len = 0;
    if (sscanf((char *)buf, "%15s %zu", type_str, &obj_len) != 2) {
        free(buf);
        return -1;
    }

    if (strcmp(type_str, "blob") == 0) *type_out = OBJ_BLOB;
    else if (strcmp(type_str, "tree") == 0) *type_out = OBJ_TREE;
    else if (strcmp(type_str, "commit") == 0) *type_out = OBJ_COMMIT;
    else {
        free(buf);
        return -1;
    }

    size_t header_len = (size_t)(nul - buf) + 1;
    if (header_len + obj_len != (size_t)sz) {
        free(buf);
        return -1;
    }

    void *out = malloc(obj_len);
    if (!out) {
        free(buf);
        return -1;
    }

    memcpy(out, buf + header_len, obj_len);
    *data_out = out;
    *len_out = obj_len;

    free(buf);
    return 0;
}
