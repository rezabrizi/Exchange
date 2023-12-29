//
// Created by Reza Tabrizi on 12/25/23.
//
#include <string>
#ifndef LIMITORDERBOOK_MESSAGE_H
#define LIMITORDERBOOK_MESSAGE_H


struct BaseMessage{
    int messageId;


    std::string messageType;
    long long timestamp;
};

struct OrderMessage : public BaseMessage{
    std::string action;
    std::string clientId;
    std::string instrumentId;
};

struct AddOrderMessage : public OrderMessage{
    double limit;
    int quantity;
    std::string type;
};

struct CancelOrderMessage : public OrderMessage{
    int orderid;
};



#endif //LIMITORDERBOOK_MESSAGE_H