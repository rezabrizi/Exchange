#pragma once
#include "exchange_common.h"

class DBConnection {
public:

    /**
     * @brief Constructor for DBConnection
     * @param connectionString
     */
    DBConnection(const std::string& connectionString) : conn(connectionString){}
    /**
     * @brief DBConnection constructor
     */
    DBConnection(DBConnection const&) = delete;


    /**
     * @brief delete DBConnection move constructor
     */
    DBConnection(DBConnection&&) = delete;


    /**
     * @brief delete the copy assignment for DBConnection class
     * @return DBConnection object reference
     */
    DBConnection& operator=(DBConnection const&) = delete;


    /**
     * @brief delete the move assignment for DBConnection class
     * @return DBConnection object reference
     */
    DBConnection& operator=(DBConnection &&) = delete;


    /**
     * @brief static method to get an instance of a DBconnection class
     * @param connectionString string to use for Connection to the database
     * @return DBConnection object reference
     */
    static DBConnection& getInstance(const std::string& connectionString) {
        std::lock_guard<std::mutex> lock(instanceMux);
        if (instance == nullptr) {
            instance.reset(new DBConnection(connectionString));
        }
        return *instance;
    }


    /**
     * @brief API to query a connected database
     * @param sql sql command to query
     * @return
     */
    pqxx::result query(const std::string& sql) {
        std::lock_guard<std::mutex> lock (queryMux);
        pqxx::work txn(conn);
        pqxx::result res = txn.exec(sql);
        txn.commit();
        return res;
    }

private:
    pqxx::connection conn;
    static inline std::unique_ptr<DBConnection> instance;
    static inline std::mutex instanceMux;
    std:: mutex queryMux;
};

