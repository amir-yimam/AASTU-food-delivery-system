Amir, [5/11/2025 11:18 PM]
#include <iostream>
#include <sqlite3.h>
#include <vector>
#include "database.h"

using namespace std;

// Function to connect to the SQLite database
sqlite3* connectToDatabase(const string& dbName) {
    sqlite3* db;
    int exit = sqlite3_open(dbName.c_str(), &db);
    if (exit) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return nullptr;
    }
    return db;
}

// Function to create tables if they do not exist
void createTables(sqlite3* db) {
    const char* createUsersTable = 
        "CREATE TABLE IF NOT EXISTS Users ("
        "user_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL UNIQUE, "
        "password TEXT NOT NULL, "
        "address TEXT NOT NULL);";

    const char* createRestaurantsTable = 
        "CREATE TABLE IF NOT EXISTS Restaurants ("
        "restaurant_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE);";

    const char* createFoodItemsTable = 
        "CREATE TABLE IF NOT EXISTS FoodItems ("
        "food_item_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "restaurant_id INTEGER, "
        "name TEXT NOT NULL, "
        "price REAL NOT NULL, "
        "FOREIGN KEY(restaurant_id) REFERENCES Restaurants(restaurant_id));";

    const char* createDrinkItemsTable = 
        "CREATE TABLE IF NOT EXISTS DrinkItems ("
        "drink_item_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "restaurant_id INTEGER, "
        "name TEXT NOT NULL, "
        "price REAL NOT NULL, "
        "FOREIGN KEY(restaurant_id) REFERENCES Restaurants(restaurant_id));";

    const char* createOrderHistoryTable = 
        "CREATE TABLE IF NOT EXISTS OrderHistory ("
        "order_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER, "
        "restaurant_id INTEGER, "
        "order_time DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "total_price REAL, "
        "FOREIGN KEY(user_id) REFERENCES Users(user_id), "
        "FOREIGN KEY(restaurant_id) REFERENCES Restaurants(restaurant_id));";
    
    sqlite3_exec(db, createUsersTable, nullptr, 0, nullptr);
    sqlite3_exec(db, createRestaurantsTable, nullptr, 0, nullptr);
    sqlite3_exec(db, createFoodItemsTable, nullptr, 0, nullptr);
    sqlite3_exec(db, createDrinkItemsTable, nullptr, 0, nullptr);
    sqlite3_exec(db, createOrderHistoryTable, nullptr, 0, nullptr);
}

// Function to load data from vectors to the database
void loadDataToDatabase(sqlite3* db, const vector<User>& users, const vector<Restaurant>& restaurants) {
    for (const auto& user : users) {
        string sql = "INSERT OR IGNORE INTO Users (username, password, address) VALUES ('" + 
                     user.username + "', '" + user.password + "', '" + user.address + "');";
        sqlite3_exec(db, sql.c_str(), nullptr, 0, nullptr);
    }

    for (const auto& restaurant : restaurants) {
        string sql = "INSERT OR IGNORE INTO Restaurants (name) VALUES ('" + restaurant.name + "');";
        sqlite3_exec(db, sql.c_str(), nullptr, 0, nullptr);

        // Load food items for the restaurant
        for (const auto& food : restaurant.foodMenu) {
            string foodSql = "INSERT OR IGNORE INTO FoodItems (restaurant_id, name, price) VALUES "
                             "((SELECT restaurant_id FROM Restaurants WHERE name = '" + restaurant.name + "'), '" + 
                             food.name + "', " + to_string(food.price) + ");";
            sqlite3_exec(db, foodSql.c_str(), nullptr, 0, nullptr);
        }

        // Load drink items for the restaurant
        for (const auto& drink : restaurant.drinkMenu) {
            string drinkSql = "INSERT OR IGNORE INTO DrinkItems (restaurant_id, name, price) VALUES "
                              "((SELECT restaurant_id FROM Restaurants WHERE name = '" + restaurant.name + "'), '" + 
                              drink.name + "', " + to_string(drink.price) + ");";
            sqlite3_exec(db, drinkSql.c_str(), nullptr, 0, nullptr);
        }
    }
}


// Function to load data from the database into vectors
void loadDataFromDatabase(sqlite3* db, vector<User>& users, vector<Restaurant>& restaurants) {
    const char* selectUsers = "SELECT * FROM Users;";
    sqlite3_exec(db, selectUsers, [](void* data, int argc, char argv, char azColName) {
        vector<User>* users = static_cast<vector<User>*>(data);
        User user;
        user.username = argv[1] ? argv[1] : "NULL";
        user.password = argv[2] ? argv[2] : "NULL";
        user.address = argv[3] ? argv[3] : "NULL";
        users->push_back(user);
        return 0;
    }, static_cast<void*>(&users), nullptr);

    const char* selectRestaurants = "SELECT * FROM Restaurants;";
    sqlite3_exec(db, selectRestaurants, [](void* data, int argc, char argv, char azColName) {
        vector<Restaurant>* restaurants = static_cast<vector<Restaurant>*>(data);
        Restaurant restaurant;
        restaurant.name = argv[1] ? argv[1] : "NULL";

        // Load food items for each restaurant
        const char* selectFoodItems = ("SELECT name, price FROM FoodItems WHERE restaurant_id = (SELECT restaurant_id FROM Restaurants WHERE name = '" + string(restaurant.name) + "');").c_str();
        sqlite3_exec(db, selectFoodItems, [](void* data, int argc, char argv, char azColName) {
            Restaurant* restaurant = static_cast<Restaurant*>(data);
            if (argc > 0) {
                FoodItem food;
                food.name = argv[0] ? argv[0] : "NULL";
                food.price = argv[1] ? atof(argv[1]) : 0.0;
                restaurant->foodMenu.push_back(food);
            }
            return 0;
        }, static_cast<void*>(&restaurant), nullptr);

        // Load drink items similarly
        const char* selectDrinkItems = ("SELECT name, price FROM DrinkItems WHERE restaurant_id = (SELECT restaurant_id FROM Restaurants WHERE name = '" + string(restaurant.name) + "');").c_str();
        sqlite3_exec(db, selectDrinkItems, [](void* data, int argc, char argv, char azColName) {
            Restaurant* restaurant = static_cast<Restaurant*>(data);
            if (argc > 0) {
                FoodItem drink;
                drink.name = argv[0] ? argv[0] : "NULL";
                drink.price = argv[1] ? atof(argv[1]) : 0.0;
                restaurant->drinkMenu.push_back(drink);
            }
            return 0;
        }, static_cast<void*>(&restaurant), nullptr);

        restaurants->push_back(restaurant);
        return 0;
    }, static_cast<void*>(&restaurants), nullptr);
}

// Function to insert an order into the OrderHistory table
void insertOrder(sqlite3* db, int userId, int restaurantId, double totalPrice) {
    string sql = "INSERT INTO OrderHistory (user_id, restaurant_id, total_price) VALUES (" +
                 to_string(userId) + ", " + to_string(restaurantId) + ", " + to_string(totalPrice) + ");";
    sqlite3_exec(db, sql.c_str(), nullptr, 0, nullptr);
}

// Function to load order history into the database
void loadOrderHistory(sqlite3* db, const vector<User>& users) {
    for (const auto& user : users) {
        for (const auto& order : user.orderHistory) {
            int userId = /* Get userId for user.username */;
            int restaurantId = /* Get restaurantId for order.first.name */;
            double totalPrice = 0.0;
            for (const auto& item : order.second) {
                totalPrice += item.second * /* Price lookup for item.first */;
            }
            insertOrder(db, userId, restaurantId, totalPrice);
        }
    }
}

// Function to close the database connection
void closeDatabase(sqlite3* db) {
    sqlite3_close(db);
}