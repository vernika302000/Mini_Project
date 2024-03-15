#include <iostream>
#include <string>
#include <sqlite3.h>
#include <random>

// Function to generate a random alphanumeric string of given length
std::string generateRandomString(int length) {
    std::string charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charset.length() - 1);
    for (int i = 0; i < length; ++i) {
        result += charset[dis(gen)];
    }
    return result;
}

// Function to initialize the database and create necessary tables
bool initializeDatabase(sqlite3*& db) {
    int rc = sqlite3_open("url_shortener.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    const char* create_table_query = "CREATE TABLE IF NOT EXISTS urls (id INTEGER PRIMARY KEY, short_code TEXT NOT NULL UNIQUE, original_url TEXT NOT NULL);";
    rc = sqlite3_exec(db, create_table_query, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creating table: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    return true;
}

// Function to shorten a URL
std::string shortenURL(sqlite3* db, const std::string& originalURL) {
    std::string shortCode = generateRandomString(6); // Change the length as needed
    std::string insert_query = "INSERT INTO urls (short_code, original_url) VALUES ('" + shortCode + "', '" + originalURL + "');";
    int rc = sqlite3_exec(db, insert_query.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error inserting URL: " << sqlite3_errmsg(db) << std::endl;
        return "";
    }
    return shortCode;
}

// Function to retrieve the original URL from the short code
std::string getOriginalURL(sqlite3* db, const std::string& shortCode) {
    std::string select_query = "SELECT original_url FROM urls WHERE short_code = '" + shortCode + "';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, select_query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return "";
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        std::cerr << "Error retrieving URL: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return "";
    }
    std::string originalURL = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);
    return originalURL;
}

int main() {
    sqlite3* db;
    if (!initializeDatabase(db)) {
        return 1;
    }

    // Example usage:
    std::string originalURL = "https://example.com/very-long-url";
    std::string shortCode = shortenURL(db, originalURL);
    if (!shortCode.empty()) {
        std::cout << "Shortened URL: http://yourdomain.com/" << shortCode << std::endl;
    }

    // Retrieving original URL
    std::string retrievedURL = getOriginalURL(db, shortCode);
    if (!retrievedURL.empty()) {
        std::cout << "Original URL: " << retrievedURL << std::endl;
    }

    sqlite3_close(db);
    return 0;
}
