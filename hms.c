/* hospital.c
   A streamlined, professional Hospital Management System in pure C.
   - Runs in any standard console/terminal.
   - No external libraries (like ncurses) needed.
   - Patient intake (disease + doctor) is a single, unified workflow.
   - Robust integer input and efficient qsort.

   --- HOW TO COMPILE ---
   gcc hospital.c -o hospital
   
   --- HOW TO RUN ---
   ./hospital
*/

#include <stdio.h>
#include <stdlib.h> // Added for qsort, atoi, strtol
#include <string.h>
#include <ctype.h>
#include <errno.h> // For checking strtol errors
#include <limits.h> // For INT_MAX, INT_MIN

#ifdef _WIN32
#include <windows.h> // For "cls"
#endif

/* --------------------- CONSTANTS --------------------- */
#define MAX_PATIENTS 500
#define MAX_DISEASES 200 // For the reference database
#define MAX_DOCTORS 100
#define MAX_APPOINTS 1000

/* --------------------- ANSI COLORS (Optional) --------------------- */
#define RESET_COLOR "\x1B[0m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"

/* --------------------- STRUCTS --------------------- */

typedef struct {
    int id;
    char name[100];
    int age;
    char gender[10];
    char phone[20];
    char disease[100];
    int doctorId;
} Patient;

typedef struct {
    int id;
    char name[100];
    char symptoms[200];
    char treatment[200];
} Disease;

typedef struct {
    int id;
    char name[100];
    char specialization[100];
    char phone[20];
} Doctor;

typedef struct {
    int id;
    int patientId;
    int doctorId;
    char date[20]; // "YYYY-MM-DD"
    char time[10]; // "HH:MM"
} Appointment;

/* --------------------- GLOBALS --------------------- */

Patient patients[MAX_PATIENTS];
Disease diseases[MAX_DISEASES];
Doctor doctors[MAX_DOCTORS];
Appointment appointments[MAX_APPOINTS];

int patientCount = 0;
int diseaseCount = 0;
int doctorCount = 0;
int appointmentCount = 0;

int nextPatientId = 1;
int nextDiseaseId = 1;
int nextDoctorId = 1;
int nextAppointmentId = 1;

/* --------------------- UTILS --------------------- */

// Clears the console screen
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear"); // Assumes POSIX-compliant system (Linux, macOS)
#endif
}

// Replacement for getLine() to safely read strings
void getLine(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL) {
        size_t ln = strlen(buffer) - 1;
        if (buffer[ln] == '\n') {
            buffer[ln] = '\0';
        } else {
            // Clear the input buffer if input was too long
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
        }
    } else {
        buffer[0] = '\0';
    }
}

// *** NEW *** Robust function to get an integer from the user
int get_int_from_user(const char *prompt) {
    char buffer[100];
    long value = 0;
    char *endptr;

    while (1) {
        getLine(prompt, buffer, sizeof(buffer));
        
        // Reset errno before call
        errno = 0;
        value = strtol(buffer, &endptr, 10);

        // Check for various errors
        if (endptr == buffer) { // No digits found
            printf(RED "Invalid input. Please enter a number.\n" RESET_COLOR);
        } else if (*endptr != '\0') { // Extra characters after number
            printf(RED "Invalid input. Please enter only a number.\n" RESET_COLOR);
        } else if (errno == ERANGE && (value == LONG_MAX || value == LONG_MIN)) { // Overflow/underflow
            printf(RED "Number is out of range.\n" RESET_COLOR);
        } else if (value > INT_MAX || value < INT_MIN) { // Out of int range
            printf(RED "Number is out of range for an ID.\n" RESET_COLOR);
        } else {
            // Valid integer
            break;
        }
    }
    return (int)value;
}

// Gets a choice and clears the input buffer
int get_choice() {
    return get_int_from_user("\nEnter your choice: ");
}

// case-insensitive string compare
int stricmp_custom(const char *a, const char *b) {
    while (*a && *b) {
        char ca = tolower((unsigned char)*a);
        char cb = tolower((unsigned char)*b);
        if (ca != cb) return (ca - cb);
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

// --- Consolidated Finder Functions ---
int findPatientIndex(int id) {
    for (int i = 0; i < patientCount; i++) {
        if (patients[i].id == id) return i;
    }
    return -1;
}

int findDoctorIndex(int id) {
    for (int i = 0; i < doctorCount; i++) {
        if (doctors[i].id == id) return i;
    }
    return -1;
}

int findAppointmentIndex(int id) {
    for (int i = 0; i < appointmentCount; i++) {
        if (appointments[i].id == id) return i;
    }
    return -1;
}

// *** NEW *** Helper to safely get patient name for prompts
const char* getPatientName(int id) {
    int i = findPatientIndex(id);
    if (i != -1) {
        return patients[i].name;
    }
    return "Unknown";
}


/* --------------------- PERSISTENCE --------------------- */

void saveData() {
    FILE *fp = fopen("hospital_data.bin", "wb");
    if (!fp) {
        printf(RED "? Error: Could not open save file for writing.\n" RESET_COLOR);
        return;
    }

    fwrite(&patientCount, sizeof(int), 1, fp);
    fwrite(&diseaseCount, sizeof(int), 1, fp);
    fwrite(&doctorCount, sizeof(int), 1, fp);
    fwrite(&appointmentCount, sizeof(int), 1, fp);

    fwrite(&nextPatientId, sizeof(int), 1, fp);
    fwrite(&nextDiseaseId, sizeof(int), 1, fp);
    fwrite(&nextDoctorId, sizeof(int), 1, fp);
    fwrite(&nextAppointmentId, sizeof(int), 1, fp);

    if (patientCount > 0) fwrite(patients, sizeof(Patient), patientCount, fp);
    if (diseaseCount > 0) fwrite(diseases, sizeof(Disease), diseaseCount, fp);
    if (doctorCount > 0) fwrite(doctors, sizeof(Doctor), doctorCount, fp);
    if (appointmentCount > 0) fwrite(appointments, sizeof(Appointment), appointmentCount, fp);

    fclose(fp);
    printf(GREEN "?? Data saved successfully.\n" RESET_COLOR);
}

void loadData() {
    FILE *fp = fopen("hospital_data.bin", "rb");
    if (!fp) {
        printf(YELLOW "No save file found. Starting new database.\n" RESET_COLOR);
        return;
    }

    fread(&patientCount, sizeof(int), 1, fp);
    fread(&diseaseCount, sizeof(int), 1, fp);
    fread(&doctorCount, sizeof(int), 1, fp);
    fread(&appointmentCount, sizeof(int), 1, fp);

    fread(&nextPatientId, sizeof(int), 1, fp);
    fread(&nextDiseaseId, sizeof(int), 1, fp);
    fread(&nextDoctorId, sizeof(int), 1, fp);
    fread(&nextAppointmentId, sizeof(int), 1, fp);

    if (patientCount > 0) fread(patients, sizeof(Patient), patientCount, fp);
    if (diseaseCount > 0) fread(diseases, sizeof(Disease), diseaseCount, fp);
    if (doctorCount > 0) fread(doctors, sizeof(Doctor), doctorCount, fp);
    if (appointmentCount > 0) fread(appointments, sizeof(Appointment), appointmentCount, fp);

    fclose(fp);
    printf(CYAN "?? Data loaded. Patients: %d, Diseases: %d, Doctors: %d, Appointments: %d\n" RESET_COLOR,
           patientCount, diseaseCount, doctorCount, appointmentCount);
    
    printf("Press Enter to continue...");
    getchar(); // Wait for user
}

/* --------------------- DOCTOR OPERATIONS --------------------- */

void displayDoctors() {
    if (doctorCount == 0) {
        printf(YELLOW "?? No doctors added yet.\n" RESET_COLOR);
        return;
    }
    printf("\n" MAGENTA "========== DOCTOR LIST ==========\n" RESET_COLOR);
    for (int i = 0; i < doctorCount; i++) {
        printf(CYAN "ID: %d" RESET_COLOR " | Name: %s | Specialization: %s\n", 
               doctors[i].id, doctors[i].name, doctors[i].specialization);
        printf("----------------------------------\n");
    }
}

void addDoctor() {
    clear_screen();
    if (doctorCount >= MAX_DOCTORS) {
        printf(RED "? Max doctors reached.\n" RESET_COLOR);
        return;
    }

    Doctor d;
    d.id = nextDoctorId++;
    char temp[200];

    printf(CYAN "\n--- New Doctor Registration ---\n" RESET_COLOR);
    getLine("Enter doctor name (e.g., Dr. Smith): ", temp, sizeof(temp));
    strncpy(d.name, temp, sizeof(d.name)-1); d.name[sizeof(d.name)-1] = '\0';

    getLine("Enter specialization: ", temp, sizeof(temp));
    strncpy(d.specialization, temp, sizeof(d.specialization)-1); d.specialization[sizeof(d.specialization)-1] = '\0';

    getLine("Enter phone: ", temp, sizeof(temp));
    strncpy(d.phone, temp, sizeof(d.phone)-1); d.phone[sizeof(d.phone)-1] = '\0';

    doctors[doctorCount++] = d;
    printf(GREEN "? Doctor added successfully! (ID: %d)\n" RESET_COLOR, d.id);
}

void viewDoctors() {
    clear_screen();
    displayDoctors(); // Use the helper
}

/* --------------------- PATIENT OPERATIONS --------------------- */

void addPatient() {
    clear_screen();
    if (patientCount >= MAX_PATIENTS) {
        printf(RED "? Max patients reached.\n" RESET_COLOR);
        return;
    }

    Patient p;
    p.id = nextPatientId++;
    char temp[200];

    printf(CYAN "\n--- New Patient Registration ---\n" RESET_COLOR);
    getLine("Enter patient name: ", temp, sizeof(temp));
    strncpy(p.name, temp, sizeof(p.name)-1); p.name[sizeof(p.name)-1] = '\0';

    p.age = get_int_from_user("Enter age: "); // Robust input

    getLine("Enter gender: ", temp, sizeof(temp));
    strncpy(p.gender, temp, sizeof(p.gender)-1); p.gender[sizeof(p.gender)-1] = '\0';

    getLine("Enter phone number: ", temp, sizeof(temp));
    strncpy(p.phone, temp, sizeof(p.phone)-1); p.phone[sizeof(p.phone)-1] = '\0';

    // --- STREAMLINED WORKFLOW ---
    printf(CYAN "\n--- Diagnosis & Assignment ---\n" RESET_COLOR);
    getLine("Enter patient's disease/condition: ", temp, sizeof(temp));
    strncpy(p.disease, temp, sizeof(p.disease)-1); p.disease[sizeof(p.disease)-1] = '\0';
    
    // Automatically show doctor list for assignment
    if (doctorCount > 0) {
        printf(YELLOW "\n--- Assign a Doctor ---" RESET_COLOR);
        displayDoctors(); // Show list
        int docId = get_int_from_user("Enter Doctor ID to assign (or 0 for none): ");
        
        if (docId != 0 && findDoctorIndex(docId) != -1) {
            p.doctorId = docId;
            printf(GREEN "? Doctor (ID: %d) assigned.\n" RESET_COLOR, docId);
        } else {
            p.doctorId = 0;
            if (docId != 0) {
                 printf(RED "?? No doctor found with ID %d. Patient assigned 'None'.\n" RESET_COLOR, docId);
            } else {
                 printf(YELLOW "?? Patient assigned 'None'.\n" RESET_COLOR);
            }
        }
    } else {
        printf(YELLOW "?? No doctors in system. Patient assigned 'None'.\n" RESET_COLOR);
        p.doctorId = 0; // No doctors to assign
    }

    patients[patientCount++] = p;
    printf(GREEN "\n? Patient added successfully! (ID: %d)\n" RESET_COLOR, p.id);
}

void viewPatients() {
    clear_screen();
    if (patientCount == 0) {
        printf(YELLOW "?? No patients available.\n" RESET_COLOR);
        return;
    }

    printf("\n" MAGENTA "========== PATIENT LIST ==========\n" RESET_COLOR);
    for (int i = 0; i < patientCount; i++) {
        printf(BLUE "ID: %d\n" RESET_COLOR, patients[i].id);
        printf("Name: %s\n", patients[i].name);
        printf("Age: %d\n", patients[i].age);
        printf("Gender: %s\n", patients[i].gender);
        printf("Phone: %s\n", patients[i].phone);
        printf(YELLOW "Disease: %s\n" RESET_COLOR, patients[i].disease);
        
        if (patients[i].doctorId != 0) {
            char dname[100] = "Unknown";
            int doc_idx = findDoctorIndex(patients[i].doctorId);
            if (doc_idx != -1) {
                strncpy(dname, doctors[doc_idx].name, sizeof(dname)-1);
            }
            printf(GREEN "Doctor: %s (ID: %d)\n" RESET_COLOR, dname, patients[i].doctorId);
        } else {
            printf(RED "Doctor: Not Assigned\n" RESET_COLOR);
        }
        printf("----------------------------------\n");
    }
}

void searchPatientById() {
    clear_screen();
    int id = get_int_from_user("\nEnter patient ID to search: ");
    if (id <= 0) { printf(RED "? Invalid ID.\n" RESET_COLOR); return; }

    int i = findPatientIndex(id);
    if (i != -1) {
        printf(GREEN "\n? Patient Found!\n" RESET_COLOR);
        printf("ID: %d\n", patients[i].id);
        printf("Name: %s\n", patients[i].name);
        printf("Age: %d\n", patients[i].age);
        printf("Gender: %s\n", patients[i].gender);
        printf("Phone: %s\n", patients[i].phone);
        printf(YELLOW "Disease: %s\n" RESET_COLOR, patients[i].disease);
        if (patients[i].doctorId) {
            int j = findDoctorIndex(patients[i].doctorId);
            if (j != -1) {
                printf(GREEN "Doctor: %s (ID: %d)\n" RESET_COLOR, doctors[j].name, doctors[j].id);
            } else {
                 printf(RED "Doctor: Unknown (ID: %d)\n" RESET_COLOR, patients[i].doctorId);
            }
        } else {
            printf(RED "Doctor: Not Assigned\n" RESET_COLOR);
        }
        return;
    }
    printf(YELLOW "? Patient with ID %d not found.\n" RESET_COLOR, id);
}

void searchPatientByName() {
    clear_screen();
    char name[100];
    getLine("\nEnter patient name to search: ", name, sizeof(name));

    int found = 0;
    for (int i = 0; i < patientCount; i++) {
        if (stricmp_custom(patients[i].name, name) == 0) {
            if (!found) { printf(GREEN "\n? Matches:\n" RESET_COLOR); }
            found = 1;
            printf("ID: %d | Name: %s | Disease: %s\n", patients[i].id, patients[i].name, patients[i].disease);
        }
    }
    if (!found) {
        printf(YELLOW "? No patient named '%s' found.\n" RESET_COLOR, name);
    }
}

void deletePatient() {
    clear_screen();
    int id = get_int_from_user("\nEnter patient ID to delete: ");
    if (id <= 0) { printf(RED "? Invalid ID.\n" RESET_COLOR); return; }

    int i = findPatientIndex(id);
    if (i != -1) {
        printf(YELLOW "Found: %s. Are you sure you want to delete? (y/n): " RESET_COLOR, patients[i].name);
        char confirm[10];
        getLine("", confirm, sizeof(confirm));
        
        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            // shift left
            for (int j = i; j < patientCount - 1; j++) patients[j] = patients[j+1];
            patientCount--;
            printf(GREEN "??? Patient deleted successfully.\n" RESET_COLOR);
        } else {
            printf(CYAN "Deletion canceled.\n" RESET_COLOR);
        }
        return;
    }
    printf(YELLOW "? No patient found with ID %d.\n" RESET_COLOR, id);
}

// *** NEW *** Comparison function for qsort
int comparePatientsByName(const void *a, const void *b) {
    Patient *pA = (Patient*)a;
    Patient *pB = (Patient*)b;
    return stricmp_custom(pA->name, pB->name);
}

void sortPatientsByName() {
    clear_screen();
    if (patientCount < 2) { 
        printf(YELLOW "?? Not enough patients to sort.\n" RESET_COLOR); 
        return; 
    }
    
    // Use qsort for efficient sorting
    qsort(patients, patientCount, sizeof(Patient), comparePatientsByName);
    
    printf(GREEN "? Patients sorted by name. Please use 'View All Patients' to see the new order.\n" RESET_COLOR);
}

/* --------------------- DISEASE REFERENCE OPERATIONS --------------------- */
void addDisease() {
    clear_screen();
    if (diseaseCount >= MAX_DISEASES) {
        printf(RED "? Max diseases reached.\n" RESET_COLOR);
        return;
    }

    Disease d;
    d.id = nextDiseaseId++;
    char tmp[300];
    
    printf(CYAN "\n--- Add to Disease Reference Database ---\n" RESET_COLOR);
    getLine("Enter disease name: ", tmp, sizeof(tmp));
    strncpy(d.name, tmp, sizeof(d.name)-1); d.name[sizeof(d.name)-1] = '\0';

    getLine("Enter common symptoms: ", tmp, sizeof(tmp));
    strncpy(d.symptoms, tmp, sizeof(d.symptoms)-1); d.symptoms[sizeof(d.symptoms)-1] = '\0';

    getLine("Enter common treatment: ", tmp, sizeof(tmp));
    strncpy(d.treatment, tmp, sizeof(d.treatment)-1); d.treatment[sizeof(d.treatment)-1] = '\0';

    diseases[diseaseCount++] = d;
    printf(GREEN "? Disease reference added successfully! (ID: %d)\n" RESET_COLOR, d.id);
}

void displayDiseases() {
    clear_screen();
    if (diseaseCount == 0) {
        printf(YELLOW "?? No diseases recorded in reference database.\n" RESET_COLOR);
        return;
    }
    printf("\n" MAGENTA "========== DISEASE REFERENCE DATABASE ==========\n" RESET_COLOR);
    for (int i = 0; i < diseaseCount; i++) {
        printf(BLUE "ID: %d\nName: %s\n" RESET_COLOR, diseases[i].id, diseases[i].name);
        printf("Symptoms: %s\nTreatment: %s\n", diseases[i].symptoms, diseases[i].treatment);
        printf("----------------------------------\n");
    }
}

/* --------------------- APPOINTMENTS --------------------- */

void addAppointment() {
    clear_screen();
    if (appointmentCount >= MAX_APPOINTS) {
        printf(RED "? Max appointments reached.\n" RESET_COLOR);
        return;
    }
    if (patientCount == 0 || doctorCount == 0) {
        printf(YELLOW "?? Need at least one patient and one doctor to schedule.\n" RESET_COLOR);
        return;
    }
    
    printf(CYAN "\n--- Schedule New Appointment ---\n" RESET_COLOR);

    int pid = get_int_from_user("Enter patient ID: ");
    int did = get_int_from_user("Enter doctor ID: ");

    int pi = findPatientIndex(pid);
    int di = findDoctorIndex(did);

    if (pi == -1 || di == -1) { 
        printf(YELLOW "? Invalid patient or doctor ID.\n" RESET_COLOR); 
        return; 
    }

    Appointment a;
    a.id = nextAppointmentId++;
    a.patientId = pid;
    a.doctorId = did;

    char temp[200];
    getLine("Enter date (YYYY-MM-DD): ", a.date, sizeof(a.date));
    getLine("Enter time (HH:MM): ", a.time, sizeof(a.time));

    appointments[appointmentCount++] = a;

    // --- FIX ---
    // If the patient doesn't have a primary doctor,
    // assign the doctor from the appointment.
    if (patients[pi].doctorId == 0) {
        patients[pi].doctorId = did;
        printf(CYAN "Note: %s has been set as the primary doctor for %s.\n" RESET_COLOR, doctors[di].name, patients[pi].name);
    }
    // --- END FIX ---

    printf(GREEN "? Appointment scheduled (ID: %d) for patient %s with %s on %s %s\n" RESET_COLOR,
           a.id, patients[pi].name, doctors[di].name, a.date, a.time);
}

void displayAppointments() {
    clear_screen();
    if (appointmentCount == 0) {
        printf(YELLOW "?? No appointments scheduled.\n" RESET_COLOR);
        return;
    }
    printf("\n" MAGENTA "========== APPOINTMENTS ==========\n" RESET_COLOR);
    for (int i = 0; i < appointmentCount; i++) {
        int pid = appointments[i].patientId;
        int did = appointments[i].doctorId;
        char pname[100] = "Unknown";
        char dname[100] = "Unknown";

        int pi = findPatientIndex(pid);
        int di = findDoctorIndex(did);
        if (pi != -1) strncpy(pname, patients[pi].name, sizeof(pname)-1);
        if (di != -1) strncpy(dname, doctors[di].name, sizeof(dname)-1);

        printf(BLUE "Appointment ID: %d\n" RESET_COLOR, appointments[i].id);
        printf("Patient: %s (ID: %d)\n", pname, pid);
        printf("Doctor: %s (ID: %d)\n", dname, did);
        printf("Date: %s\nTime: %s\n", appointments[i].date, appointments[i].time);
        printf("----------------------------------\n");
    }
}

void cancelAppointment() {
    clear_screen();
    int id = get_int_from_user("\nEnter appointment ID to cancel: ");
    if (id <= 0) { printf(RED "? Invalid ID.\n" RESET_COLOR); return; }

    int i = findAppointmentIndex(id);
    if (i != -1) {
        // *** BUG FIX ***: Used new getPatientName helper function
        printf(YELLOW "Found appointment for %s. Are you sure? (y/n): " RESET_COLOR, getPatientName(appointments[i].patientId));
        char confirm[10];
        getLine("", confirm, sizeof(confirm));

        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            for (int j = i; j < appointmentCount - 1; j++) appointments[j] = appointments[j+1];
            appointmentCount--;
            printf(GREEN "? Appointment canceled.\n" RESET_COLOR);
        } else {
            printf(CYAN "Canceled.\n" RESET_COLOR);
        }
        return;
    }
    printf(YELLOW "? No appointment found with ID %d.\n" RESET_COLOR, id);
}

/* --------------------- MENU / UI --------------------- */

void display_menu() {
    clear_screen();
    printf(CYAN "============================================\n" RESET_COLOR);
    printf(CYAN "    Professional Hospital Management System\n" RESET_COLOR);
    printf(CYAN "============================================\n\n" RESET_COLOR);

    printf(YELLOW "Patient Management\n" RESET_COLOR);
    printf(BLUE " 1." RESET_COLOR " Add Patient (Full Intake)\n");
    printf(BLUE " 2." RESET_COLOR " View All Patients\n");
    printf(BLUE " 3." RESET_COLOR " Search Patient by ID\n");
    printf(BLUE " 4." RESET_COLOR " Search Patient by Name\n");
    printf(BLUE " 5." RESET_COLOR " Delete Patient\n");
    printf(BLUE " 6." RESET_COLOR " Sort Patients by Name\n");
    
    printf(YELLOW "\nStaff & Reference\n" RESET_COLOR);
    printf(BLUE " 7." RESET_COLOR " Add Doctor\n");
    printf(BLUE " 8." RESET_COLOR " View Doctors\n");
    printf(BLUE " 9." RESET_COLOR " Add Disease (Reference)\n");
    printf(BLUE " 10." RESET_COLOR " View Diseases (Reference)\n");
    
    printf(YELLOW "\nScheduling\n" RESET_COLOR);
    printf(BLUE " 11." RESET_COLOR " Schedule Appointment\n");
    printf(BLUE " 12." RESET_COLOR " View Appointments\n");
    printf(BLUE " 13." RESET_COLOR " Cancel Appointment\n");

    printf(YELLOW "\nSystem\n" RESET_COLOR);
    printf(BLUE " 14." RESET_COLOR " Save Data Now\n");
    printf(BLUE " 15." RESET_COLOR " Exit\n");
}

/* --------------------- MAIN --------------------- */

int main() {
    loadData(); // Load data on start
    
    int running = 1;
    while (running) {
        display_menu();
        int choice = get_choice(); // Robust choice function

        switch (choice) {
            case 1: addPatient(); break;
            case 2: viewPatients(); break;
            case 3: searchPatientById(); break;
            case 4: searchPatientByName(); break;
            case 5: deletePatient(); break;
            case 6: sortPatientsByName(); break;
            case 7: addDoctor(); break;
            case 8: viewDoctors(); break;
            case 9: addDisease(); break;
            case 10: displayDiseases(); break;
            case 11: addAppointment(); break;
            case 12: displayAppointments(); break;
            case 13: cancelAppointment(); break;
            case 14: saveData(); break;
            case 15:
                saveData(); // Auto-save on exit
                printf(MAGENTA "?? Exiting. Goodbye!\n" RESET_COLOR);
                running = 0;
                break;
            default:
                printf(RED "?? Invalid choice. Try again.\n" RESET_COLOR);
        }

        if (running && choice != 15) {
            printf("\nPress Enter to return to menu...");
            getchar(); // Wait for user
        }
    }
    return 0;
}
