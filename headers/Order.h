//
// Created by Reza Tabrizi on 11/23/23.
//
#include <string>
#include <utility>
#ifndef LIMITORDERBOOK_ORDER_H
#define LIMITORDERBOOK_ORDER_H

class Order{
public:
    int id;
    std::string instrument;
    std::string owner;
    bool bidOrAsk;
    int quantity;
    double limit;
    long long entryTime;
    Order* nextOrder;
    Order* prevOrder;


    Order(int id, std::string instrument, std::string  owner, bool bidOrAsk, int quantity, double limit, long long entryTime): id(id), instrument(std::move(instrument)), owner(std::move(owner)), bidOrAsk(bidOrAsk), quantity(quantity), limit(limit), entryTime(entryTime), nextOrder(nullptr), prevOrder(nullptr){};
};

#endif //LIMITORDERBOOK_ORDER_H
