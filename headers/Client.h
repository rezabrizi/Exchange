#pragma once
#include "DBConnection.h"
#include "exchange_common.h"


class ClientManager
{

public:
    ClientManager()
    {
        StartUpClient();
    }

    void StartUpClient(){
        std::string clientFetchQuery = "(Select * FROM clients)";
        pqxx::result R = db.query(clientFetchQuery);

        for (auto row: R)
        {
            clients.insert( row["clientId"].as<std::string>());
        }
    }

    bool FindClient(const std::string& client) {
        return (clients.find(client) != clients.end());
    }

    void RegisterClient(const std::string& client) {
        if (FindClient(client)) return;
        try {
            std::string registerClientQuery = "INSERT INTO Clients (ClientId) VALUES ("
                                        + client + ")";
            db.query(registerClientQuery);
        } catch (const std::exception& e)
        {
            std::cerr << "Database query failed to add new client: " << e.what() << std::endl;
        }

        clients.insert(client);
    }


private:
    std::unordered_set<std::string> clients;
    DBConnection& db = DBConnection::getInstance("dbname=exchange user=rezatabrizi password=1123 host=localhost port=5432");
};

