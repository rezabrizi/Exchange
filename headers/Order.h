#pragma once
#include "exchange_common.h"

/**
 * @class Order
 * @brief defines an order object
 */

class Order{
public:

    Order(int orderId, std::string instrumentId, std::string type, std::string clientId, bool bidOrAsk, int quantity, double limit,
          long long entryTime, long long cancelTime): order_id(orderId), instrument_id(std::move(instrumentId)), type(std::move(type)),
                                                      client_id(std::move(clientId)), bid_ask(bidOrAsk), quantity(quantity), price(limit), entry_time(entryTime),
                                                      cancel_time(cancelTime), next_order(nullptr), prev_order(nullptr){};
    // order id unique for individual instruments
    int order_id;

    // tradable asset
    std::string instrument_id;

    // client placing the order
    std::string client_id;

    // the type of the order: market or limit
    std::string type;

    // whether the order is a buy or sell order
    bool bid_ask;

    // quantity of the order
    int quantity;

    // price of the order
    double price;

    // order placement time
    long long entry_time;

    // order cancel time
    long long cancel_time;

    // next order
    Order* next_order;

    //prev order
    Order* prev_order;
};
