# PES VCS Lab - Content Addressable Version Control System

## 👨‍💻 Student Details
- Name: C. KOUSHIK
- SRN: PES1UG24CS128
- Section: 4-C

---

## 📌 Project Description

This project implements a simplified Git-like Version Control System called PES VCS.  
It supports object storage, trees, indexing, commits, and history traversal.

---

## ⚙️ Features Implemented

### Phase 1 - Object Store
- Content-addressable storage using SHA-256
- Blob objects stored in `.pes/objects/`
- Functions:
  - object_write()
  - object_read()

---

### Phase 2 - Tree Objects
- Directory snapshot representation
- Tree serialization and parsing
- Functions:
  - tree_serialize()
  - tree_parse()
  - tree_from_index()

---

### Phase 3 - Index (Staging Area)
- Tracks files to be committed
- Text-based `.pes/index`
- Functions:
  - index_load()
  - index_save()
  - index_add()

---

### Phase 4 - Commit System
- Stores project snapshots
- Maintains commit history
- Functions:
  - commit_create()
  - commit_parse()
  - commit_serialize()
  - commit_walk()

---

## 🚀 How to Run

make  
./pes init  
echo Hello > file.txt  
./pes add file.txt  
./pes commit -m "Initial commit"  
./pes log  

---

## 📸 Screenshots Included

Phase 1: Object tests

<img width="984" height="162" alt="1A" src="https://github.com/user-attachments/assets/35c03be4-10ff-4126-8360-22fd53f53d77" />


<img width="801" height="89" alt="1B" src="https://github.com/user-attachments/assets/5e706703-b716-4faf-b540-dbdb54ccbcb0" />

Phase 2: Tree serialization

<img width="668" height="125" alt="2A" src="https://github.com/user-attachments/assets/5a6b1cc7-776c-449e-82fb-2ee3c67b4e5e" />


<img width="1421" height="74" alt="2B" src="https://github.com/user-attachments/assets/fff79b36-888e-405c-a49a-6dd713462c4f" />

Phase 3: Index and status

<img width="828" height="493" alt="3A" src="https://github.com/user-attachments/assets/942ae951-5a6d-4952-a78c-dc6996c461a0" />

<img width="959" height="75" alt="3B" src="https://github.com/user-attachments/assets/527061e9-ee11-4657-b90e-0568c200f446" />

Phase 4: Commit log

<img width="925" height="829" alt="4A" src="https://github.com/user-attachments/assets/dbe86107-17de-455d-b9d6-d2bcac8d6487" />

<img width="855" height="263" alt="4B" src="https://github.com/user-attachments/assets/91326ab7-6409-4551-acbb-26b506aee138" />

<img width="798" height="88" alt="4C" src="https://github.com/user-attachments/assets/ba6b0652-3489-40bc-af89-de00288ef09e" />

FINAL INTEGRATION TEST

image
---

## 🧠 Concepts Learned

- Hashing (SHA-256)  
- File system storage  
- Version control internals  
- Trees and commits  
- Atomic file operations  

---

## 📂 Repository Structure

.
├── object.c  
├── tree.c  
├── index.c  
├── commit.c  
├── pes.c  
├── Makefile  
├── README.md  
└── screenshots/  

---

## 🎯 Conclusion

This project helped in understanding how Git internally manages data using content-addressable storage, trees, and commits.
