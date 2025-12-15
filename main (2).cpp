#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <algorithm>

using namespace std;

// -----------------------------
// Utility functions
// -----------------------------
string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    if (start == string::npos || end == string::npos) return "";
    return s.substr(start, end - start + 1);
}

string simpleHash(const string &password) {
    // NOTE: This is a SIMPLE educational hash (NOT secure for real systems)
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

string currentDateTime() {
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    stringstream ss;
    ss << 1900 + ltm->tm_year << "-"
       << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
       << setw(2) << setfill('0') << ltm->tm_mday << " "
       << setw(2) << setfill('0') << ltm->tm_hour << ":"
       << setw(2) << setfill('0') << ltm->tm_min << ":"
       << setw(2) << setfill('0') << ltm->tm_sec;
    return ss.str();
}

// -----------------------------
// User class
// -----------------------------
class User {
public:
    string username;
    string passwordHash;
    string email;
    string createdAt;

    User() {}

    User(string u, string pHash, string e, string c)
        : username(u), passwordHash(pHash), email(e), createdAt(c) {}

    string serialize() const {
        return username + "," + passwordHash + "," + email + "," + createdAt;
    }

    static User deserialize(const string &line) {
        stringstream ss(line);
        string u, p, e, c;
        getline(ss, u, ',');
        getline(ss, p, ',');
        getline(ss, e, ',');
        getline(ss, c, ',');
        return User(trim(u), trim(p), trim(e), trim(c));
    }
};

// -----------------------------
// UserManager class
// -----------------------------
class UserManager {
private:
    string filename;
    vector<User> users;

    void loadFromFile() {
        users.clear();
        ifstream file(filename);
        if (!file.is_open()) return;

        string line;
        while (getline(file, line)) {
            if (!trim(line).empty()) {
                users.push_back(User::deserialize(line));
            }
        }
        file.close();
    }

    void saveToFile() {
        ofstream file(filename, ios::trunc);
        for (const auto &u : users) {
            file << u.serialize() << endl;
        }
        file.close();
    }

public:
    UserManager(const string &file) : filename(file) {
        loadFromFile();
    }

    bool usernameExists(const string &username) {
        return any_of(users.begin(), users.end(), [&](const User &u) {
            return u.username == username;
        });
    }

    bool emailExists(const string &email) {
        return any_of(users.begin(), users.end(), [&](const User &u) {
            return u.email == email;
        });
    }

    bool registerUser(const string &username, const string &password, const string &email) {
        if (usernameExists(username) || emailExists(email)) {
            return false;
        }
        string hash = simpleHash(password);
        User u(username, hash, email, currentDateTime());
        users.push_back(u);
        saveToFile();
        return true;
    }

    bool loginUser(const string &username, const string &password) {
        string hash = simpleHash(password);
        for (const auto &u : users) {
            if (u.username == username && u.passwordHash == hash) {
                return true;
            }
        }
        return false;
    }

    void listUsers() {
        cout << "\nRegistered Users:\n";
        cout << left << setw(15) << "Username"
             << setw(25) << "Email"
             << "Created At" << endl;
        cout << string(60, '-') << endl;
        for (const auto &u : users) {
            cout << left << setw(15) << u.username
                 << setw(25) << u.email
                 << u.createdAt << endl;
        }
    }
};

// -----------------------------
// Menu functions
// -----------------------------
void showMainMenu() {
    cout << "\n====== LOGIN & REGISTRATION SYSTEM ======\n";
    cout << "1. Register\n";
    cout << "2. Login\n";
    cout << "3. List Users (Admin Demo)\n";
    cout << "4. Exit\n";
    cout << "Choose an option: ";
}

void handleRegister(UserManager &manager) {
    string username, password, email;

    cout << "\n--- Registration ---\n";
    cout << "Username: ";
    cin >> username;
    cout << "Email: ";
    cin >> email;
    cout << "Password: ";
    cin >> password;

    if (password.length() < 4) {
        cout << "Password must be at least 4 characters long.\n";
        return;
    }

    if (manager.registerUser(username, password, email)) {
        cout << "Registration successful!\n";
    } else {
        cout << "Username or email already exists.\n";
    }
}

void handleLogin(UserManager &manager) {
    string username, password;

    cout << "\n--- Login ---\n";
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;

    if (manager.loginUser(username, password)) {
        cout << "Login successful. Welcome, " << username << "!\n";
    } else {
        cout << "Invalid username or password.\n";
    }
}

// -----------------------------
// Main function
// -----------------------------
int main() {
    UserManager manager("users.db");
    int choice;

    do {
        showMainMenu();
        cin >> choice;

        switch (choice) {
            case 1:
                handleRegister(manager);
                break;
            case 2:
                handleLogin(manager);
                break;
            case 3:
                manager.listUsers();
                break;
            case 4:
                cout << "Exiting program. Goodbye!\n";
                break;
            default:
                cout << "Invalid option. Try again.\n";
        }
    } while (choice != 4);

    return 0;
}
