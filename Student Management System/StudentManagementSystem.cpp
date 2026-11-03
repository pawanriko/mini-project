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
#include <cstdlib>
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

string toLowerStr(string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

bool containsCI(const string& haystack, const string& needle) {
    return toLowerStr(haystack).find(toLowerStr(needle)) != string::npos;
}

// Input ended (Ctrl+D / Ctrl+Z / a piped script ran dry): leave
// cleanly instead of looping forever or inventing sentinel values.
[[noreturn]] void exitOnEof() {
    cout << "\n  Input closed (EOF) — exiting.\n";
    exit(0);
}

// Strict whole-string integer parse: "12abc" and "" are NOT numbers.
bool parseIntStrict(const string& raw, int& out) {
    string s = trim(raw);
    if (s.empty()) return false;
    try {
        size_t pos = 0;
        int v = stoi(s, &pos);
        if (pos != s.size()) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

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
        if (cin.eof()) exitOnEof();
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
        if (cin.eof()) exitOnEof();
        cout << "  [!] This field cannot be empty.\n";
    }
}

// For fields that get saved to the CSV file (name, course): a comma
// inside the value would corrupt the record's columns, so refuse it
// here instead of silently losing the record on the next load.
string readField(const string& prompt) {
    while (true) {
        string value = readText(prompt);
        if (value.find(',') == string::npos) return value;
        cout << "  [!] Commas aren't allowed in this field — they'd break the save file. Try again without one.\n";
    }
}

// --------------------------- storage --------------------------
// File format (unchanged): id,name,age,course per line.

vector<Student> loadRecords() {
    vector<Student> students;
    ifstream file(FILE_NAME);
    if (!file.is_open()) return students;

    string row;
    bool firstLine = true;
    while (getline(file, row)) {
        if (firstLine) {
            firstLine = false;
            // A UTF-8 BOM (left by some Windows editors) must not eat record #1.
            if (row.size() >= 3 && row.compare(0, 3, "\xEF\xBB\xBF") == 0) row.erase(0, 3);
        }
        row = trim(row);
        if (row.empty()) continue;

        stringstream ss(row);
        string idPart, name, agePart, course;
        if (!getline(ss, idPart, ',')) continue;
        if (!getline(ss, name, ',')) continue;
        if (!getline(ss, agePart, ',')) continue;
        if (!getline(ss, course)) continue;
        if (course.find(',') != string::npos) continue; // extra fields — not our format

        Student s;
        if (!parseIntStrict(idPart, s.id)) continue;   // "12abc" is not an ID
        if (!parseIntStrict(agePart, s.age)) continue;
        s.name = trim(name);
        s.course = trim(course);
        if (s.name.empty() || s.course.empty()) continue;
        if (s.age < 1 || s.age > 120) continue;
        students.push_back(s);
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
    s.name = readField("  Name         : ");
    s.age = readAge("  Age          : ");
    s.course = readField("  Course       : ");

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

    cout << "    1.  By ID (exact)\n";
    cout << "    2.  By Name (part of the name is enough)\n";
    cout << "    3.  By Course (part of the course is enough)\n\n";
    int by = readInt("  Search by (1-3): ");

    if (by == 1) {
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
        return;
    }

    if (by == 2 || by == 3) {
        string term = readText(by == 2 ? "  Enter name to search: " : "  Enter course to search: ");
        vector<Student> matches;
        for (const auto& s : students) {
            if (by == 2 ? containsCI(s.name, term) : containsCI(s.course, term)) {
                matches.push_back(s);
            }
        }
        if (matches.empty()) {
            cout << "  [!] No student matches \"" << term << "\".\n";
            return;
        }
        cout << "\n";
        printTableHeader();
        for (const auto& s : matches) printRow(s);
        cout << "  " << line('-') << "\n";
        cout << "  " << matches.size() << " match(es) for \"" << term << "\".\n";
        return;
    }

    cout << "  [!] Please choose 1, 2 or 3.\n";
}

void showStatistics(const vector<Student>& students) {
    sectionTitle("Statistics");

    if (students.empty()) {
        cout << "  No records yet — nothing to summarise.\n";
        return;
    }

    long ageSum = 0;
    const Student* youngest = &students[0];
    const Student* oldest = &students[0];
    vector<pair<string, int>> courseCounts; // in first-seen order

    for (const auto& s : students) {
        ageSum += s.age;
        if (s.age < youngest->age) youngest = &s;
        if (s.age > oldest->age) oldest = &s;
        bool seen = false;
        for (auto& p : courseCounts) {
            if (toLowerStr(p.first) == toLowerStr(s.course)) { p.second++; seen = true; break; }
        }
        if (!seen) courseCounts.push_back({s.course, 1});
    }

    sort(courseCounts.begin(), courseCounts.end(),
         [](const pair<string, int>& a, const pair<string, int>& b) { return a.second > b.second; });

    cout << "  Total students : " << students.size() << "\n";
    cout << "  Average age    : " << fixed << setprecision(1)
         << (double)ageSum / students.size() << " years\n";
    cout << "  Youngest       : " << youngest->name << " (" << youngest->age << ")\n";
    cout << "  Oldest         : " << oldest->name << " (" << oldest->age << ")\n";
    cout << "\n  Students per course:\n";

    int maxCount = courseCounts[0].second;
    for (const auto& p : courseCounts) {
        int barLen = maxCount ? (p.second * 24) / maxCount : 0;
        cout << "    " << left << setw(22) << p.first
             << string(barLen, '#') << " " << p.second << "\n";
    }
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

    s->name = readField("  Name         : ");
    s->age = readAge("  Age          : ");
    s->course = readField("  Course       : ");

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
        cout << "    6.  Statistics\n";
        cout << "    7.  Exit\n";
        cout << "\n  " << line('-') << "\n";

        choice = readInt("  Choose an option (1-7): ");

        switch (choice) {
            case 1: addStudent(students); pause(); break;
            case 2: searchStudent(students); pause(); break;
            case 3: updateStudent(students); pause(); break;
            case 4: deleteStudent(students); pause(); break;
            case 5: viewAllStudents(students); pause(); break;
            case 6: showStatistics(students); pause(); break;
            case 7:
                cout << "\n  Goodbye. " << students.size()
                     << " record(s) saved in " << FILE_NAME << ".\n\n";
                break;
            default:
                cout << "  [!] Please choose a number from 1 to 7.\n";
                pause();
                break;
        }
    } while (choice != 7 && !cin.eof());

    return 0;
}
