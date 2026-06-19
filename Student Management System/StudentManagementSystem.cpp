#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <cctype>

using namespace std;

class Student {
public:
    int id;
    string name;
    int age;
    string course;

    void display() const {
        cout << left << setw(10) << id 
             << setw(25) << name 
             << setw(10) << age 
             << setw(20) << course << endl;
    }

    void saveToFile(ofstream &out) const {
        out << id << "," << name << "," << age << "," << course << "\n";
    }
};

const string FILE_NAME = "students.txt";

int getValidInteger(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            return value;
        } else {
            cout << "Invalid input! Please enter a valid number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

int getValidAge(const string& prompt) {
    while (true) {
        int age = getValidInteger(prompt);
        if (age >= 1 && age <= 120) {
            return age;
        }
        cout << "Invalid age! Age must be between 1 and 120.\n";
    }
}

string getValidString(const string& prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        size_t first = input.find_first_not_of(" \t\n\r");
        if (first != string::npos) {
            size_t last = input.find_last_not_of(" \t\n\r");
            return input.substr(first, (last - first + 1));
        }
        cout << "Input cannot be empty or just spaces! Please try again.\n";
    }
}

vector<Student> loadRecords() {
    vector<Student> students;
    ifstream file(FILE_NAME);
    if (!file.is_open()) return students;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        try {
            Student s;
            size_t pos = 0;
            string temp = line;
            
            pos = temp.find(',');
            if (pos == string::npos) continue;
            s.id = stoi(temp.substr(0, pos));
            temp.erase(0, pos + 1);

            pos = temp.find(',');
            if (pos == string::npos) continue;
            s.name = temp.substr(0, pos);
            if (s.name.empty()) continue;
            temp.erase(0, pos + 1);

            pos = temp.find(',');
            if (pos == string::npos) continue;
            s.age = stoi(temp.substr(0, pos));
            if (s.age < 1 || s.age > 120) continue;
            temp.erase(0, pos + 1);

            s.course = temp;
            if (s.course.empty()) continue;

            students.push_back(s);
        }
        catch (...) {
            continue;
        }
    }
    file.close();
    return students;
}

void saveAllRecords(const vector<Student>& students) {
    ofstream file(FILE_NAME, ios::trunc);
    if (!file.is_open()) {
        cout << "Error opening file for writing!\n";
        return;
    }
    for (const auto& s : students) {
        s.saveToFile(file);
    }
    file.close();
}

bool compareById(const Student& a, const Student& b) {
    return a.id < b.id;
}

void addStudent() {
    vector<Student> students = loadRecords();
    Student s;

    cout << "\n--- Add New Student ---\n";
    s.id = getValidInteger("Enter Student ID: ");
    
    for (const auto& student : students) {
        if (student.id == s.id) {
            cout << "Error: A student with ID " << s.id << " already exists!\n";
            return;
        }
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    s.name = getValidString("Enter Name: ");
    s.age = getValidAge("Enter Age: ");
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    s.course = getValidString("Enter Course: ");

    students.push_back(s);
    sort(students.begin(), students.end(), compareById);
    saveAllRecords(students);
    cout << "Student added and record saved successfully!\n";
}

void searchStudent() {
    vector<Student> students = loadRecords();
    int searchId = getValidInteger("\nEnter Student ID to search: ");

    for (const auto& s : students) {
        if (s.id == searchId) {
            cout << "\nStudent Found:\n";
            cout << left << setw(10) << "ID" << setw(25) << "Name" << setw(10) << "Age" << setw(20) << "Course" << endl;
            cout << string(65, '-') << endl;
            s.display();
            return;
        }
    }
    cout << "Student with ID " << searchId << " not found.\n";
}

void updateStudent() {
    vector<Student> students = loadRecords();
    int updateId = getValidInteger("\nEnter Student ID to update: ");
    bool found = false;

    for (auto& s : students) {
        if (s.id == updateId) {
            found = true;
            cout << "\nExisting details:\n";
            s.display();
            cout << "\nEnter New Details:\n";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            s.name = getValidString("Enter New Name: ");
            s.age = getValidAge("Enter New Age: ");
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            s.course = getValidString("Enter New Course: ");
            break;
        }
    }

    if (found) {
        saveAllRecords(students);
        cout << "Record updated successfully!\n";
    } else {
        cout << "Student with ID " << updateId << " not found.\n";
    }
}

void deleteStudent() {
    vector<Student> students = loadRecords();
    int deleteId = getValidInteger("\nEnter Student ID to delete: ");

    auto it = remove_if(students.begin(), students.end(), [deleteId](const Student& s) {
        return s.id == deleteId;
    });

    if (it != students.end()) {
        students.erase(it, students.end());
        saveAllRecords(students);
        cout << "Record deleted successfully!\n";
    } else {
        cout << "Student with ID " << deleteId << " not found.\n";
    }
}

void viewAllStudents() {
    vector<Student> students = loadRecords();
    if (students.empty()) {
        cout << "\nNo valid records found.\n";
        return;
    }

    cout << "\n" << string(65, '=') << "\n";
    cout << left << setw(10) << "ID" << setw(25) << "Name" << setw(10) << "Age" << setw(20) << "Course" << endl;
    cout << string(65, '=') << "\n";
    
    for (const auto& s : students) {
        s.display();
    }
    cout << string(65, '=') << "\n";
}

int main() {
    int choice;
    do {
        cout << "\n===============================\n";
        cout << "   STUDENT MANAGEMENT SYSTEM   \n";
        cout << "===============================\n";
        cout << "1. Add Student\n";
        cout << "2. Search Student\n";
        cout << "3. Update Student\n";
        cout << "4. Delete Student\n";
        cout << "5. View All Students\n";
        cout << "6. Exit\n";
        
        choice = getValidInteger("Enter your choice (1-6): ");

        switch (choice) {
            case 1: addStudent(); break;
            case 2: searchStudent(); break;
            case 3: updateStudent(); break;
            case 4: deleteStudent(); break;
            case 5: viewAllStudents(); break;
            case 6: cout << "\nExiting program...\n"; break;
            default: cout << "\nInvalid choice! Please try again.\n";
        }
    } while (choice != 6);

    return 0;
}