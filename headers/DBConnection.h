//
// Created by Reza Tabrizi on 1/5/24.
//

#include <pqxx/pqxx>
#include <mutex>

#ifndef LIMITORDERBOOK_DBCONNECTION_H
#define LIMITORDERBOOK_DBCONNECTION_H

class DBConnection {
private:
    pqxx::connection conn;
    static std::unique_ptr<DBConnection> instance;
    static std::mutex mutex;

    DBConnection(const std::string& connectionString) : conn(connectionString){}

public:
    DBConnection(DBConnection const&) = delete;
    DBConnection(DBConnection&&) = delete;
    DBConnection& operator=(DBConnection const&) = delete;
    DBConnection& operator=(DBConnection &&) = delete;

    static DBConnection& getInstance (const std::string& connectionString);

    pqxx::result query(const std::string& sql);
};

#endif //LIMITORDERBOOK_DBCONNECTION_H
