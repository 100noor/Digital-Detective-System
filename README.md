# 🔍 Digital Detective System

A Graph-Based Investigation and Suspicious Activity Detection System developed in **C++**, **Qt Creator**, and **SQLite** as a Data Structures & Algorithms project.

The system helps investigators manage suspects, relationships, activity logs, and automatically detect suspicious patterns using graph analysis, BFS traversal,hash function, ceacer cipher and rule-based intelligence.

---

## 📖 Project Overview

The Digital Detective System simulates a criminal investigation environment where investigators can:
- hash 256 on password
- Manage suspect profiles
- Build relationship networks
- Store investigation logs
- Find shortest connections between people
- Detect suspicious activities automatically
- Generate risk alerts
- Encrypt sensitive information
- Undo recent operations

The project demonstrates the practical implementation of various Data Structures, Algorithms, and basic Artificial Intelligence concepts.

---

## 🚀 Features

### 👤 User Authentication
- User Registration (Sign Up)
- Login System
- Logout Functionality
- Password Validation
- Username Validation

### 👥 Person Management
- Add Person
- Edit Person Role
- Search Person
- Soft Delete Person
- Reactivate Deleted Person

### 🔗 Relationship Management
- Add Relationship
- Remove Relationship
- Graph-Based Network Creation
- Adjacency List Representation

### 📝 Activity Log Management
- Add Investigation Logs
- Timestamp Validation
- Date-Based Log Organization
- Sorted Log Storage

### 🔍 Investigation Analysis
- Find Shortest Path using BFS
- Detect Network Clusters
- Rank People by Connections
- Suspicious Activity Detection
- Rule-Based Alert Generation

### 🔐 Security Features
- Caesar Cipher Encryption
- Caesar Cipher Decryption
- User Authentication

### ↩ Undo System
- Undo Added Logs
- Undo Deleted Persons
- Undo Role Modifications

### 💾 Database Integration
- SQLite Database
- Persistent Data Storage
- Automatic Data Loading on Startup

---

## 🧠 AI Concepts Used

Although this is not a Machine Learning project, it implements several Artificial Intelligence concepts:

### Rule-Based Expert System
The system automatically generates alerts using predefined investigation rules.

Examples:

- IF suspicion score ≥ 8 → HIGH RISK
- IF many suspect connections exist → HIGH RISK
- IF cluster contains many suspects → SUSPICIOUS CLUSTER

### Pattern Recognition
Activity logs are analyzed for suspicious keywords:

- transfer
- money
- drugs
- hack
- suspicious

### Knowledge Representation
The investigation network is represented using a graph:

- Person → Node
- Relationship → Edge

### Search Algorithms
Breadth First Search (BFS) is used to find shortest paths between suspects.

### Network Analysis
Cluster detection identifies suspicious groups and connected communities.

---

## 📚 Data Structures Used

| Data Structure | Purpose |
|--------------|----------|
| Vector | Store persons, logs, graph data |
| Map | Fast lookup and log organization |
| Queue | BFS traversal |
| Stack | Undo functionality |
| Graph | Relationship network |
| Adjacency List | Graph representation |

---

## ⚙ Algorithms Used

### Breadth First Search (BFS)
Used to:
- Find shortest path between suspects
- Discover connections within the network

### Cluster Detection
Used to:
- Find connected groups of people
- Identify suspicious communities

### Rule Engine
Used to:
- Generate risk alerts automatically

### Caesar Cipher
Used to:
- Encrypt and decrypt sensitive information

---

## 🏗 Technologies Used

- C++
- Qt Creator
- Qt Widgets
- SQLite
- CMake

---

## Project Structure
DigitalDetectiveSystem/
│
├── main.cpp
├── mainwindow.cpp
├── mainwindow.h
├── mainwindow.ui
│
├── login.cpp
├── login.h
├── login.ui
│
├── signup.cpp
├── signup.h
├── signup.ui
│
├── logic.cpp
├── logic.h
│
├── dbmanager.cpp
├── dbmanager.h
│
├── models.h
│
├── database.db
│
└── README.md
---

## 🖥 Screens Included

- Login Window
- Registration Window
- Main Dashboard
- Person Management
- Relationship Management
- Activity Logs
- Suspicious Detection
- BFS Shortest Path Output

---

## 📊 Sample Use Case

1. Create users through Sign Up.
2. Login to the system.
3. Add suspects and investigators.
4. Create relationships between people.
5. Add activity logs.
6. Run suspicious detection.
7. View generated alerts.
8. Find shortest paths between suspects.
9. Analyze clusters within the network.

---

## 🔮 Future Improvements

- Machine Learning Based Detection
- Natural Language Processing (NLP)
- Real Graph Visualization
- Cloud Database Integration
- Multi-User Support
- Mobile Application
- Stronger Encryption Techniques
- AI Chat Assistant

---

## 👨‍💻 Team Members

- Alisha Muneer
- Raheen Ifhaam
- Ayza Ifhaam
- Noor-ul-Ain

---

## 🎓 Academic Information

Course: Data Structures & Algorithms

Additional Concepts:
- Artificial Intelligence
- Information Security
- Programming Fundamentals

Institution:
COMSATS University Islamabad

---

## 📜 License

This project was developed for educational and academic purposes only.
