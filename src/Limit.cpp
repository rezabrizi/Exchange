#include "../headers/Limit.h"

void Limit::AddOrder(Order *order) {
    if (headOrder == nullptr){
        headOrder = order;
        tailOrder = order;
    }else{
        tailOrder->next_order = order;
        order->prev_order = tailOrder;
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
    if (order->prev_order == nullptr && order->next_order == nullptr){
        headOrder = tailOrder = nullptr;
    }

    else if (order->prev_order == nullptr){
        headOrder = order->next_order;
        headOrder->prev_order = nullptr;
    }
    else if (order->next_order == nullptr){
        tailOrder = order->prev_order;
        tailOrder->next_order = nullptr;
    }

    else {
        Order* prevNode = order->prev_order;
        Order* nextNode = order->next_order;
        prevNode->next_order = nextNode;
        nextNode->prev_order = prevNode;
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