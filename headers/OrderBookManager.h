//
// Created by Reza Tabrizi on 12/24/23.
//

#include <unordered_map>
#include <string>
#include "LOB.h"


#ifndef LIMITORDERBOOK_ORDERBOOKMANAGER_H
#define LIMITORDERBOOK_ORDERBOOKMANAGER_H

class OrderBookManger{

    std::unordered_map<std::string, LOB> OrderBook;


public:
    OrderBookManger();
    void StartEvenLoop();
    void ProcessMessage();
    void AddNewOrder();
    void CheckForMatch();

};

#endif //LIMITORDERBOOK_ORDERBOOKMANAGER_H
