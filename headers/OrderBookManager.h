//
// Created by Reza Tabrizi on 12/24/23.
//

#include <unordered_map>
#include <string>
#include "Message.h"
#include "CentralMessageSystem.h"
#include "LOB.h"


#ifndef LIMITORDERBOOK_ORDERBOOKMANAGER_H
#define LIMITORDERBOOK_ORDERBOOKMANAGER_H

class OrderBookManager{
    CentralMessageSystem& CMS;
    std::unordered_map<std::string, LOB*> OrderBook;


public:
    OrderBookManager(CentralMessageSystem& CMS);

    /**
     * @brief Callback function to receive messages from the CMS
     * @param message the message
     */
    void ProcessMessage(const BaseMessage& message);
    /**
     * if the order book doesn't exist create an order book object. If it does exist, then just add the order to it. and call check for matches on that security.
     * return
     */
    OrderConfirmationMessage AddOrder(const AddOrderMessage& message);
    /**
     * call the execute function.
     */
    OrderConfirmationMessage CancelOrder(int orderId);
    std::vector<TradeExecutionMessage> CheckForMatch(std::string instrumentId);
};

#endif //LIMITORDERBOOK_ORDERBOOKMANAGER_H
