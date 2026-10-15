#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "appointments.dat"

typedef struct {
    int id;
    char patientName[50];
    int age;
    char gender[15];
    char doctorName[50];
    char date[15];
    char time[15];
    char phone[20];
    int status;  // 1 = Booked, 0 = Cancelled
} Appointment;

int generateID(void) {
    FILE *fp = fopen(FILE_NAME, "rb");
    Appointment a;
    int maxID = 0;

    if (fp == NULL)
        return 1;

    while (fread(&a, sizeof(Appointment), 1, fp) == 1) {
        if (a.id > maxID)
            maxID = a.id;
    }

    fclose(fp);
    return maxID + 1;
}

void addAppointment(void) {
    FILE *fp = fopen(FILE_NAME, "ab");
    Appointment a;

    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    a.id = generateID();
    a.status = 1;

    printf("\n--- Book Doctor Appointment ---\n");
    printf("Patient name: ");
    scanf(" %49[^\n]", a.patientName);

    printf("Patient age: ");
    scanf("%d", &a.age);

    printf("Gender: ");
    scanf(" %14s", a.gender);

    printf("Doctor name: ");
    scanf(" %49[^\n]", a.doctorName);

    printf("Appointment date (DD-MM-YYYY): ");
    scanf(" %14s", a.date);

    printf("Appointment time (e.g. 10:30AM): ");
    scanf(" %14s", a.time);

    printf("Phone number: ");
    scanf(" %19s", a.phone);

    fwrite(&a, sizeof(Appointment), 1, fp);
    fclose(fp);

    printf("\nAppointment booked successfully!\n");
    printf("Appointment ID: %d\n", a.id);
}

void displayAppointment(Appointment a) {
    printf("\n----------------------------------\n");
    printf("Appointment ID : %d\n", a.id);
    printf("Patient Name   : %s\n", a.patientName);
    printf("Age            : %d\n", a.age);
    printf("Gender         : %s\n", a.gender);
    printf("Doctor         : %s\n", a.doctorName);
    printf("Date           : %s\n", a.date);
    printf("Time           : %s\n", a.time);
    printf("Phone          : %s\n", a.phone);
    printf("Status         : %s\n",
           a.status ? "Booked" : "Cancelled");
    printf("----------------------------------\n");
}

void viewAppointments(void) {
    FILE *fp = fopen(FILE_NAME, "rb");
    Appointment a;
    int found = 0;

    if (fp == NULL) {
        printf("\nNo appointments found.\n");
        return;
    }

    printf("\n======= ALL APPOINTMENTS =======\n");

    while (fread(&a, sizeof(Appointment), 1, fp) == 1) {
        displayAppointment(a);
        found = 1;
    }

    if (!found)
        printf("No appointments found.\n");

    fclose(fp);
}

void searchAppointment(void) {
    FILE *fp = fopen(FILE_NAME, "rb");
    Appointment a;
    int id, found = 0;

    if (fp == NULL) {
        printf("\nNo appointments found.\n");
        return;
    }

    printf("\nEnter appointment ID: ");
    scanf("%d", &id);

    while (fread(&a, sizeof(Appointment), 1, fp) == 1) {
        if (a.id == id) {
            displayAppointment(a);
            found = 1;
            break;
        }
    }

    if (!found)
        printf("\nAppointment ID not found.\n");

    fclose(fp);
}

void cancelAppointment(void) {
    FILE *fp = fopen(FILE_NAME, "rb");
    FILE *temp;
    Appointment a;
    int id, found = 0;

    if (fp == NULL) {
        printf("\nNo appointments found.\n");
        return;
    }

    temp = fopen("temp.dat", "wb");

    if (temp == NULL) {
        printf("Error creating temporary file!\n");
        fclose(fp);
        return;
    }

    printf("\nEnter appointment ID to cancel: ");
    scanf("%d", &id);

    while (fread(&a, sizeof(Appointment), 1, fp) == 1) {
        if (a.id == id) {
            if (a.status == 1) {
                a.status = 0;
                found = 1;
            } else {
                found = 2;
            }
        }

        fwrite(&a, sizeof(Appointment), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    if (remove(FILE_NAME) != 0 ||
        rename("temp.dat", FILE_NAME) != 0) {
        printf("Error updating appointment file!\n");
        return;
    }

    if (found == 1)
        printf("\nAppointment cancelled successfully.\n");
    else if (found == 2)
        printf("\nThis appointment is already cancelled.\n");
    else
        printf("\nAppointment ID not found.\n");
}

int main(void) {
    int choice;

    do {
        printf("\n====================================\n");
        printf("   DENTAL CLINIC MANAGEMENT SYSTEM\n");
        printf("====================================\n");
        printf("1. Book Doctor Appointment\n");
        printf("2. View All Appointments\n");
        printf("3. Search Appointment\n");
        printf("4. Cancel Appointment\n");
        printf("5. Exit\n");
        printf("------------------------------------\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            return 1;
        }

        switch (choice) {
            case 1:
                addAppointment();
                break;

            case 2:
                viewAppointments();
                break;

            case 3:
                searchAppointment();
                break;

            case 4:
                cancelAppointment();
                break;

            case 5:
                printf("\nThank you for using the system!\n");
                break;

            default:
                printf("\nInvalid choice. Try again.\n");
        }

    } while (choice != 5);

    return 0;
}
