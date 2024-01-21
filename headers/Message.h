//
// Created by Reza Tabrizi on 12/25/23.
//

#include <string>
#ifndef LIMITORDERBOOK_MESSAGE_H
#define LIMITORDERBOOK_MESSAGE_H
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

    OrderMessage(int msgId, const std::string& type, long long timestamp, const std::string& clientId, const std::string& instrumentId)
            : BaseMessage(msgId, type, timestamp), clientId(clientId), instrumentId(instrumentId) {}
};

struct AddOrderMessage : public OrderMessage {
    bool bidOrAsk;
    double limit;
    int quantity;
    std::string orderType;

    AddOrderMessage(int msgId, const std::string& msgType, long long timestamp, const std::string& clientId, const std::string& instrumentId, bool bidAsk, double limit, int quantity, const std::string& orderType)
            : OrderMessage(msgId, msgType, timestamp, clientId, instrumentId), bidOrAsk(bidAsk), limit(limit), quantity(quantity), orderType(orderType) {}
};

//@TODO Fix the parameter names from here on out

struct CancelOrderMessage : public OrderMessage {
    int orderId;

    CancelOrderMessage(int id, const std::string& type, long long ts, const std::string& client, const std::string& instrument, int order)
            : OrderMessage(id, type, ts, client, instrument), orderId(order) {}
};

struct TradeExecutionMessage : public BaseMessage {
    int orderId;
    std::string clientId;
    std::string instrumentId;
    double price;
    int quantity;

    TradeExecutionMessage(int id, const std::string& type, long long ts, int order, const std::string& client, const std::string& instrument, double pr, int qty)
            : BaseMessage(id, type, ts), orderId(order), clientId(client), instrumentId(instrument), price(pr), quantity(qty) {}
};

struct OrderConfirmationMessage : public AddOrderMessage {
    int orderId;

    OrderConfirmationMessage(int id, const std::string& msgType, long long ts, const std::string& client, const std::string& instrument, bool bidAsk, double lim, int qty, const std::string& tp, int order)
            : AddOrderMessage(id, msgType, ts, client, instrument, bidAsk, lim, qty, tp), orderId(order) {}
};

struct SystemMessage : public BaseMessage {
    std::string instrumentId;
    std::string alert;
    std::string description;

    SystemMessage(int id, const std::string& type, long long ts, const std::string& instrument, const std::string& alrt, const std::string& desc)
            : BaseMessage(id, type, ts), instrumentId(instrument), alert(alrt), description(desc) {}
};

#endif //LIMITORDERBOOK_MESSAGE_H
