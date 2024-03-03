#pragma once
#include "Order.h"



class Limit{
    double limitPrice;
    int size;
    int volume;
    Order* headOrder;
    Order* tailOrder;

public:
    Limit(double limitPrice): limitPrice(limitPrice), size(0), volume(0), headOrder(nullptr), tailOrder(nullptr){};
    void AddOrder(Order *order);
    void RemoveOrder(Order *order);
    void ReduceOrder(Order *order, int reduceQuantity);
    double GetLimitPrice() const;
    int GetLimitSize() const;
    int GetLimitVolume() const;
    Order* GetTopOrder() const;
};

