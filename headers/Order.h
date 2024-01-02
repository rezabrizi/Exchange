//
// Created by Reza Tabrizi on 11/23/23.
//
#include <string>
#include <utility>
#ifndef LIMITORDERBOOK_ORDER_H
#define LIMITORDERBOOK_ORDER_H

class Order{
public:
    int orderId;
    std::string instrumentId;
    std::string clientId;
    std::string type;
    bool bidOrAsk;
    int quantity;
    double price;
    long long entryTime;
    long long cancelTime;
    Order* nextOrder;
    Order* prevOrder;


    Order(int orderId, std::string instrumentId, std::string type, std::string clientId, bool bidOrAsk, int quantity, double limit,
          long long entryTime, long long cancelTime): orderId(orderId), instrumentId(std::move(instrumentId)), type(std::move(type)),
                                                      clientId(std::move(clientId)), bidOrAsk(bidOrAsk), quantity(quantity), price(limit), entryTime(entryTime),
                                                      cancelTime(cancelTime), nextOrder(nullptr), prevOrder(nullptr){};
};

#endif //LIMITORDERBOOK_ORDER_H
