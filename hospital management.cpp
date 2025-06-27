/*
 * Hospital Management System (Console-based)
 * Language   : C (C99)
 * Author     : Angadi Anjali
 * Build (file mode):   gcc hospital.c -o hospital
 * Build (MySQL mode):  gcc hospital.c -o hospital `mysql_config --cflags --libs` -DUSE_MYSQL=1
 */

/********************** CONFIGURATION ***************************/
#ifndef USE_MYSQL
#define USE_MYSQL 0          /* 0 = file mode, 1 = MySQL mode */
#endif

#define PATIENT_FILE      "patients.dat"
#define APPOINTMENT_FILE  "appointments.dat"

#define MAX_NAME_LEN      100
#define MAX_GENDER_LEN    10
#define MAX_DIAG_LEN      120
#define MAX_DATE_LEN      12   /* YYYY-MM-DD */
#define MAX_TIME_LEN      8    /* HH:MM */
#define MAX_DOCTOR_LEN    64

/********************** HEADERS & GLOBALS **********************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if USE_MYSQL
#include <mysql/mysql.h>
#endif

/* ---------- Data Structures ---------- */
typedef struct {
    int  id;
    char name[MAX_NAME_LEN];
    int  age;
    char gender[MAX_GENDER_LEN];
    char diagnosis[MAX_DIAG_LEN];
} Patient;

typedef struct {
    int  appointmentId;
    int  patientId;
    char date[MAX_DATE_LEN];
    char time[MAX_TIME_LEN];
    char doctor[MAX_DOCTOR_LEN];
} Appointment;

/* ---------- MySQL helpers ---------- */
#if USE_MYSQL
static MYSQL *conn = NULL;
static void db_connect() {
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root", "root", "hospital_db", 0, NULL, 0)) {
        fprintf(stderr, "MySQL connection failed: %s\n", mysql_error(conn));
        exit(EXIT_FAILURE);
    }
}
static void db_disconnect() {
    if (conn) mysql_close(conn);
}
#endif

/* ---------- Utility: safe fgets & flush stdin ---------- */
static void flushStdin(void) {
    int c; while ((c = getchar()) != '\n' && c != EOF);
}
static void inputString(const char *prompt, char *buf, size_t len) {
    printf("%s", prompt);
    if (fgets(buf, (int)len, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    }
}

/* ---------- ID Generators ---------- */
static int generateNewId(const char *fname, size_t structSize, int idOffset) {
    FILE *fp = fopen(fname, "rb");
    if (!fp) return idOffset; /* first record */
    fseek(fp, -((long)structSize), SEEK_END);
    int lastId = 0;
    fread(&lastId, sizeof(int), 1, fp); /* first field inside both structs is id */
    fclose(fp);
    return lastId + 1;
}

/********************** Patient Functions **********************/
static void addPatient() {
#if USE_MYSQL
    /* --- insert into MySQL --- */
    Patient p; 
    p.id = 0; /* AUTO_INCREMENT handled by DB */
    inputString("Name              : ", p.name, MAX_NAME_LEN);
    printf("Age               : ");  scanf("%d", &p.age); flushStdin();
    inputString("Gender (M/F/O)    : ", p.gender, MAX_GENDER_LEN);
    inputString("Diagnosis         : ", p.diagnosis, MAX_DIAG_LEN);

    char query[512];
    snprintf(query, sizeof(query),
            "INSERT INTO patients(name, age, gender, diagnosis) VALUES('%s', %d, '%s', '%s')",
            p.name, p.age, p.gender, p.diagnosis);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "MySQL error: %s\n", mysql_error(conn));
    } else {
        printf("Patient added successfully.\n");
    }
#else
    Patient p;
    p.id = generateNewId(PATIENT_FILE, sizeof(Patient), 1);
    inputString("Name              : ", p.name, MAX_NAME_LEN);
    printf("Age               : ");  scanf("%d", &p.age); flushStdin();
    inputString("Gender (M/F/O)    : ", p.gender, MAX_GENDER_LEN);
    inputString("Diagnosis         : ", p.diagnosis, MAX_DIAG_LEN);

    FILE *fp = fopen(PATIENT_FILE, "ab");
    if (!fp) { perror("File open"); return; }
    fwrite(&p, sizeof(Patient), 1, fp);
    fclose(fp);
    printf("Patient added. Generated ID = %d\n", p.id);
#endif
}

static void viewPatients() {
#if USE_MYSQL
    if (mysql_query(conn, "SELECT id,name,age,gender,diagnosis FROM patients")) {
        fprintf(stderr, "MySQL error: %s\n", mysql_error(conn));
        return;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row;
    printf("\n%-4s %-25s %-4s %-8s %-s\n", "ID", "Name", "Age", "Gender", "Diagnosis");
    while ((row = mysql_fetch_row(res))) {
        printf("%-4s %-25s %-4s %-8s %-s\n", row[0], row[1], row[2], row[3], row[4]);
    }
    mysql_free_result(res);
#else
    FILE *fp = fopen(PATIENT_FILE, "rb");
    if (!fp) { puts("No patients yet."); return; }
    Patient p;
    printf("\n%-4s %-25s %-4s %-8s %-s\n", "ID", "Name", "Age", "Gender", "Diagnosis");
    while (fread(&p, sizeof(Patient), 1, fp) == 1) {
        printf("%-4d %-25s %-4d %-8s %-s\n", p.id, p.name, p.age, p.gender, p.diagnosis);
    }
    fclose(fp);
#endif
}

static void searchPatient() {
    int searchId;
    printf("Enter Patient ID to search: ");
    scanf("%d", &searchId); flushStdin();
#if USE_MYSQL
    char query[128];
    snprintf(query, sizeof(query), "SELECT id,name,age,gender,diagnosis FROM patients WHERE id=%d", searchId);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "MySQL error: %s\n", mysql_error(conn));
        return;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        printf("\nID: %s\nName: %s\nAge: %s\nGender: %s\nDiagnosis: %s\n", row[0], row[1], row[2], row[3], row[4]);
    } else {
        puts("Patient not found.");
    }
    mysql_free_result(res);
#else
    FILE *fp = fopen(PATIENT_FILE, "rb"); if (!fp) { puts("No records."); return; }
    Patient p; int found = 0;
    while (fread(&p, sizeof(Patient), 1, fp) == 1) {
        if (p.id == searchId) { found = 1; break; }
    }
    fclose(fp);
    if (found) {
        printf("\nID: %d\nName: %s\nAge: %d\nGender: %s\nDiagnosis: %s\n", p.id, p.name, p.age, p.gender, p.diagnosis);
    } else puts("Patient not found.");
#endif
}

static void editPatient() {
    int editId; printf("Enter Patient ID to edit: "); scanf("%d", &editId); flushStdin();
#if USE_MYSQL
    /* simplistic: update name, age, diagnosis */
    char name[MAX_NAME_LEN], diag[MAX_DIAG_LEN]; int age;
    inputString("New Name (leave blank to keep): ", name, MAX_NAME_LEN);
    printf("New Age (0 to keep)           : "); scanf("%d", &age); flushStdin();
    inputString("New Diagnosis (blank=keep)    : ", diag, MAX_DIAG_LEN);
    char query[512] = {0};
    strcat(query, "UPDATE patients SET ");
    int first = 1;
    if (strlen(name)) { snprintf(query + strlen(query), sizeof(query)-strlen(query), "name='%s'", name); first = 0; }
    if (age) { snprintf(query + strlen(query), sizeof(query)-strlen(query), "%s age=%d", first?"":" ,", age); first = 0; }
    if (strlen(diag)) { snprintf(query + strlen(query), sizeof(query)-strlen(query), "%s diagnosis='%s'", first?"":" ,", diag); }
    snprintf(query + strlen(query), sizeof(query)-strlen(query), " WHERE id=%d", editId);
    if (mysql_query(conn, query)) fprintf(stderr, "MySQL error: %s\n", mysql_error(conn));
    else puts("Patient updated.");
#else
    FILE *fp = fopen(PATIENT_FILE, "rb+"); if (!fp) { puts("No records."); return; }
    Patient p; long pos=-1; while (fread(&p, sizeof(Patient), 1, fp) == 1) {
        if (p.id == editId) { pos = ftell(fp) - sizeof(Patient); break; }
    }
    if (pos == -1) { puts("Patient not found."); fclose(fp); return; }
    /* get new values */
    inputString("New Name (leave blank to keep): ", p.name, MAX_NAME_LEN);
    printf("New Age (0 to keep)           : "); int newAge; scanf("%d", &newAge); flushStdin(); if (newAge) p.age = newAge;
    inputString("New Gender (blank=keep)       : ", p.gender, MAX_GENDER_LEN);
    inputString("New Diagnosis (blank=keep)    : ", p.diagnosis, MAX_DIAG_LEN);
    fseek(fp, pos, SEEK_SET);
    fwrite(&p, sizeof(Patient), 1, fp);
    fclose(fp);
    puts("Patient updated.");
#endif
}

/******************** Appointment Functions ********************/
static void scheduleAppointment() {
    Appointment a; 
#if USE_MYSQL
    a.appointmentId = 0; /* handled by DB */
    printf("Patient ID          : "); scanf("%d", &a.patientId); flushStdin();
    inputString("Date (YYYY-MM-DD)   : ", a.date, MAX_DATE_LEN);
    inputString("Time (HH:MM)        : ", a.time, MAX_TIME_LEN);
    inputString("Doctor              : ", a.doctor, MAX_DOCTOR_LEN);
    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO appointments(patientId,date,time,doctor) VALUES(%d,'%s','%s','%s')",
             a.patientId, a.date, a.time, a.doctor);
    if (mysql_query(conn, query)) fprintf(stderr, "MySQL error: %s\n", mysql_error(conn));
    else puts("Appointment scheduled.");
#else
    a.appointmentId = generateNewId(APPOINTMENT_FILE, sizeof(Appointment), 1001);
    printf("Patient ID          : "); scanf("%d", &a.patientId); flushStdin();
    inputString("Date (YYYY-MM-DD)   : ", a.date, MAX_DATE_LEN);
    inputString("Time (HH:MM)        : ", a.time, MAX_TIME_LEN);
    inputString("Doctor              : ", a.doctor, MAX_DOCTOR_LEN);
    FILE *fp = fopen(APPOINTMENT_FILE, "ab"); if (!fp) { perror("File open"); return; }
    fwrite(&a, sizeof(Appointment), 1, fp); fclose(fp);
    printf("Appointment scheduled. ID=%d\n", a.appointmentId);
#endif
}

static void viewAppointments() {
#if USE_MYSQL
    if (mysql_query(conn, "SELECT appointmentId,patientId,date,time,doctor FROM appointments")) {
        fprintf(stderr, "MySQL error: %s\n", mysql_error(conn));
        return;
    }
    MYSQL_RES *res = mysql_store_result(conn); MYSQL_ROW row;
    printf("\n%-4s %-6s %-12s %-6s %-s\n", "ID", "P_ID", "Date", "Time", "Doctor");
    while ((row = mysql_fetch_row(res))) {
        printf("%-4s %-6s %-12s %-6s %-s\n", row[0], row[1], row[2], row[3], row[4]);
    }
    mysql_free_result(res);
#else
    FILE *fp = fopen(APPOINTMENT_FILE, "rb"); if (!fp) { puts("No appointments."); return; }
    Appointment a; printf("\n%-4s %-6s %-12s %-6s %-s\n", "ID", "P_ID", "Date", "Time", "Doctor");
    while (fread(&a, sizeof(Appointment), 1, fp) == 1) {
        printf("%-4d %-6d %-12s %-6s %-s\n", a.appointmentId, a.patientId, a.date, a.time, a.doctor);
    }
    fclose(fp);
#endif
}

/********************** MAIN MENU ******************************/
static void mainMenu() {
    int choice;
    do {
        puts("\n========= Hospital Management =========");
        puts("1. Add Patient");
        puts("2. View Patients");
        puts("3. Edit Patient");
        puts("4. Search Patient");
        puts("5. Schedule Appointment");
        puts("6. View Appointments");
        puts("0. Exit");
        printf("Enter choice: ");
        scanf("%d", &choice); flushStdin();
        switch (choice) {
            case 1: addPatient(); break;
            case 2: viewPatients(); break;
            case 3: editPatient(); break;
            case 4: searchPatient(); break;
            case 5: scheduleAppointment(); break;
            case 6: viewAppointments(); break;
            case 0: puts("Exiting..."); break;
            default: puts("Invalid choice");
        }
    } while (choice != 0);
}

int main() {
#if USE_MYSQL
    db_connect();
#endif
    mainMenu();
#if USE_MYSQL
    db_disconnect();
#endif
    return 0;
}

