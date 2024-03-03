#include "../headers/Limit.h"

void Limit::AddOrder(Order *order) {
    if (headOrder == nullptr){
        headOrder = order;
        tailOrder = order;
    }else{
        tailOrder->nextOrder = order;
        order->prevOrder = tailOrder;
        tailOrder = order;
    }
    size++;
    volume += order->quantity;
}

void Limit::ReduceOrder(Order *order, int reduceQuantity) {
    if (reduceQuantity < order->quantity){
        order->quantity -= reduceQuantity;
        volume -= reduceQuantity;
    }
}

void Limit::RemoveOrder(Order *order) {
    if (order == nullptr) return;
    if (order->prevOrder == nullptr && order->nextOrder == nullptr){
        headOrder = tailOrder = nullptr;
    }

    else if (order->prevOrder == nullptr){
        headOrder = order->nextOrder;
        headOrder->prevOrder = nullptr;
    }
    else if (order->nextOrder == nullptr){
        tailOrder = order->prevOrder;
        tailOrder->nextOrder = nullptr;
    }

    else {
        Order* prevNode = order->prevOrder;
        Order* nextNode = order->nextOrder;
        prevNode->nextOrder = nextNode;
        nextNode->prevOrder = prevNode;
    }
    volume -= order->quantity;
    size--;
}

double Limit::GetLimitPrice()const{
    return limitPrice;
}

int Limit::GetLimitSize()const{
    return size;
}

int Limit::GetLimitVolume()const{
    return volume;
}

Order* Limit::GetTopOrder() const {
    if (headOrder == nullptr) return nullptr;
    return headOrder;
}