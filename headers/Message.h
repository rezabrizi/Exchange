#pragma once
#include "exchange_common.h"

struct BaseMessage {
    int messageId;
    std::string messageType;
    long long timestamp;

    BaseMessage(int msgId, const std::string& msgType, long long timestamp)
            : messageId(msgId), messageType(msgType), timestamp(timestamp) {}
    virtual ~BaseMessage() = default;
};

struct OrderMessage : public BaseMessage {
    std::string clientId;
    std::string instrumentId;


    OrderMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& clientId, const std::string& instrumentId)
            : BaseMessage(msgId, msgType, timestamp), clientId(clientId), instrumentId(instrumentId) {}
};

struct AddOrderMessage : public OrderMessage {
    bool bidOrAsk;
    double limit;
    int quantity;
    std::string orderType;

    AddOrderMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& clientId, const std::string& instrumentId, bool bidAsk, double limit, int quantity, const std::string& orderType)
            : OrderMessage(msgId, msgType, timestamp, clientId, instrumentId), bidOrAsk(bidAsk), limit(limit), quantity(quantity), orderType(orderType) {}
};

struct CancelOrderMessage : public OrderMessage {
    int orderId;

    CancelOrderMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& clientId, const std::string& instrumentId, int orderId)
            : OrderMessage(msgId, msgType, timestamp, clientId, instrumentId), orderId(orderId) {}
};

struct TradeExecutionMessage : public BaseMessage {
    int orderId;
    std::string clientId;
    std::string instrumentId;
    double price;
    int quantity;

    TradeExecutionMessage(int msgId, const std::string& msgType, long long timestamp, int orderId, const std::string& clientId, const std::string& instrumentId, double price, int quantity)
            : BaseMessage(msgId, msgType, timestamp), orderId(orderId), clientId(clientId), instrumentId(instrumentId), price(price), quantity(quantity) {}
};

struct OrderConfirmationMessage : public AddOrderMessage {
    int orderId;
    long long cancelTime;

    OrderConfirmationMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& clientId, const std::string& instrumentId, bool bidAsk, double limit, int quantity, const std::string& orderType, int orderId, long long cancelTime)
            : AddOrderMessage(msgId, msgType, timestamp, clientId, instrumentId, bidAsk, limit, quantity, orderType), orderId(orderId), cancelTime(cancelTime){}
};

struct SystemMessage : public BaseMessage {
    std::string instrumentId;
    std::string alert;
    std::string description;

    SystemMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& instrumentId, const std::string& alert, const std::string& description)
            : BaseMessage(msgId, msgType, timestamp), instrumentId(instrumentId), alert(alert), description(description) {}
};


