/*
 * Dental Clinic Management System
 * Doctor & Appointment Management Module
 *
 * Storage:
 *   doctors.dat       - seeded doctor directory (binary records)
 *   appointments.dat  - appointment records (binary records)
 *
 * Build:
 *   gcc -std=c11 -Wall -Wextra -pedantic main.c -o dental_clinic
 * Run:
 *   dental_clinic.exe      (Windows)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define DOCTOR_FILE "doctors.dat"
#define APPOINTMENT_FILE "appointments.dat"
#define NAME_LEN 60
#define SPEC_LEN 80
#define DATE_LEN 11       /* YYYY-MM-DD + '\0' */
#define TIME_LEN 6        /* HH:MM + '\0' */
#define PHONE_LEN 20
#define NOTES_LEN 120

typedef struct {
    int doctorID;
    char doctorName[NAME_LEN];
    char specialization[SPEC_LEN];
    char category[40];
} Doctor;

typedef struct {
    int appointmentID;
    int patientID;
    int doctorID;
    char patientName[NAME_LEN];
    char doctorName[NAME_LEN];
    char date[DATE_LEN];
    char time[TIME_LEN];
    char phone[PHONE_LEN];
    char notes[NOTES_LEN];
    int status; /* 1 = booked, 0 = cancelled */
} Appointment;

/* ---------- Safe input helpers ---------- */

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[--n] = '\0';
    }
}

static void read_line(const char *prompt, char *buffer, size_t size) {
    int ch;
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    if (strchr(buffer, '\n') == NULL) {
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
    trim_newline(buffer);
}

static int read_int(const char *prompt, int min, int max) {
    char line[80];
    char *end;
    long value;

    for (;;) {
        read_line(prompt, line, sizeof line);
        errno = 0;
        value = strtol(line, &end, 10);
        while (isspace((unsigned char)*end)) end++;
        if (line[0] != '\0' && errno == 0 && *end == '\0' &&
            value >= min && value <= max) {
            return (int)value;
        }
        printf("Please enter a whole number from %d to %d.\n", min, max);
    }
}

static int valid_date(const char *s) {
    int y, m, d;
    char extra;
    if (strlen(s) != 10 || sscanf(s, "%4d-%2d-%2d%c", &y, &m, &d, &extra) != 3)
        return 0;
    if (s[4] != '-' || s[7] != '-' || y < 2020 || m < 1 || m > 12)
        return 0;

    {
        static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        int max_day = days[m - 1];
        int leap = (y % 400 == 0) || (y % 4 == 0 && y % 100 != 0);
        if (m == 2 && leap) max_day = 29;
        return d >= 1 && d <= max_day;
    }
}

static int valid_time(const char *s) {
    int h, m;
    char extra;
    if (strlen(s) != 5 || sscanf(s, "%2d:%2d%c", &h, &m, &extra) != 2)
        return 0;
    return s[2] == ':' && h >= 0 && h <= 23 && m >= 0 && m <= 59;
}

static void read_date(char *date) {
    for (;;) {
        read_line("Appointment date (YYYY-MM-DD): ", date, DATE_LEN);
        if (valid_date(date)) return;
        printf("Invalid date. Use YYYY-MM-DD, for example 2026-11-05.\n");
    }
}

static void read_time(char *time) {
    for (;;) {
        read_line("Appointment time (24-hour HH:MM): ", time, TIME_LEN);
        if (valid_time(time)) return;
        printf("Invalid time. Use 24-hour format, for example 14:30.\n");
    }
}

/* ---------- Doctor directory ---------- */

static const Doctor DEFAULT_DOCTORS[] = {
    {1,  "Dr. Rupa Chandramala. R", "Oral Medicine & Radiology", "Oral Physician"},
    {2,  "Dr. Chetana Chalageri", "General Dentistry", "Senior Dental Surgeon"},
    {3,  "Dr. Navyashree. R", "Periodontics", "Periodontist"},
    {4,  "Dr. Poornima. B", "General Dentistry", "Senior Dental Surgeon"},
    {5,  "Dr. Suchith. V", "General Dentistry", "Junior Dental Surgeon"},
    {6,  "Dr. Monjit Neog", "Oral Medicine & Radiology", "Junior Dental Surgeon"},
    {7,  "Dr. Deepika. V", "General Dentistry", "Junior Dental Surgeon"},
    {8,  "Dr. Arun. V", "Root Canal Treatment / Endodontics", "Senior Consultant"},
    {9,  "Dr. Babashankar Alva", "Prosthodontics & Implantology", "Senior Consultant"},
    {10, "Dr. Ravi Kumar", "Paedodontics / Pediatric Dentistry", "Senior Consultant"},
    {11, "Dr. Roshan Sagarkar", "Orthodontics", "Senior Consultant"},
    {12, "Dr. Suhas. K", "Oral & Maxillofacial Surgery", "Consultant"}
};

#define DEFAULT_DOCTOR_COUNT (sizeof(DEFAULT_DOCTORS) / sizeof(DEFAULT_DOCTORS[0]))

static int doctor_exists(int doctorID, Doctor *out) {
    FILE *fp = fopen(DOCTOR_FILE, "rb");
    Doctor d;
    if (!fp) return 0;
    while (fread(&d, sizeof d, 1, fp) == 1) {
        if (d.doctorID == doctorID) {
            if (out) *out = d;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static void initialize_doctors(void) {
    FILE *fp = fopen(DOCTOR_FILE, "rb");
    if (fp) {
        fclose(fp);
        return;
    }

    fp = fopen(DOCTOR_FILE, "wb");
    if (!fp) {
        printf("Warning: Could not create %s. Doctor directory will be unavailable.\n",
               DOCTOR_FILE);
        return;
    }

    if (fwrite(DEFAULT_DOCTORS, sizeof(Doctor), DEFAULT_DOCTOR_COUNT, fp)
        != DEFAULT_DOCTOR_COUNT) {
        printf("Warning: Could not write the complete doctor directory.\n");
    }
    fclose(fp);
}

static void list_doctors(void) {
    FILE *fp = fopen(DOCTOR_FILE, "rb");
    Doctor d;
    if (!fp) {
        printf("Could not open doctor directory.\n");
        return;
    }

    printf("\n%-4s %-30s %-38s %s\n", "ID", "DOCTOR", "SPECIALIZATION", "CATEGORY");
    printf("-----------------------------------------------------------------------------------------------\n");
    while (fread(&d, sizeof d, 1, fp) == 1) {
        printf("%-4d %-30s %-38s %s\n", d.doctorID, d.doctorName,
               d.specialization, d.category);
    }
    fclose(fp);
}

/* ---------- Appointment storage and operations ---------- */

static int next_appointment_id(void) {
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    Appointment a;
    int maxID = 0;

    if (!fp) return 1;
    while (fread(&a, sizeof a, 1, fp) == 1) {
        if (a.appointmentID > maxID) maxID = a.appointmentID;
    }
    fclose(fp);
    return maxID + 1;
}

static int slot_is_taken(int doctorID, const char *date, const char *time) {
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    Appointment a;
    if (!fp) return 0;

    while (fread(&a, sizeof a, 1, fp) == 1) {
        if (a.status == 1 && a.doctorID == doctorID &&
            strcmp(a.date, date) == 0 && strcmp(a.time, time) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static void add_appointment(void) {
    Appointment a;
    Doctor d;
    FILE *fp;

    memset(&a, 0, sizeof a);
    printf("\n--- Book an Appointment ---\n");
    a.patientID = read_int("Patient ID (from patient module): ", 1, 2147483647);

    read_line("Patient name: ", a.patientName, sizeof a.patientName);
    if (a.patientName[0] == '\0') {
        printf("Patient name cannot be empty. Booking cancelled.\n");
        return;
    }

    list_doctors();
    a.doctorID = read_int("Select doctor ID: ", 1, 12);
    if (!doctor_exists(a.doctorID, &d)) {
        printf("Doctor ID not found. Booking cancelled.\n");
        return;
    }
    strcpy(a.doctorName, d.doctorName);

    read_date(a.date);
    read_time(a.time);
    if (slot_is_taken(a.doctorID, a.date, a.time)) {
        printf("That doctor already has a booked appointment at this date and time.\n");
        printf("Please choose another time. No appointment was saved.\n");
        return;
    }

    read_line("Contact phone (optional): ", a.phone, sizeof a.phone);
    read_line("Reason / treatment (optional): ", a.notes, sizeof a.notes);
    a.appointmentID = next_appointment_id();
    a.status = 1;

    fp = fopen(APPOINTMENT_FILE, "ab");
    if (!fp) {
        printf("Error: Could not open %s for writing.\n", APPOINTMENT_FILE);
        return;
    }
    if (fwrite(&a, sizeof a, 1, fp) != 1) {
        printf("Error: Appointment could not be saved.\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    printf("\nAppointment booked successfully!\n");
    printf("Appointment ID: %d\nPatient ID: %d\nDoctor: %s\nDate: %s\nTime: %s\n",
           a.appointmentID, a.patientID, a.doctorName, a.date, a.time);
}

static const char *status_text(int status) {
    return status == 1 ? "Booked" : "Cancelled";
}

static void print_appointment(const Appointment *a) {
    printf("\nAppointment ID : %d\n", a->appointmentID);
    printf("Patient ID     : %d\n", a->patientID);
    printf("Patient name   : %s\n", a->patientName);
    printf("Doctor ID      : %d\n", a->doctorID);
    printf("Doctor         : %s\n", a->doctorName);
    printf("Date           : %s\n", a->date);
    printf("Time           : %s\n", a->time);
    printf("Phone          : %s\n", a->phone[0] ? a->phone : "Not provided");
    printf("Reason         : %s\n", a->notes[0] ? a->notes : "Not provided");
    printf("Status         : %s\n", status_text(a->status));
}

static void view_all_appointments(void) {
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    Appointment a;
    int found = 0;

    if (!fp) {
        printf("\nNo appointments have been saved yet.\n");
        return;
    }

    printf("\n--- All Appointments ---\n");
    while (fread(&a, sizeof a, 1, fp) == 1) {
        print_appointment(&a);
        found = 1;
    }
    if (!found) printf("No appointments have been saved yet.\n");
    fclose(fp);
}

static void search_appointments(void) {
    int choice, key, found = 0;
    char date[DATE_LEN];
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    Appointment a;

    if (!fp) {
        printf("\nNo appointments have been saved yet.\n");
        return;
    }

    printf("\nSearch by:\n1. Appointment ID\n2. Patient ID\n3. Doctor ID\n4. Date\n");
    choice = read_int("Choose: ", 1, 4);

    if (choice == 1) key = read_int("Appointment ID: ", 1, 2147483647);
    else if (choice == 2) key = read_int("Patient ID: ", 1, 2147483647);
    else if (choice == 3) key = read_int("Doctor ID: ", 1, 12);
    else read_date(date);

    while (fread(&a, sizeof a, 1, fp) == 1) {
        int match = 0;
        if (choice == 1) match = a.appointmentID == key;
        else if (choice == 2) match = a.patientID == key;
        else if (choice == 3) match = a.doctorID == key;
        else match = strcmp(a.date, date) == 0;

        if (match) {
            print_appointment(&a);
            found = 1;
        }
    }
    if (!found) printf("No matching appointments found.\n");
    fclose(fp);
}

static void cancel_appointment(void) {
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    FILE *temp = fopen("appointments.tmp", "wb");
    Appointment a;
    int id, found = 0;

    if (!fp) {
        printf("\nNo appointments file exists yet.\n");
        if (temp) fclose(temp);
        remove("appointments.tmp");
        return;
    }
    if (!temp) {
        printf("Error: Could not create temporary file.\n");
        fclose(fp);
        return;
    }

    id = read_int("Enter appointment ID to cancel: ", 1, 2147483647);
    while (fread(&a, sizeof a, 1, fp) == 1) {
        if (a.appointmentID == id) {
            found = 1;
            if (a.status == 1) a.status = 0;
            else printf("Appointment %d was already cancelled.\n", id);
        }
        if (fwrite(&a, sizeof a, 1, temp) != 1) {
            printf("Error writing temporary appointment file.\n");
            fclose(fp);
            fclose(temp);
            remove("appointments.tmp");
            return;
        }
    }
    fclose(fp);
    fclose(temp);

    if (!found) {
        printf("Appointment ID %d was not found.\n", id);
        remove("appointments.tmp");
        return;
    }
    if (remove(APPOINTMENT_FILE) != 0) {
        printf("Could not replace appointment file: %s\n", strerror(errno));
        remove("appointments.tmp");
        return;
    }
    if (rename("appointments.tmp", APPOINTMENT_FILE) != 0) {
        printf("Could not finalize cancellation: %s\n", strerror(errno));
        return;
    }
    printf("Appointment %d cancelled (record retained for history).\n", id);
}

static void doctor_appointment_report(void) {
    int doctorID, found = 0;
    FILE *fp;
    Appointment a;
    Doctor d;

    list_doctors();
    doctorID = read_int("Enter doctor ID for report: ", 1, 12);
    if (!doctor_exists(doctorID, &d)) {
        printf("Doctor not found.\n");
        return;
    }

    printf("\n--- Appointment report for %s ---\n", d.doctorName);
    fp = fopen(APPOINTMENT_FILE, "rb");
    if (!fp) {
        printf("No appointments recorded.\n");
        return;
    }
    while (fread(&a, sizeof a, 1, fp) == 1) {
        if (a.doctorID == doctorID) {
            print_appointment(&a);
            found = 1;
        }
    }
    if (!found) printf("No appointments for this doctor.\n");
    fclose(fp);
}

static void show_menu(void) {
    printf("\n============================================\n");
    printf(" DENTAL CLINIC - DOCTOR & APPOINTMENTS\n");
    printf("============================================\n");
    printf("1. View doctor directory\n");
    printf("2. Book appointment\n");
    printf("3. View all appointments\n");
    printf("4. Search appointments\n");
    printf("5. Cancel appointment\n");
    printf("6. View appointments by doctor\n");
    printf("0. Save and exit\n");
}

int main(void) {
    int choice;
    initialize_doctors();

    printf("Welcome to the Dental Clinic Management System\n");
    for (;;) {
        show_menu();
        choice = read_int("Enter choice: ", 0, 6);
        switch (choice) {
            case 1: list_doctors(); break;
            case 2: add_appointment(); break;
            case 3: view_all_appointments(); break;
            case 4: search_appointments(); break;
            case 5: cancel_appointment(); break;
            case 6: doctor_appointment_report(); break;
            case 0:
                printf("Data is stored in local .dat files. Goodbye!\n");
                return 0;
        }
    }
}
