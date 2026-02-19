
#include <iostream>
#include "user.h"
#include "database/database.h"

using namespace std;

void registerUserInDatabase(sqlite3* db, const string& username, const string& password, const string& address) {
    // Implement SQL to insert new user into the Users table
    string sql = "INSERT INTO Users (username, password, address) VALUES ('" + username + "', '" + password + "', '" + address + "');";
    sqlite3_exec(db, sql.c_str(), nullptr, 0, nullptr);
}