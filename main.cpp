#include <iostream>
#include "headers/LOB.h"
#include "headers/DBConnection.h"

int main() {
    // Connection string for the PostgreSQL database
    DBConnection& db = DBConnection::getInstance("dbname=exchange user=myuser password=1123 host=localhost port=5432");
    std::string insertSql = "INSERT INTO my_table (name, age) VALUES ('John Doe', 30), ('Jane Doe', 25);";

    db.query(insertSql);

    std::string selectSql = "SELECT id, name, age FROM my_table;";
    pqxx::result res = db.query(selectSql);

    for (auto row : res) {
        int id = row["id"].as<int>();
        std::string name = row["name"].as<std::string>();
        int age = row["age"].as<int>();
        std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
    }

    return 0;
}
