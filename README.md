# C-Hospital-Management-System


A robust and professional **Hospital Management System (HMS)** developed from scratch in **pure C**. This application runs entirely in a standard console or terminal environment, requiring **no external libraries** (like ncurses) to compile or run.

The system is designed for efficiency and reliability, featuring robust input handling, data persistence, and an integrated workflow for managing patients, doctors, and appointments.

## ✨ Key Features

* **Pure C Implementation:** Zero external library dependencies, making it highly portable.
* **Robust Input Handling:** Uses `strtol` for safe and error-checked integer input (`get_int_from_user`), preventing crashes from non-numeric input.
* **Data Persistence:** Saves all system data (patients, doctors, appointments, etc.) to a binary file (`hospital_data.bin`) on exit and loads it automatically on startup.
* **Efficient Sorting:** Implements the standard `qsort` utility to efficiently sort and display patients by name.
* **Streamlined Workflow:** Patient intake includes a unified step for diagnosis entry and immediate doctor assignment.
* **Core Modules:**
    * Patient Management (Add, View, Search, Delete, Sort)
    * Doctor Management (Add, View)
    * Disease Reference (Add, View common symptoms/treatments)
    * Appointment Scheduling (Schedule, View, Cancel)
* **Cross-Platform Compatibility:** Utilizes platform-specific commands (`cls`/`clear`) for screen clearing.

## ⚙️ How to Compile

The system is compiled using the standard GNU Compiler Collection (`gcc`).

1.  Save the source code as `hospital.c`.
2.  Open your terminal or command prompt.
3.  Execute the following command to compile the executable:

```bash
gcc hospital.c -o hospital
