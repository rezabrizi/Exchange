//
// Created by Reza Tabrizi on 11/23/23.
//
#include <iostream>
#include <string>
#include <iomanip>
#include <utility>
#include "../headers/LOB.h"

// you cannot make a match if the orders are by the same company
// best bid and best ask are both by janestreet, this match cannot be made

void LOB::AddLimitOrder(std::string instrument, std::string owner, bool bidOrAsk, int quantity, double limitPrice, int entryTime) {
    Order* newOrder = new Order(curr_id, std::move(instrument), std::move(owner), bidOrAsk, quantity, limitPrice, entryTime);

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
}

void LOB::Execute() {
    while (bestBid != nullptr && bestAsk != nullptr && bestBid->GetLimitPrice() >= bestAsk->GetLimitPrice()){
            Order* topBidOrder = bestBid->GetTopOrder();
            Order* topAskOrder = bestAsk->GetTopOrder();

            WalkBook(&topBidOrder, &topAskOrder);
            if (topBidOrder->owner == topAskOrder->owner){
                break;
            }
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

void LOB::WalkLimits(Order **topBidOrder, Order **topAskOrder) {
    // top ask is $88  if there are 3 asks
    // js:1 js:2 js:3
    // your top bid is $90 if there are 7 orders at 90
    // js:4  js:6 js:7 js:7
    Order* tempTopBidOrder = (*topBidOrder);
    Order* tempTopAskOrder = (*topAskOrder);

    // Ensure initial pointers are not null
    if (tempTopBidOrder == nullptr || tempTopAskOrder == nullptr) {
        return;
    }

    while ((tempTopBidOrder->owner == tempTopAskOrder->owner) &&
           (tempTopBidOrder->nextOrder != nullptr || tempTopAskOrder->nextOrder != nullptr )){
        if (tempTopBidOrder->nextOrder != nullptr && tempTopAskOrder->nextOrder != nullptr){
            // we are making an assumption that to orders will not have the exact same time
            if (tempTopBidOrder->nextOrder->entryTime <= tempTopAskOrder->nextOrder->entryTime){
                tempTopBidOrder = tempTopBidOrder->nextOrder;
            } else {
                tempTopAskOrder = tempTopAskOrder->nextOrder;
            }
        }
        else {
            if (tempTopBidOrder->nextOrder != nullptr){
                tempTopBidOrder = tempTopBidOrder->nextOrder;
            }else {
                tempTopAskOrder = tempTopAskOrder->nextOrder;
            }
        }
    }

    if (tempTopBidOrder->owner != tempTopAskOrder->owner) {
        (*topBidOrder) = tempTopBidOrder;
        (*topAskOrder) = tempTopAskOrder;
    }
}

void LOB::WalkBook(Order **topBidOrder, Order **topAskOrder) {
    if ((*topBidOrder) == nullptr || (*topAskOrder) == nullptr) {
        return;
    }

    Order* tempTopBidOrder = (*topBidOrder);
    Order* tempTopAskOrder = (*topAskOrder);

    while (tempTopBidOrder->owner == tempTopAskOrder->owner){
        WalkLimits (&tempTopBidOrder, &tempTopAskOrder);
        if (tempTopBidOrder->owner == tempTopAskOrder->owner){
            Limit* nextBestBid = FindNextHighestLimit (buyTree, tempTopBidOrder->limit);
            Limit* nextBestAsk = FindNextLowestLimit(sellTree, tempTopAskOrder->limit);
            if (nextBestBid == nullptr && nextBestAsk == nullptr) {
                break;
            }

            if (tempTopBidOrder->entryTime <= tempTopAskOrder->entryTime && nextBestAsk != nullptr && tempTopBidOrder->limit >= nextBestAsk->GetLimitPrice()){
                tempTopAskOrder = nextBestAsk->GetTopOrder();
            } else if (tempTopBidOrder->entryTime <= tempTopAskOrder->entryTime && nextBestBid != nullptr && nextBestBid->GetLimitPrice() >= tempTopAskOrder->limit){
                tempTopBidOrder = nextBestBid->GetTopOrder();
            } else if (tempTopBidOrder->entryTime >= tempTopAskOrder->entryTime && nextBestBid != nullptr && nextBestBid->GetLimitPrice() >= tempTopAskOrder->limit) {
                tempTopBidOrder = nextBestBid->GetTopOrder();
            }else if (tempTopBidOrder->entryTime >= tempTopAskOrder->entryTime && nextBestAsk != nullptr && tempTopBidOrder->limit >= nextBestAsk->GetLimitPrice()){
                tempTopAskOrder = nextBestAsk->GetTopOrder();
            }
        }
    }
    if (tempTopBidOrder->owner != tempTopAskOrder->owner){
        (*topBidOrder) = tempTopBidOrder;
        (*topAskOrder) = tempTopAskOrder;
    }
}

Limit* LOB::FindNextLowestLimit(const std::map<double, Limit*>& tree, const double& bestPrice){
    auto it = tree.find(bestPrice);

    // Check if the iterator is not the first element and is valid
    if (it != tree.begin() && it != tree.end()) {
        --it; // Move to the previous element
        return it->second; // Return the Limit* for the next lowest price
    }

    // If bestBid is at the beginning or not found, return nullptr
    return nullptr;
};

Limit* LOB::FindNextHighestLimit(const std::map<double, Limit*>& tree, const double& bestPrice) {
    auto it = tree.find(bestPrice);

    // Check if the iterator is valid and not the last element
    if (it != tree.end()) {
        ++it; // Move to the next element

        // Check if we have not reached the end of the map
        if (it != tree.end()) {
            return it->second; // Return the Limit* for the next highest price
        }
    }

    // If bestBid is at the end or not found, return nullptr
    return nullptr;
}

int LOB::GetBidVolumeAtLimit(double limit) const {
    return (buyMap.find(limit) != buyMap.end()) ? buyMap.at(limit)->GetLimitVolume() : -1;
}

int LOB::GetAskVolumeAtLimit(double limit) const {
    return (sellMap.find(limit) != sellMap.end()) ? sellMap.at(limit)->GetLimitVolume() : -1;
}

double LOB::GetBestBid() const {
    return (bestBid != nullptr) ? bestBid->GetLimitPrice() : 0.00;
}

double LOB::GetBestAsk() const {
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

