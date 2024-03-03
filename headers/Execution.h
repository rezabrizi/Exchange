#pragma once
#include "exchange_common.h"



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

    int GetExecutionId() const {return executionId;}
    int GetOrderId() const {return orderId;}
    std::string GetInstrumentId () const {return instrumentId;}
    std::string GetClientId () const {return clientId;}
    double GetPrice () const {return price;}
    int GetQuantity() const {return quantity;}
    long long GetTimestamp() const {return timestamp;}

};


