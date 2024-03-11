#pragma once
#include "exchange_common.h"

/**
 * @file message.h
 * @brief contains all the message classes corresponding to system messages
 */


struct BaseMessage {
    int message_id;
    std::string message_type;
    long long timestamp;

    BaseMessage(int msgId, const std::string& msgType, long long timestamp)
            : message_id(msgId), message_type(msgType), timestamp(timestamp) {}
    virtual ~BaseMessage() = default;
};

struct OrderMessage : public BaseMessage {
    std::string client_id;
    std::string instrument_id;


    OrderMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& clientId, const std::string& instrumentId)
            : BaseMessage(msgId, msgType, timestamp), client_id(clientId), instrument_id(instrumentId) {}
};

struct AddOrderMessage : public OrderMessage {
    bool bid_ask;
    double limit;
    int quantity;
    std::string order_type;

    AddOrderMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& clientId, const std::string& instrumentId, bool bidAsk, double limit, int quantity, const std::string& orderType)
            : OrderMessage(msgId, msgType, timestamp, clientId, instrumentId), bid_ask(bidAsk), limit(limit), quantity(quantity), order_type(orderType) {}
};

struct CancelOrderMessage : public OrderMessage {
    int order_id;

    CancelOrderMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& clientId, const std::string& instrumentId, int orderId)
            : OrderMessage(msgId, msgType, timestamp, clientId, instrumentId), order_id(orderId) {}
};

struct TradeExecutionMessage : public BaseMessage {
    int order_id;
    std::string client_id;
    std::string instrument_id;
    double price;
    int quantity;

    TradeExecutionMessage(int msgId, const std::string& msgType, long long timestamp, int orderId, const std::string& clientId, const std::string& instrumentId, double price, int quantity)
            : BaseMessage(msgId, msgType, timestamp), order_id(orderId), client_id(clientId), instrument_id(instrumentId), price(price), quantity(quantity) {}
};

struct OrderConfirmationMessage : public AddOrderMessage {
    int order_id;
    long long cancel_time;

    OrderConfirmationMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& clientId, const std::string& instrumentId, bool bidAsk, double limit, int quantity, const std::string& orderType, int orderId, long long cancelTime)
            : AddOrderMessage(msgId, msgType, timestamp, clientId, instrumentId, bidAsk, limit, quantity, orderType), order_id(orderId), cancel_time(cancelTime){}
};

struct SystemMessage : public BaseMessage {
    std::string instrument_id;
    std::string alert;
    std::string description;

    SystemMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& instrumentId, const std::string& alert, const std::string& description)
            : BaseMessage(msgId, msgType, timestamp), instrument_id(instrumentId), alert(alert), description(description) {}
};


