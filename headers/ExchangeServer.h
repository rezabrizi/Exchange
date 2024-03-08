
#pragma once

#include <utility>

#include "exchange_common.h"
#include "Message.h"
#include "net.h"
#include "CentralMessageSystem.h"
#include "DBConnection.h"


enum class ExchangeMessages : uint32_t
{
    Authenticate,
    AddOrder,
    CancelOrder,
    Execution,
    OrderConfirmation,
    AlertMessage
};

struct Client
{
    std::string client_id;
    std::string ip;
    std::string pass_key;
};


class ExchangeServer : public net::server_interface<ExchangeMessages>
{
public:

    ExchangeServer(uint16_t nPort, CentralMessageSystem& cms) : net::server_interface<ExchangeMessages> (nPort), cms(cms)
    {
        fill_registered_clients();
        fill_instruments();

        cms.subscribe("TradeExecutionMessage", [this](const BaseMessage &message) {
            this->ProcessMessage(message);
        });

        cms.subscribe("OrderConfirmationMessage", [this](const BaseMessage &message) {
            this->ProcessMessage(message);
        });

        cms.subscribe("AlertMessage", [this](const BaseMessage &message) {
            this->ProcessMessage(message);
        });

    }

private:

    long long GetCurrentTimeStamp ()
    {
        auto now = std::chrono::system_clock::now();

        // Convert time point to duration since epoch, then to milliseconds
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();


        long long currentTimestamp = static_cast<long long>(millis);
        return currentTimestamp;
    }

    void fill_registered_clients()
    {
        std::string client_query = "Select * from Clients";
        pqxx::result R = db.query(client_query);
        for (auto row: R)
        {
            auto client_id = row["ID"].as<std::string>();
            auto name = row["name"].as<std::string>();
            auto ip = row["TCPIPAddress"].as<std::string>();
            auto pass_key = row["Passkey"].as<std::string>();
            Client new_client = {client_id, ip, pass_key};
            clients[client_id] = new_client;
            white_list.insert(ip);
        }
    }

    void fill_instruments()
    {
        std::string instrument_query = "Select * from Instruments";
        pqxx::result R = db.query(instrument_query);

        for (auto row: R)
        {
            auto instrument_id = row["ID"].as<std::string>();
            tradable_instruments.insert(instrument_id);
        }
    }

    void ProcessMessage(const BaseMessage& message)
    {
        if (message.messageType == "TradeExecutionMessage")
        {
            const auto *trade_execution_message = dynamic_cast <const TradeExecutionMessage *>(&message);

            if (trade_execution_message != nullptr)
            {
                try
                {
                    int order_id = trade_execution_message->orderId;
                    std::string client_name = trade_execution_message->clientId;
                    std::string instrument_id = trade_execution_message->instrumentId;
                    double price = trade_execution_message->price;
                    int quantity = trade_execution_message->quantity;
                    long long timestamp = trade_execution_message->timestamp;

                    if (client_connections.find(client_name) != client_connections.end())
                    {
                        net::message<ExchangeMessages> msg;
                        msg.header.id = ExchangeMessages::Execution;

                        msg << client_name << instrument_id << order_id << price << quantity << timestamp;
                        MessageClient(client_connections[client_name], msg);
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Server failed while sending an execution message to " << trade_execution_message->clientId << " " << e.what() << "\n";
                }
            }
        }
        else if (message.messageType == "OrderConfirmationMessage")
        {
            const auto* order_confirmation_message = dynamic_cast <const OrderConfirmationMessage *> (&message);
            if (order_confirmation_message != nullptr)
            {
                try
                {
                    int order_id = order_confirmation_message->orderId;
                    std::string client_name = order_confirmation_message->clientId;
                    std::string instrument_id = order_confirmation_message->instrumentId;

                    long long cancel_time = order_confirmation_message->cancelTime;

                    net::message<ExchangeMessages> msg;
                    msg.header.id = ExchangeMessages::OrderConfirmation;

                    msg << client_name << instrument_id << order_id << cancel_time;
                    MessageClient(client_connections[client_name], msg);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Server failed while sending an order confirmation message to " << order_confirmation_message->clientId << " " << e.what() << "\n";
                }
            }
        }
        else if (message.messageType == "AlertMessage")
        {
            std::cout << " there is a system emssage\n";
            const auto* system_message = dynamic_cast <const SystemMessage*> (&message);

            if (system_message != nullptr)
            {
                try
                {
                    std::string instrument_id = system_message->instrumentId;

                    for (const auto& client_name: client_interest[instrument_id])
                    {
                        if (client_connections.find(client_name) != client_connections.end())
                        {
                            net::message<ExchangeMessages> msg;
                            msg.header.id = ExchangeMessages::AlertMessage;

                            std::string alert = system_message->alert;
                            std::string description = system_message->description;

                            msg << instrument_id << alert << description;

                            MessageClient(client_connections[client_name], msg);
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }

                    }

                }
                catch (const std::exception& e)
                {
                    std::cerr << "Server failed while sending a system message to all connection_ids " << e.what() << "\n";
                }
            }
        }
    }

    bool OnClientConnect(std::shared_ptr<net::connection<ExchangeMessages>> client, std::string ip) override
    {
        if (white_list.find(ip) != white_list.end())
            return true;
        return false;
    }

    void OnClientDisconnect(std::shared_ptr<net::connection<ExchangeMessages>> client) override
    {
        if (connection_ids.find(client->GetID()) != connection_ids.end())
        {
            std::string client_name = connection_ids[client->GetID()];
            client_connections.erase(client_name);
            for (auto& pair: client_interest)
            {

                if (pair.second.find(client_name) != pair.second.end())
                {
                    pair.second.erase(client_name);
                }
            }
            connection_ids.erase(client->GetID());
        }
    }


    void OnMessage(std::shared_ptr<net::connection<ExchangeMessages>> client, net::message<ExchangeMessages>& msg) override
    {
        switch(msg.header.id)
        {
            case ExchangeMessages::Authenticate:
            {
                std::string client_name;
                msg >> client_name;
                transform(client_name.begin(), client_name.end(), client_name.begin(), ::toupper);

                if (clients.find(client_name) != clients.end())
                {
                    connection_ids[client->GetID()] = client_name;
                    client_connections[client_name] = client;
                }
                break;
            }


            case ExchangeMessages::AddOrder:
            {
                try {
                    if (connection_ids.find(client->GetID()) != connection_ids.end() && client_connections.find(connection_ids[client->GetID()]) != client_connections.end())
                    {

                        std::string client_name = connection_ids[client->GetID()];

                        std::string instrument_id;
                        bool bid_or_ask;
                        double price;
                        int quantity;
                        std::string order_type;

                        msg >> price >> instrument_id >>  bid_or_ask >> order_type >> quantity;

                        transform(instrument_id.begin(), instrument_id.end(), instrument_id.begin(), ::toupper);

                        if (tradable_instruments.find(instrument_id) != tradable_instruments.end() && quantity > 0 && (order_type == "market" || order_type == "limit"))
                        {
                            client_interest[instrument_id].insert(client_name);
                            std::unique_ptr<BaseMessage> add_order_message = std::make_unique<AddOrderMessage>(
                                    cms.assign_messaging_id(),
                                    "AddOrderMessage",
                                    GetCurrentTimeStamp(),
                                    client_name,
                                    instrument_id,
                                    bid_or_ask,
                                    price,
                                    quantity,
                                    order_type
                            );

                            cms.publish(std::move(add_order_message));
                        }
                    }

                } catch (std::exception& e)
                {
                    std::cerr << e.what () << "\n";
                }
                break;
            }

            case ExchangeMessages::CancelOrder:
            {
                if (connection_ids.find(client->GetID()) != connection_ids.end() && client_connections.find(connection_ids[client->GetID()]) != client_connections.end())
                {
                    std::string client_name = connection_ids[client->GetID()];
                    std::string instrument_id;
                    int orderId;

                    msg >> instrument_id  >> orderId;
                    transform(instrument_id.begin(), instrument_id.end(), instrument_id.begin(), ::toupper);

                    if (tradable_instruments.find(instrument_id) != tradable_instruments.end() && orderId > -1)
                    {
                        std::unique_ptr<BaseMessage> cancel_order_message = std::make_unique<CancelOrderMessage>(
                                cms.assign_messaging_id(),
                                "CancelOrderMessage",
                                GetCurrentTimeStamp(),
                                client_name,
                                instrument_id,
                                orderId
                        );
                        cms.publish(std::move(cancel_order_message));
                    }
                }
                break;
            }
        }
    }


    // client IP white list
    std::unordered_set <std::string> white_list;

    // instruments that can be traded
    std::unordered_set <std::string> tradable_instruments;

    // client information
    std::unordered_map <std::string, Client> clients;

    // connection ids and who owns it
    std::unordered_map<uint32_t, std::string> connection_ids;

    // client connection objects
    std::unordered_map<std::string, std::shared_ptr<net::connection<ExchangeMessages>>> client_connections;

    // client interest list based on instruments
    std::unordered_map <std::string, std::unordered_set<std::string>> client_interest;

    CentralMessageSystem& cms;

    DBConnection& db = DBConnection::getInstance("dbname=exchange user=rezatabrizi password=1123 host=localhost port=5432");
};