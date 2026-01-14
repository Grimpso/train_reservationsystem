#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

#include "sqlite3.h"

using namespace std;

/* ---------- CALLBACK ---------- */
int callback(void*, int argc, char** argv, char** colName) {
    for (int i = 0; i < argc; i++) {
        cout << colName[i] << ": "
             << (argv[i] ? argv[i] : "NULL") << " | ";
    }
    cout << endl;
    return 0;
}

/* ---------- PNR ---------- */
string generatePNR() {
    return "PNR" + to_string(rand() % 90000 + 10000);
}

int main() {
    srand(time(0));

    sqlite3* db;
    char* errMsg = 0;

    sqlite3_open("../database/train.db", &db);

    /* ---------- TABLES ---------- */
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS trains ("
        "train_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "train_name TEXT,"
        "source TEXT,"
        "destination TEXT,"
        "total_seats INTEGER,"
        "available_seats INTEGER);",
        0, 0, &errMsg);

    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS bookings ("
        "booking_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "pnr TEXT,"
        "train_id INTEGER,"
        "seats_booked INTEGER);",
        0, 0, &errMsg);

    int role;
    cout << "1. Admin\n2. User\nSelect role: ";
    cin >> role;

    /* ================= ADMIN ================= */
    if (role == 1) {
        string name, src, dest;
        int seats;

        cout << "Train Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Source: ";
        getline(cin, src);

        cout << "Destination: ";
        getline(cin, dest);

        cout << "Total Seats: ";
        cin >> seats;

        string sql =
            "INSERT INTO trains (train_name, source, destination, total_seats, available_seats) "
            "VALUES ('" + name + "','" + src + "','" + dest + "'," +
            to_string(seats) + "," + to_string(seats) + ");";

        sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
        cout << "-> Train added successfully!\n";
    }

    /* ================= USER ================= */
    else if (role == 2) {
        int choice;
        while (true) {
            cout << "\n1. View Trains\n2. Book Train\n3. Exit\nChoice: ";
            cin >> choice;

            if (choice == 1) {
                sqlite3_exec(db, "SELECT * FROM trains;", callback, 0, &errMsg);
            }

            else if (choice == 2) {
                int trainId, seats;
                cout << "Enter Train ID: ";
                cin >> trainId;
                cout << "Seats to book: ";
                cin >> seats;

                int available = -1;
                auto seatCallback = [](void* data, int argc, char** argv, char**) {
                    if (argv[0]) *(int*)data = stoi(argv[0]);
                    return 0;
                };

                string check =
                    "SELECT available_seats FROM trains WHERE train_id=" +
                    to_string(trainId) + ";";

                sqlite3_exec(db, check.c_str(), seatCallback, &available, &errMsg);

                if (available < seats || available == -1) {
                    cout << "-> Seats not available\n";
                } else {
                    string update =
                        "UPDATE trains SET available_seats = available_seats - " +
                        to_string(seats) +
                        " WHERE train_id=" + to_string(trainId) + ";";

                    sqlite3_exec(db, update.c_str(), 0, 0, &errMsg);

                    string pnr = generatePNR();
                    string book =
                        "INSERT INTO bookings (pnr, train_id, seats_booked) "
                        "VALUES ('" + pnr + "'," +
                        to_string(trainId) + "," +
                        to_string(seats) + ");";

                    sqlite3_exec(db, book.c_str(), 0, 0, &errMsg);

                    cout << "-> Booking successful\n";
                    cout << "-> PNR: " << pnr << endl;
                }
            }

            else if (choice == 3) break;
        }
    }

    sqlite3_close(db);
    return 0;
}
