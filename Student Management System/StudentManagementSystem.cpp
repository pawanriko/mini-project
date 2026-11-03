// ============================================================
//  Student Management System
//  A console app to add, search, update, delete and view
//  student records. Records are stored in students.txt
//  (one CSV line per student: id,name,age,course), always
//  kept sorted by ID.
//
//  Build:  g++ -std=c++17 -o sms StudentManagementSystem.cpp
//  Run:    ./sms        (students.txt is created next to it)
// ============================================================

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ---------------------------- data ----------------------------

struct Student {
    int id;
    string name;
    int age;
    string course;
};

const string FILE_NAME = "students.txt";
const int TABLE_WIDTH = 68;

// ------------------------- small helpers ----------------------

string trim(const string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

string line(char c, int n = TABLE_WIDTH) { return string(n, c); }

void clearScreen() {
    // ANSI clear + home; harmless if the terminal ignores it.
    cout << "\033[2J\033[H";
}

void pause() {
    cout << "\n  Press Enter to continue...";
    string dummy;
    getline(cin, dummy); // the stream is already at a line boundary
}

// ------------------------- input handling ---------------------

int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        if (cin.eof()) return -1;
        cout << "  [!] Please enter a whole number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int readAge(const string& prompt) {
    while (true) {
        int age = readInt(prompt);
        if (age >= 1 && age <= 120) return age;
        cout << "  [!] Age must be between 1 and 120.\n";
    }
}

string readText(const string& prompt) {
    while (true) {
        cout << prompt;
        string input;
        getline(cin, input);
        input = trim(input);
        if (!input.empty()) return input;
        cout << "  [!] This field cannot be empty.\n";
        if (cin.eof()) return "";
    }
}

// --------------------------- storage --------------------------
// File format (unchanged): id,name,age,course per line.

vector<Student> loadRecords() {
    vector<Student> students;
    ifstream file(FILE_NAME);
    if (!file.is_open()) return students;

    string row;
    while (getline(file, row)) {
        row = trim(row);
        if (row.empty()) continue;

        try {
            stringstream ss(row);
            string idPart, name, agePart, course;
            if (!getline(ss, idPart, ',')) continue;
            if (!getline(ss, name, ',')) continue;
            if (!getline(ss, agePart, ',')) continue;
            if (!getline(ss, course)) continue;

            Student s;
            s.id = stoi(trim(idPart));
            s.name = trim(name);
            s.age = stoi(trim(agePart));
            s.course = trim(course);
            if (s.name.empty() || s.course.empty()) continue;
            if (s.age < 1 || s.age > 120) continue;
            students.push_back(s);
        } catch (...) {
            continue; // skip malformed lines, never crash on bad data
        }
    }
    return students;
}

bool saveRecords(const vector<Student>& students) {
    ofstream file(FILE_NAME, ios::trunc);
    if (!file.is_open()) {
        cout << "  [!] Could not open " << FILE_NAME << " for writing.\n";
        return false;
    }
    for (const auto& s : students) {
        file << s.id << "," << s.name << "," << s.age << "," << s.course << "\n";
    }
    return true;
}

Student* findById(vector<Student>& students, int id) {
    for (auto& s : students) {
        if (s.id == id) return &s;
    }
    return nullptr;
}

// ----------------------------- UI -----------------------------

void printBanner() {
    cout << "\n  " << line('=') << "\n";
    cout << "  " << setw(44) << right << "STUDENT MANAGEMENT SYSTEM" << "\n";
    cout << "  " << line('=') << "\n";
}

void printTableHeader() {
    cout << "  " << line('-') << "\n";
    cout << "  " << left << setw(8) << "ID"
         << setw(26) << "Name"
         << setw(8) << "Age"
         << setw(24) << "Course" << "\n";
    cout << "  " << line('-') << "\n";
}

void printRow(const Student& s) {
    cout << "  " << left << setw(8) << s.id
         << setw(26) << (s.name.size() > 24 ? s.name.substr(0, 23) + "…" : s.name)
         << setw(8) << s.age
         << setw(24) << (s.course.size() > 22 ? s.course.substr(0, 21) + "…" : s.course)
         << "\n";
}

void sectionTitle(const string& title) {
    cout << "\n  --- " << title << " " << line('-', TABLE_WIDTH - 6 - (int)title.size()) << "\n\n";
}

// -------------------------- operations ------------------------

void addStudent(vector<Student>& students) {
    sectionTitle("Add New Student");

    int id = readInt("  Student ID   : ");
    if (findById(students, id)) {
        cout << "  [!] A student with ID " << id << " already exists.\n";
        return;
    }

    Student s;
    s.id = id;
    s.name = readText("  Name         : ");
    s.age = readAge("  Age          : ");
    s.course = readText("  Course       : ");

    students.push_back(s);
    sort(students.begin(), students.end(),
         [](const Student& a, const Student& b) { return a.id < b.id; });

    if (saveRecords(students)) {
        cout << "\n  [OK] " << s.name << " (ID " << s.id << ") added. "
             << students.size() << " record(s) on file.\n";
    }
}

void searchStudent(const vector<Student>& students) {
    sectionTitle("Search Student");

    int id = readInt("  Enter ID to search: ");
    for (const auto& s : students) {
        if (s.id == id) {
            cout << "\n";
            printTableHeader();
            printRow(s);
            cout << "  " << line('-') << "\n";
            return;
        }
    }
    cout << "  [!] No student found with ID " << id << ".\n";
}

void updateStudent(vector<Student>& students) {
    sectionTitle("Update Student");

    int id = readInt("  Enter ID to update: ");
    Student* s = findById(students, id);
    if (!s) {
        cout << "  [!] No student found with ID " << id << ".\n";
        return;
    }

    cout << "\n  Current record:\n";
    printTableHeader();
    printRow(*s);
    cout << "  " << line('-') << "\n\n";
    cout << "  Enter the new details:\n";

    s->name = readText("  Name         : ");
    s->age = readAge("  Age          : ");
    s->course = readText("  Course       : ");

    if (saveRecords(students)) {
        cout << "\n  [OK] Record for ID " << id << " updated.\n";
    }
}

void deleteStudent(vector<Student>& students) {
    sectionTitle("Delete Student");

    int id = readInt("  Enter ID to delete: ");
    Student* s = findById(students, id);
    if (!s) {
        cout << "  [!] No student found with ID " << id << ".\n";
        return;
    }

    cout << "  About to delete: " << s->name << " (ID " << id << ", " << s->course << ")\n";
    string confirm = readText("  Type 'y' to confirm: ");
    if (confirm != "y" && confirm != "Y") {
        cout << "  Cancelled — nothing was deleted.\n";
        return;
    }

    students.erase(remove_if(students.begin(), students.end(),
                             [id](const Student& st) { return st.id == id; }),
                   students.end());

    if (saveRecords(students)) {
        cout << "  [OK] Record deleted. " << students.size() << " record(s) remain.\n";
    }
}

void viewAllStudents(const vector<Student>& students) {
    sectionTitle("All Students");

    if (students.empty()) {
        cout << "  No records yet. Use option 1 to add the first student.\n";
        return;
    }

    printTableHeader();
    for (const auto& s : students) printRow(s);
    cout << "  " << line('-') << "\n";
    cout << "  Total: " << students.size() << " student(s)\n";
}

// ----------------------------- main ---------------------------

int main() {
    vector<Student> students = loadRecords(); // loaded once, kept in memory

    int choice;
    do {
        clearScreen();
        printBanner();
        cout << "  Records on file: " << students.size() << "\n\n";
        cout << "    1.  Add Student\n";
        cout << "    2.  Search Student\n";
        cout << "    3.  Update Student\n";
        cout << "    4.  Delete Student\n";
        cout << "    5.  View All Students\n";
        cout << "    6.  Exit\n";
        cout << "\n  " << line('-') << "\n";

        choice = readInt("  Choose an option (1-6): ");

        switch (choice) {
            case 1: addStudent(students); pause(); break;
            case 2: searchStudent(students); pause(); break;
            case 3: updateStudent(students); pause(); break;
            case 4: deleteStudent(students); pause(); break;
            case 5: viewAllStudents(students); pause(); break;
            case 6:
                cout << "\n  Goodbye. " << students.size()
                     << " record(s) saved in " << FILE_NAME << ".\n\n";
                break;
            default:
                cout << "  [!] Please choose a number from 1 to 6.\n";
                pause();
                break;
        }
    } while (choice != 6 && !cin.eof());

    return 0;
}
