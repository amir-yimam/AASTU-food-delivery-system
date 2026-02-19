#ifndef RESTAURANT_H
#define RESTAURANT_H

#include <string>
#include <vector>

// Structure to represent a food item
struct FoodItem {
    std::string name;
    double price;
};

// Structure to represent a restaurant
struct Restaurant {
    std::string name;
    std::vector<FoodItem> foodMenu;
    std::vector<FoodItem> drinkMenu;
};

// Function prototypes
void loadRestaurantsFromDatabase(sqlite3* db, std::vector<Restaurant>& restaurants);

#endif // RESTAURANT_H