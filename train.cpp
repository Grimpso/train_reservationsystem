#include <iostream>
#include "sqlite3.h"
#include <string>

using namespace std;

int callback(void* NotUsed, int argc, char** argv, char** colName) {
    for (int i = 0; i < argc; i++) {
        cout << colName[i] << ": "
             << (argv[i] ? argv[i] : "NULL") << " | ";
    }
    cout << endl;
    return 0;
}

int main() {
    sqlite3* db;
    char* errMsg = 0;

    int rc = sqlite3_open("train.db", &db);
    if (rc != SQLITE_OK) {
        cout << "Database open failed\n";
        return 0;
    }

    int choice;
    while (true) {
        cout << "\n--- TRAIN RESERVATION SYSTEM ---\n";
        cout << "1. Show Trains\n";
        cout << "2. Book Ticket\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            sqlite3_exec(db, "SELECT * FROM trains;", callback, 0, &errMsg);
        }

        else if (choice == 2) {
            int trainId, seats;
            cout << "Enter Train ID: ";
            cin >> trainId;
            cout << "Enter seats to book: ";
            cin >> seats;

            string checkSQL =
                "SELECT available_seats FROM trains WHERE train_id = " +
                to_string(trainId) + ";";

            int availableSeats = -1;

            auto seatCallback = [](void* data, int argc, char** argv, char**) -> int {
                int* seats = (int*)data;
                if (argv[0])
                    *seats = stoi(argv[0]);
                return 0;
            };

            sqlite3_exec(db, checkSQL.c_str(), seatCallback, &availableSeats, &errMsg);

            if (availableSeats == -1) {
                cout << "Invalid Train ID!\n";
            }
            else if (availableSeats < seats) {
                cout << "Not enough seats available!\n";
            }
            else {
                string updateSQL =
                    "UPDATE trains SET available_seats = available_seats - " +
                    to_string(seats) +
                    " WHERE train_id = " + to_string(trainId) + ";";

                sqlite3_exec(db, updateSQL.c_str(), 0, 0, &errMsg);
                cout << "✅ Ticket booked successfully!\n";
            }
        }

        else if (choice == 3) {
            cout << "Exiting system...\n";
            break;
        }

        else {
            cout << "Invalid choice!\n";
        }
    }

    sqlite3_close(db);
    return 0;
}
