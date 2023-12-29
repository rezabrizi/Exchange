//
// Created by Reza Tabrizi on 11/23/23.
//
#include <string>
#include <utility>
#ifndef LIMITORDERBOOK_ORDER_H
#define LIMITORDERBOOK_ORDER_H

class Order{
public:
    // need to add type (market, limit, etc..), status, cancelTime
    int id;
    std::string instrumentId;
    std::string clientId;
    bool bidOrAsk;
    int quantity;
    double price;
    long long entryTime;
    Order* nextOrder;
    Order* prevOrder;


    Order(int id, std::string instrument, std::string  owner, bool bidOrAsk, int quantity, double limit, long long entryTime): id(id), instrumentId(std::move(instrument)), clientId(std::move(owner)), bidOrAsk(bidOrAsk), quantity(quantity), price(limit), entryTime(entryTime), nextOrder(nullptr), prevOrder(nullptr){};
};

#endif //LIMITORDERBOOK_ORDER_H
