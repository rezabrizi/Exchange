#include "../headers/Limit.h"

void Limit::add_order(Order *order) {
    if (head_order == nullptr) {
        head_order = order;
        tail_order = order;
    } else {
        tail_order->next_order = order;
        order->prev_order = tail_order;
        tail_order = order;
    }
    size++;
    volume += order->quantity;
}

void Limit::reduce_order(Order *order, int reduceQuantity) {
    if (reduceQuantity < order->quantity) {
        order->quantity -= reduceQuantity;
        volume -= reduceQuantity;
    }
}

void Limit::remove_order(Order *order) {
    if (order == nullptr) return;
    if (order->prev_order == nullptr && order->next_order == nullptr) {
        head_order = tail_order = nullptr;
    } else if (order->prev_order == nullptr) {
        head_order = order->next_order;
        head_order->prev_order = nullptr;
    } else if (order->next_order == nullptr) {
        tail_order = order->prev_order;
        tail_order->next_order = nullptr;
    } else {
        Order *prevNode = order->prev_order;
        Order *nextNode = order->next_order;
        prevNode->next_order = nextNode;
        nextNode->prev_order = prevNode;
    }
    volume -= order->quantity;
    size--;
}

double Limit::get_limit_price() const {
    return limit_price;
}

int Limit::get_limit_size() const {
    return size;
}

int Limit::get_limit_volume() const {
    return volume;
}

Order *Limit::get_top_order() const {
    if (head_order == nullptr) return nullptr;
    return head_order;
}