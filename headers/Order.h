#pragma once
#include "exchange_common.h"


class Order{
public:

    Order(int orderId, std::string instrumentId, std::string type, std::string clientId, bool bidOrAsk, int quantity, double limit,
          long long entryTime, long long cancelTime): orderId(orderId), instrumentId(std::move(instrumentId)), type(std::move(type)),
                                                      clientId(std::move(clientId)), bidOrAsk(bidOrAsk), quantity(quantity), price(limit), entryTime(entryTime),
                                                      cancelTime(cancelTime), nextOrder(nullptr), prevOrder(nullptr){};
    // order id unique for individual instruments
    int orderId;

    // tradable asset
    std::string instrumentId;

    // client placing the order
    std::string clientId;

    // the type of the order: market or limit
    std::string type;

    // whether the order is a buy or sell order
    bool bidOrAsk;

    // quantity of the order
    int quantity;

    // price of the order
    double price;

    // order placement time
    long long entryTime;

    // order cancel time
    long long cancelTime;

    // next order
    Order* nextOrder;

    //prev order
    Order* prevOrder;
};
