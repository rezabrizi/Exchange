//
// Created by Reza Tabrizi on 12/30/23.
//
#include <string>
#include <utility>

#ifndef LIMITORDERBOOK_EXECUTION_H
#define LIMITORDERBOOK_EXECUTION_H

class Execution{
    int executionId;
    int orderId;
    std::string instrumentId;
    std::string clientId;
    double price;
    int quantity;
    long long timestamp;

public:
    Execution(int executionId, int orderId, std::string instrumentId, std::string clientId, double price, int quantity, long long timestamp):
    executionId(executionId), orderId(orderId), instrumentId(std::move(instrumentId)), clientId(std::move(clientId)), price(price),
    quantity(quantity), timestamp(timestamp){};
};


#endif //LIMITORDERBOOK_EXECUTION_H
