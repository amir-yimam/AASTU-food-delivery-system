#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include <utility>
#include "restaurant.h"

// Structure to represent a user
struct User {
    std::string username;
    std::string password;
    std::string address;
    std::vector<std::pair<Restaurant, std::vector<std::pair<std::string, int>>>> orderHistory; // Changed to vector for items
};

// Function prototypes
void registerUserInDatabase(sqlite3* db, const std::string& username, const std::string& password, const std::string& address);

#endif // USER_H