//
// Created by Reza Tabrizi on 12/24/23.
//

#include <unordered_map>
#include <string>
#include <future>
#include <mutex>
#include "Message.h"
#include "CentralMessageSystem.h"
#include "LOB.h"

#ifndef LIMITORDERBOOK_ORDERBOOKMANAGER_H
#define LIMITORDERBOOK_ORDERBOOKMANAGER_H

class OrderBookManager{
    CentralMessageSystem& CMS;
    std::mutex orderBookMutex;
    std::unordered_map<std::string, LOB*> orderBook;
    DBConnection& db = DBConnection::getInstance("dbname=exchange user=rezatabrizi password=1123 host=localhost port=5432");

    void StartUpOrderBook();
    void ProcessMessage(const BaseMessage& message);
    std::unique_ptr<OrderConfirmationMessage> AddOrder(const AddOrderMessage* message);
    std::unique_ptr<OrderConfirmationMessage> CancelOrder(const std::string& instrumentId, int orderId);
    std::vector<std::unique_ptr<TradeExecutionMessage>> CheckForMatch(const std::string& instrumentId, const std::string& type);
    void WriteOrderToDB(const Order* order);
    void WriteExecutionToDB(const Execution* execution);
    void CancelOrderInDB (const std::string &instrumentId, int orderId, long long cancelTime);

public:
    OrderBookManager(CentralMessageSystem& CMS);
};

#endif //LIMITORDERBOOK_ORDERBOOKMANAGER_H
