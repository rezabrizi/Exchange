//
// Created by Reza Tabrizi on 11/23/23.
//
#include <iostream>
#include <string>
#include <iomanip>
#include "../headers/LOB.h"


void LOB::AddLimitOrder(bool bidOrAsk, int quantity, double limitPrice, int entryTime) {
    Order* newOrder = new Order(curr_id, bidOrAsk, quantity, limitPrice, entryTime);

    if (bidOrAsk) {
        if(buyMap.find(limitPrice) != buyMap.end()){
            buyMap[limitPrice]->AddOrder(newOrder);
        }else{
            Limit* newLimit = new Limit(limitPrice);
            newLimit->AddOrder(newOrder);
            buyTree[limitPrice] = newLimit;
            buyMap[limitPrice] = newLimit;
            if (bestBid == nullptr || bestBid->GetLimitPrice() < limitPrice){
                bestBid = newLimit;
            }
        }
    }else{
        if(sellMap.find(limitPrice) != sellMap.end()){
            sellMap[limitPrice]->AddOrder(newOrder);
        }else{
            Limit* newLimit = new Limit(limitPrice);
            newLimit->AddOrder(newOrder);
            sellTree[limitPrice] = newLimit;
            sellMap[limitPrice] = newLimit;
            if (bestAsk == nullptr || bestAsk->GetLimitPrice() > limitPrice){
                bestAsk = newLimit;
            }
        }
    }
    std::string type_of_order = (bidOrAsk) ? "bid" : "ask";
    std::cout << "-------ADDING--------" << std::endl;
    std::cout << "ID:   " << curr_id << " " << type_of_order << "   $" << limitPrice << "   " << quantity << " shares   time   " << entryTime << std::endl;
    if (bestBid != nullptr) {
        std::cout << "BEST BID: $" << bestBid->GetLimitPrice() << std::endl;
    }else {
        std::cout << "EMPTY BOOK"<< std::endl;
    }

    if (bestAsk != nullptr) {
        std::cout << "BEST ASK: $" << bestAsk->GetLimitPrice() << std::endl;
    }else {
        std::cout << "EMPTY BOOK"<< std::endl;
    }


    std::cout << "--------DONE---------" << std::endl;


    orders[curr_id] = newOrder;
    curr_id++;

}

void LOB::RemoveLimitOrder(int orderId) {
    // If the order doesn't exist exit out of the function
    if (orders.find(orderId) == orders.end()) return;

    // Find the Order pointer for the order that is being removed
    Order* orderToCancel = orders[orderId];
    Limit* parentLimit = nullptr;

    // if the order is a bid
    if (orderToCancel->bidOrAsk){

        // parentLimit is the corresponding doublyLinkedList limit
        parentLimit = buyMap[orderToCancel->limit];
        double parentLimitPrice = parentLimit->GetLimitPrice();

        // RemoveOrder removes the order and deletes the order object
        parentLimit->RemoveOrder(orderToCancel);

        /**
         * if the size of the limit of the order is now empty due to the removal of the order
         * then additional steps are required
         */
        if (parentLimit->GetLimitSize() == 0){
            // remove the limit from the limit hashmap
            buyMap.erase(parentLimitPrice);
            // remove the limit from the limit tree
            buyTree.erase(parentLimitPrice);
            /**
             * there is the possibility of the best bid/ask changing
             * if the limit that was removed was the best bid/ask then find the new best bid/ask
             */
            if (bestBid->GetLimitPrice() == parentLimitPrice)
                bestBid = !buyTree.empty() ? (*(buyTree.rbegin())).second : nullptr;
            delete parentLimit;
        }
    // if the order is an ask
    }else{
        // parentLimit is the corresponding doublyLinkedList limit
        parentLimit = sellMap[orderToCancel->limit];
        double parentLimitPrice = parentLimit->GetLimitPrice();

        // RemoveOrder removes the order and deletes the order object
        parentLimit->RemoveOrder(orderToCancel);
        /**
         * if the size of the limit of the order is now empty due to the removal of the order
         * then additional steps are required
         */
        if (parentLimit->GetLimitSize() == 0){
            sellMap.erase(parentLimitPrice);
            sellTree.erase(parentLimitPrice);
            /**
             * there is the possibility of the best bid/ask changing
             * if the limit that was removed was the best bid/ask then find the new best bid/ask
             */
            if (bestAsk->GetLimitPrice() == parentLimitPrice)
                bestAsk = !sellTree.empty() ? (*(sellTree.begin())).second : nullptr;
            delete parentLimit;
        }
    }
    orders.erase(orderId);
/**
    std::cout << "-------cancelling an order--------" << std::endl;
    std::cout << "order id to cancel: " << orderId << std::endl;
    if (bestBid != nullptr) {
        std::cout << "Best Bid now is: $" << bestBid->GetLimitPrice()<< std::endl;
    }else {
        std::cout << "The Bid Book is empty, so there are no best bids"<< std::endl;
    }
    */
}

void LOB::Execute() {
    while (bestBid != nullptr && bestAsk != nullptr && bestBid->GetLimitPrice() >= bestAsk->GetLimitPrice()){
            Order* topBidOrder = bestBid->GetTopOrder();
            Order* topAskOrder = bestAsk->GetTopOrder();

            if (topBidOrder->quantity > topAskOrder->quantity){
                bestBid->ReduceOrder(topBidOrder, topAskOrder->quantity);
                RemoveLimitOrder(topAskOrder->id);
            }
            else if (topBidOrder->quantity < topAskOrder->quantity){
                bestAsk->ReduceOrder(topAskOrder, topBidOrder->quantity);
                RemoveLimitOrder(topBidOrder->id);
            }else{
                RemoveLimitOrder(topBidOrder->id);
                RemoveLimitOrder(topAskOrder->id);
            }
    }
}

int LOB::GetBidVolumeAtLimit(double limit) {
    return (buyMap.find(limit) != buyMap.end()) ? buyMap[limit]->GetLimitVolume() : -1;
}

int LOB::GetAskVolumeAtLimit(double limit) {
    return (sellMap.find(limit) != sellMap.end()) ? sellMap[limit]->GetLimitVolume() : -1;
}

double LOB::GetBestBid() {
    return (bestBid != nullptr) ? bestBid->GetLimitPrice() : 0.00;
}

double LOB::GetBestAsk() {
    return (bestAsk != nullptr) ? bestAsk->GetLimitPrice() : 0.00;
}

void LOB::PrintBidBook() {
    std::cout << "----- Bid Book -----" << std::endl;
    std::cout << std::left << std::setw(10) << "Price" << std::setw(15) << "Quantity" << std::setw(15) << "Total Orders" << std::endl;
    std::cout << std::setfill('-') << std::setw(40) << "-" << std::endl; // Separator

    for (const auto& limitPair : buyTree) {
        double price = limitPair.first;
        Limit* limit = limitPair.second;
        int totalOrders = limit->GetLimitSize(); // Assuming GetTotalOrders() returns the number of orders at this price
        int totalQuantity = limit->GetLimitVolume(); // Assuming GetTotalQuantity() returns the total quantity of orders at this price

        std::cout << std::setfill(' ') << std::left;
        std::cout << std::setw(10) << price << std::setw(15) << totalQuantity << std::setw(15) << totalOrders << std::endl;
    }
}

void LOB::PrintAskBook() {
    std::cout << "----- Ask Book -----" << std::endl;
    std::cout << std::left << std::setw(10) << "Price" << std::setw(15) << "Quantity" << std::setw(15) << "Total Orders" << std::endl;
    std::cout << std::setfill('-') << std::setw(40) << "-" << std::endl; // Separator

    for (const auto& limitPair : sellTree) {
        double price = limitPair.first;
        Limit* limit = limitPair.second;
        int totalOrders = limit->GetLimitSize(); // Assuming GetTotalOrders() returns the number of orders at this price
        int totalQuantity = limit->GetLimitVolume(); // Assuming GetTotalQuantity() returns the total quantity of orders at this price

        std::cout << std::setfill(' ') << std::left;
        std::cout << std::setw(10) << price << std::setw(15) << totalQuantity << std::setw(15) << totalOrders << std::endl;
    }
}

