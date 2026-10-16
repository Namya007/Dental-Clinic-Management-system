# Dental Clinic Management System — Doctor & Appointment Module

This folder contains a standalone C module for the doctor's directory and appointment management.

## Features
- Seeds a directory of 12 doctors on first run.
- Books appointments with a patient ID, patient name, doctor, date, time, optional phone and reason.
- Validates date (`YYYY-MM-DD`) and time (24-hour `HH:MM`) formats.
- Prevents two active appointments for the same doctor at the same date and time.
- Lists and searches appointments by appointment ID, patient ID, doctor ID or date.
- Cancels appointments while retaining the record for history.
- Shows appointments for a selected doctor.

## Files created when the program runs
- `doctors.dat`: local binary doctor directory.
- `appointments.dat`: local binary appointment records.
- `appointments.tmp`: temporary file used while cancelling an appointment; normally removed.

These data files are created in the program's current working directory. Keep them backed up. Do not commit real patient data to a public GitHub repository.

## Compile and run

### GCC (Linux/macOS)
```bash
gcc -std=c11 -Wall -Wextra -pedantic main.c -o dental_clinic
./dental_clinic
```

### Windows (MinGW GCC)
```bash
gcc -std=c11 -Wall -Wextra -pedantic main.c -o dental_clinic.exe
dental_clinic.exe
```

If your compiler doesn't support C11, C99 should also work with this code.

## Team integration notes
This is currently a standalone module:
- The booking screen asks for patient ID and patient name manually. Integrate with your teammate's patient module by calling their lookup function and retrieving the registered patient's ID/name instead.
- Billing can use the appointment ID to refer to an appointment. Agree on a shared header/record format with your teammate before merging modules.
- Binary `.dat` files are local storage, not a shared database. Each teammate's copy of the program has separate local data unless you deliberately use a shared database or transfer data.
- Do not upload real patient names, phone numbers, or appointment records to this public repository.

## Doctor directory
The initial directory uses the 12 doctors supplied in the project description. The role/specialization labels are based on the supplied list and may be edited if the clinic provides official wording.

## Important limitation
The program prevents duplicate slots only within the same local `appointments.dat` file. It is intended as a student project, not a production clinical system. It does not provide authentication, encryption, concurrent multi-user access, or medical-record security.
