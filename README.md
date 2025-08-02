# CSE 314: Operating Systems Sessional

This repository holds my solutions for the CSE 314 Operating Systems sessional assignments at BUET (January 2025). The projects cover shell scripting, xv6 kernel modification, and process synchronization (IPC).

---

## 📂 Repository Structure
```
.
├── IPC
│   └── 2105028.cpp
├── Shell Scripting
│   └── organize.sh
└── System Call and Scheduling
└── 2105XXX.patch
```

---

## 🚀 Assignment Overviews

### 1. Shell Scripting: Submission Analyzer

A script ([organize.sh](./Shell%20Scripting/organize.sh)) that automates the grading process. It organizes student submissions from `.zip` files, analyzes code for metrics like line/comment counts, and executes the code against test cases to generate a final `result.csv` report.

### 2. xv6: System Call & MLFQ Scheduler

Modifications to the xv6 kernel, provided as a [patch file](./System%20Call%20and%20Scheduling/2105XXX.patch).

* **System Call:** Implemented a new `history` system call to track the usage count and time consumed by all system calls, with locking for multi-CPU support.
* **Scheduler:** Implemented a Multilevel Feedback Queue (MLFQ) scheduler with a top-level **Lottery** queue and a bottom-level **Round-Robin** queue.

### 3. IPC: The Peaky Blinders Problem

A C++ PThreads solution ([2105028.cpp](./IPC/2105028.cpp)) for a complex synchronization scenario. It manages thread access to limited shared resources and implements a **Readers-Writer** solution to control access to a shared data structure, all without busy waiting.

---

### Disclaimer

This work is for educational and reference purposes. Please adhere to your institution's academic integrity policies.
