//
// Created by Reza Tabrizi on 1/5/24.
//
#include "../headers/DBConnection.h"

std::unique_ptr<DBConnection> DBConnection::instance;
std::mutex DBConnection::mutex;

DBConnection& DBConnection::getInstance(const std::string& connectionString) {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance == nullptr) {
        instance.reset(new DBConnection(connectionString));
    }
    return *instance;
}

pqxx::result DBConnection::query(const std::string& sql) {
    pqxx::work txn(conn);
    pqxx::result res = txn.exec(sql);
    txn.commit();
    return res;
}
