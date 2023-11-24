//
// Created by Reza Tabrizi on 11/23/23.
//

#ifndef LIMITORDERBOOK_ORDER_H
#define LIMITORDERBOOK_ORDER_H

class Order{
public:
    int id;
    bool bidOrAsk;
    int quantity;
    double limit;
    int entryTime;
    Order* nextOrder;
    Order* prevOrder;


    Order(int id, bool bidOrAsk, int quantity, double limit, int entryTime): id(id), bidOrAsk(bidOrAsk), quantity(quantity), limit(limit), entryTime(entryTime), nextOrder(nullptr), prevOrder(nullptr){};
};

#endif //LIMITORDERBOOK_ORDER_H
