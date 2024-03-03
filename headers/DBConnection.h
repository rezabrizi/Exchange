#pragma once
#include "exchange_common.h"

class DBConnection {
private:
    pqxx::connection conn;
    static inline std::unique_ptr<DBConnection> instance;
    static inline std::mutex instanceMux;
    std:: mutex queryMux;

    DBConnection(const std::string& connectionString) : conn(connectionString){}

public:
    DBConnection(DBConnection const&) = delete;
    DBConnection(DBConnection&&) = delete;
    DBConnection& operator=(DBConnection const&) = delete;
    DBConnection& operator=(DBConnection &&) = delete;

    static DBConnection& getInstance(const std::string& connectionString) {
        std::lock_guard<std::mutex> lock(instanceMux);
        if (instance == nullptr) {
            instance.reset(new DBConnection(connectionString));
        }
        return *instance;
    }
    pqxx::result query(const std::string& sql) {
        std::lock_guard<std::mutex> lock (queryMux);
        pqxx::work txn(conn);
        pqxx::result res = txn.exec(sql);
        txn.commit();
        return res;
    }
};

