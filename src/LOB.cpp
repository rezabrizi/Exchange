//
// Created by Reza Tabrizi on 11/23/23.
//
#include <iostream>
#include <string>
#include <iomanip>
#include <utility>
#include <cstdlib>
#include "../headers/LOB.h"


ActiveLOBMapAndTree LOB::GetActiveMapAndTree(bool bidOrAsk){
    ActiveLOBMapAndTree mat{};
    mat.activeMapPtr = (bidOrAsk) ? &buyMap : &sellMap;
    mat.activeTreePtr = (bidOrAsk) ? &buyTree : &sellTree;
    return mat;
}


void LOB::AddLimitOrder(std::string instrument, std::string owner, bool bidOrAsk, int quantity, double limitPrice, int entryTime) {
    Order* newOrder = new Order(curr_id, std::move(instrument), std::move(owner), bidOrAsk, quantity, limitPrice, entryTime);
    ActiveLOBMapAndTree mat = GetActiveMapAndTree(bidOrAsk);

    if (mat.activeMapPtr->find(limitPrice) != mat.activeMapPtr->end()){
        (*mat.activeMapPtr)[limitPrice]->AddOrder(newOrder);
    }else{
        Limit* newLimit = new Limit(limitPrice);
        newLimit->AddOrder(newOrder);
        (*mat.activeTreePtr)[limitPrice] = newLimit;
        (*mat.activeMapPtr)[limitPrice] = newLimit;
        if (bidOrAsk){
            if (bestBid == nullptr || bestBid->GetLimitPrice() < limitPrice){
                bestBid = newLimit;
            }
        }else {
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
    ActiveLOBMapAndTree mat = GetActiveMapAndTree(orderToCancel->bidOrAsk);
    Limit* parentLimit = (*mat.activeMapPtr)[orderToCancel->limit];
    double parentLimitPrice = parentLimit->GetLimitPrice();

    parentLimit->RemoveOrder(orderToCancel);

    if (parentLimit->GetLimitSize() == 0){
        mat.activeMapPtr->erase(parentLimitPrice);
        mat.activeTreePtr->erase(parentLimitPrice);

        if (orderToCancel->bidOrAsk){
            if (bestBid->GetLimitPrice() == parentLimitPrice){
                bestBid = (!mat.activeTreePtr->empty()) ? (*(mat.activeTreePtr->rbegin())).second : nullptr;
            }
        } else {
            if (bestAsk->GetLimitPrice() == parentLimitPrice){
                bestAsk = (!mat.activeTreePtr->empty()) ? (*(mat.activeTreePtr->begin())).second : nullptr;
            }
        }
        delete parentLimit;
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


Limit* LOB::FindNextLowestLimit(const std::map<double, Limit*>& tree, const double& bestPrice) {
    auto it = tree.find(bestPrice);

    // If bestPrice is not found or it is the first element, return nullptr
    if (it == tree.end() || it == tree.begin()) {
        return nullptr;
    }

    // Move to the previous element (which exists since it is not at the beginning)
    --it;
    return it->second; // Return the Limit* for the next lowest price
}


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

