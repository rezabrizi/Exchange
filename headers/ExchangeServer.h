
#pragma once

#include <utility>

#include "exchange_common.h"
#include "Message.h"
#include "net.h"
#include "CentralMessageSystem.h"
#include "DBConnection.h"

enum class ExchangeMessages : uint32_t
{
    Connect,
    AddOrder,
    CancelOrder,
    Execution,
    OrderConfirmation,
    AlertMessage
};


class ExchangeServer : public net::server_interface<ExchangeMessages>
{
public:

    ExchangeServer(uint16_t nPort, CentralMessageSystem& cms, std::unordered_set<std::string>& wl) : net::server_interface<ExchangeMessages> (nPort), cms(cms), white_list(std::move(wl))
    {
        fill_registered_clients();

        cms.Subscribe("TradeExecutionMessage", [this](const BaseMessage& message) {
            this->ProcessMessage(message);
        });

        cms.Subscribe("OrderConfirmationMessage", [this](const BaseMessage& message) {
            this->ProcessMessage(message);
        });

        cms.Subscribe("AlertMessage", [this](const BaseMessage& message) {
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
            client_connections[client_id] = nullptr;
        }
    }

    void ProcessMessage(const BaseMessage& message)
    {
        if (message.messageType == "TradeExecutionMessage")
        {
            std::cout << "There is an execution \n";

            const TradeExecutionMessage *trade_execution_message = dynamic_cast <const TradeExecutionMessage *>(&message);

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

                    net::message<ExchangeMessages> msg;
                    msg.header.id = ExchangeMessages::Execution;

                    msg << client_name << instrument_id << order_id << price << quantity << timestamp;

                    MessageClient(client_connections[client_name], msg);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Server failed while sending an execution message to " << trade_execution_message->clientId << " " << e.what() << "\n";
                }
            }
        }
        else if (message.messageType == "OrderConfirmationMessage")
        {
            const OrderConfirmationMessage* order_confirmation_message = dynamic_cast <const OrderConfirmationMessage *> (&message);
            std::cout << "There is a confirmation \n";
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

                }
                catch (const std::exception& e)
                {
                    std::cerr << "Server failed while sending an order confirmation message to " << order_confirmation_message->clientId << " " << e.what() << "\n";
                }
            }

        }
        else if (message.messageType == "SystemMessage")
        {
            const SystemMessage* system_message = dynamic_cast <const SystemMessage*> (&message);

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
                        }

                    }

                }
                catch (const std::exception& e)
                {
                    std::cerr << "Server failed while sending a system message to all clients " << e.what() << "\n";
                }
            }
        }



    }

    virtual bool OnClientConnect(std::shared_ptr<net::connection<ExchangeMessages>> client, std::string ip)
    {
        if (white_list.find(ip) != white_list.end())
            return true;
        return false;
    }

    virtual void OnClientDisconnect(std::shared_ptr<net::connection<ExchangeMessages>> client)
    {
        if (clients.find(client->GetID()) != clients.end())
        {
            std::string client_name = clients[client->GetID()];
            client_connections[client_name] = nullptr;
            for (auto& pair: client_interest)
            {

                if (pair.second.find(client_name) != pair.second.end())
                {
                    pair.second.erase(client_name);
                }
            }
            clients.erase(client->GetID());
        }
    }


    virtual void OnMessage(std::shared_ptr<net::connection<ExchangeMessages>> client, net::message<ExchangeMessages>& msg)
    {
        //TODO Message Validation
        std::cout << "Message Type: " << static_cast<int>(msg.header.id) << std::endl;

        switch(msg.header.id)
        {
            case ExchangeMessages::Connect:
            {
                std::string client_name;
                msg >> client_name;

                if (client_connections.find(client_name) != client_connections.end())
                {
                    clients[client->GetID()] = client_name;
                    client_connections[client_name] = client;

                    std::cout << "Client " << client_name << " is registered!\n";/**/
                }
                break;
            }



            case ExchangeMessages::AddOrder:
            {
                if (clients.find(client->GetID()) != clients.end() && client_connections[clients[client->GetID()]] != nullptr)
                {
                    std::string client_name = clients[client->GetID()];
                    std::string instrument_id;
                    bool bid_or_ask;
                    double price;
                    int quantity;
                    std::string order_type;

                    msg >> instrument_id >> bid_or_ask >> price >> quantity >> order_type;

                    client_interest[instrument_id].insert(client_name);
                    std::cout << "Order Confirmation - Client ID: " << client_name << ", Instrument ID: " << instrument_id
                    << ", price: $" << price << ", quantity: " << quantity << ", order type: " << order_type << "\n";
                    std::unique_ptr<BaseMessage> add_order_message = std::make_unique<AddOrderMessage>(
                            cms.AssignMessageId(),
                            "AddOrderMessage",
                            GetCurrentTimeStamp(),
                            client_name,
                            instrument_id,
                            bid_or_ask,
                            price,
                            quantity,
                            order_type
                    );

                    cms.Publish(std::move(add_order_message));
                }
                break;
            }

            case ExchangeMessages::CancelOrder:
            {
                if (clients.find(client->GetID()) != clients.end() && client_connections[clients[client->GetID()]] != nullptr)
                {
                    std::string client_name = clients[client->GetID()];
                    std::string instrument_id;
                    bool bid_or_ask;
                    double price;
                    int quantity;
                    std::string order_type;
                    int orderId;

                    msg >> instrument_id >> bid_or_ask >> price >> quantity >> order_type >> orderId;

                    std::unique_ptr<BaseMessage> cancel_order_message = std::make_unique<CancelOrderMessage>(
                            cms.AssignMessageId(),
                            "CancelOrderMessage",
                            GetCurrentTimeStamp(),
                            client_name,
                            instrument_id,
                            orderId
                    );
                    cms.Publish(std::move(cancel_order_message));

                }
                break;
            }

        }
    }



    std::unordered_set <std::string> white_list;

    std::unordered_set <std::string> tradable_instruments;

    std::unordered_map<uint32_t, std::string> clients;

    std::unordered_map<std::string, std::shared_ptr<net::connection<ExchangeMessages>>> client_connections;

    std::unordered_map <std::string, std::unordered_set<std::string>> client_interest;

    CentralMessageSystem& cms;

    DBConnection& db = DBConnection::getInstance("dbname=exchange user=rezatabrizi password=1123 host=localhost port=5432");
};