//
// Created by Reza Tabrizi on 11/23/23.
//
#include "../headers/LOB.h"


ActiveLOBMapAndTree LOB::GetActiveMapAndTree(bool bidOrAsk){
    ActiveLOBMapAndTree mat{};
    mat.activeMapPtr = (bidOrAsk) ? &buyMap : &sellMap;
    mat.activeTreePtr = (bidOrAsk) ? &buyTree : &sellTree;
    return mat;
}


long long LOB::GetTimeStamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    return millis;
}


void LOB::AddMarketOrder(Order* newOrder) {
    if (newOrder->bidOrAsk){
        marketOrderBuyQueue.push(newOrder);
    }else{
        marketOrderSellQueue.push(newOrder);
    }
    orders[currentOrderId] = newOrder;
    currentOrderId++;
}


void LOB::AddLimitOrder(Order* newOrder) {
    ActiveLOBMapAndTree mat = GetActiveMapAndTree(newOrder->bidOrAsk);

    if (mat.activeMapPtr->find(newOrder->price) != mat.activeMapPtr->end()){
        (*mat.activeMapPtr)[newOrder->price]->AddOrder(newOrder);
    }else{
        Limit* newLimit = new Limit(newOrder->price);
        newLimit->AddOrder(newOrder);
        (*mat.activeTreePtr)[newOrder->price] = newLimit;
        (*mat.activeMapPtr)[newOrder->price] = newLimit;
        if (newOrder->bidOrAsk){
            if (bestBid == nullptr || bestBid->GetLimitPrice() < newOrder->price){
                bestBid = newLimit;
            }
        }else {
            if (bestAsk == nullptr || bestAsk->GetLimitPrice() > newOrder->price){
                bestAsk = newLimit;
            }
        }
    }
    orders[currentOrderId] = newOrder;
    currentOrderId++;
}


Order* LOB::AddOrder(std::string instrumentId, std::string type, std::string clientId, bool bidOrAsk, int quantity, double limitPrice, long long entryTime) {
    Order* newOrder = new Order(currentOrderId, std::move(instrumentId), std::move(type), std::move(clientId), bidOrAsk, quantity, limitPrice, entryTime, -1);

    if (newOrder->type == "market"){
        AddMarketOrder(newOrder);
    } else if (newOrder->type == "limit"){
        AddLimitOrder(newOrder);
    }

    // For printing purposes
    std::string type_of_order = (bidOrAsk) ? "bid" : "ask";
    std::cout << "-------ADDING--------" << std::endl;
    std::cout << "ID:   " << currentOrderId << " " << type_of_order << "   $" << limitPrice << "   " << quantity << " shares   time   " << entryTime << std::endl;
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

    return newOrder;
}


void LOB::RemoveLimitOrder(int orderId) {
    // If the order doesn't exist exit out of the function
    if (orders.find(orderId) == orders.end()) return;

    // Find the Order pointer for the order that is being removed
    Order* orderToRemove = orders[orderId];
    ActiveLOBMapAndTree mat = GetActiveMapAndTree(orderToRemove->bidOrAsk);
    Limit* parentLimit = (*mat.activeMapPtr)[orderToRemove->price];
    double parentLimitPrice = parentLimit->GetLimitPrice();

    parentLimit->RemoveOrder(orderToRemove);

    if (parentLimit->GetLimitSize() == 0){
        mat.activeMapPtr->erase(parentLimitPrice);
        mat.activeTreePtr->erase(parentLimitPrice);

        if (orderToRemove->bidOrAsk){
            if (bestBid->GetLimitPrice() == parentLimitPrice){
                bestBid = FindNextHighestLimit(*mat.activeTreePtr, parentLimitPrice);
            }
        } else {
            if (bestAsk->GetLimitPrice() == parentLimitPrice){
                bestAsk = FindNextLowestLimit(*mat.activeTreePtr, parentLimitPrice);
            }
        }
        delete parentLimit;
    }
    orders.erase(orderId);
}


std::vector<Execution*> LOB::Execute(bool isLimit = true){
    std::vector<Execution*> executions;
    MatchMarketOrders(executions);
    if (isLimit){
        MatchLimitOrders(executions);
    }
    return executions;
}


Order* LOB::CancelOrder(int orderId) {
    if (orders.find(orderId) == orders.end()) {
        throw std::runtime_error("Order not found");
    }

    Order* orderToCancel = orders[orderId];
    long long cancelTime = GetTimeStamp();
    orderToCancel->cancelTime = cancelTime;

    //@TODO Update the database too
    if (orderToCancel->type == "market") {

    }
    if (orderToCancel->type == "limit"){
        RemoveLimitOrder(orderId);
    }
}


void LOB::MatchMarketOrders(std::vector<Execution*> &executions){
    bool matchNotFound = false;

    while(!marketOrderBuyQueue.empty() && !matchNotFound){
        while (bestAsk != nullptr){
            Order* currentMarketOrder = marketOrderBuyQueue.front();

            // handle a cancelled market order
            if (currentMarketOrder->cancelTime != -1){
                marketOrderBuyQueue.pop();
                orders.erase(currentMarketOrder->orderId);
                delete currentMarketOrder;
                continue;
            }
            Order* topLimitOrder = bestAsk->GetTopOrder();
            WalkOneBook(false, &topLimitOrder, currentMarketOrder);
            Limit* topLimit = buyTree[topLimitOrder->price];

            if (currentMarketOrder->clientId == topLimitOrder->clientId){
                matchNotFound = true;
                break;
            }
            // execution is possible now
            int executionQuantity = (currentMarketOrder->quantity > topLimitOrder->quantity) ?  topLimitOrder->quantity : currentMarketOrder->quantity;
            long long executionTimeStamp = GetTimeStamp();
            Execution* marketExecution = new Execution(currentExecutionId, currentMarketOrder->orderId, currentMarketOrder->instrumentId, currentMarketOrder->clientId, topLimitOrder->price, executionQuantity, executionTimeStamp);
            Execution* limitExecution = new Execution(currentExecutionId, topLimitOrder->orderId, topLimitOrder->instrumentId, topLimitOrder->clientId, topLimitOrder->price, executionQuantity, executionTimeStamp);
            executions.push_back(marketExecution);
            executions.push_back(limitExecution);
            currentExecutionId++;
            if (currentMarketOrder->quantity > topLimitOrder->quantity){
                currentMarketOrder->quantity -= topLimitOrder->quantity;
                RemoveLimitOrder(topLimitOrder->orderId);
            }else if (currentMarketOrder->quantity < topLimitOrder->quantity){
                topLimit->ReduceOrder(topLimitOrder, currentMarketOrder->quantity);
                orders.erase(currentMarketOrder->orderId);
                marketOrderBuyQueue.pop();
                delete currentMarketOrder;
            }else{
                RemoveLimitOrder(topLimitOrder->orderId);
                orders.erase(currentMarketOrder->orderId);
                marketOrderBuyQueue.pop();
                delete currentMarketOrder;
            }
        }
    }

    matchNotFound = false;
    while(!marketOrderSellQueue.empty() && !matchNotFound){
        while (bestBid != nullptr){
            Order* currentMarketOrder = marketOrderSellQueue.front();
            // handle a cancelled market order
            if (currentMarketOrder->cancelTime != -1){
                marketOrderSellQueue.pop();
                orders.erase(currentMarketOrder->orderId);
                delete currentMarketOrder;
                continue;
            }
            Order* topLimitOrder = bestBid->GetTopOrder();
            WalkOneBook(true, &topLimitOrder, currentMarketOrder);
            Limit* topLimit = sellTree[topLimitOrder->price];

            if (currentMarketOrder->clientId == topLimitOrder->clientId){
                matchNotFound = true;
                break;
            }
            int executionQuantity = (currentMarketOrder->quantity > topLimitOrder->quantity) ?  topLimitOrder->quantity : currentMarketOrder->quantity;
            long long executionTimeStamp = GetTimeStamp();
            Execution* marketExecution = new Execution(currentExecutionId, currentMarketOrder->orderId, currentMarketOrder->instrumentId, currentMarketOrder->clientId, topLimitOrder->price, executionQuantity, executionTimeStamp);
            Execution* limitExecution = new Execution(currentExecutionId, topLimitOrder->orderId, topLimitOrder->instrumentId, topLimitOrder->clientId, topLimitOrder->price, executionQuantity, executionTimeStamp);
            executions.push_back(marketExecution);
            executions.push_back(limitExecution);
            currentExecutionId++;
            if (currentMarketOrder->quantity > topLimitOrder->quantity){
                currentMarketOrder->quantity -= topLimitOrder->quantity;
                RemoveLimitOrder(topLimitOrder->orderId);
            }else if (currentMarketOrder->quantity < topLimitOrder->quantity){
                topLimit->ReduceOrder(topLimitOrder, currentMarketOrder->quantity);
                orders.erase(currentMarketOrder->orderId);
                marketOrderSellQueue.pop();
                delete currentMarketOrder;
            }else{
                RemoveLimitOrder(topLimitOrder->orderId);
                orders.erase(currentMarketOrder->orderId);
                marketOrderSellQueue.pop();
                delete currentMarketOrder;
            }
        }
    }
}


void LOB::MatchLimitOrders(std::vector<Execution*> &executions) {
    while (bestBid != nullptr && bestAsk != nullptr && bestBid->GetLimitPrice() >= bestAsk->GetLimitPrice()){
            Order* topBidOrder = bestBid->GetTopOrder();
            Order* topAskOrder = bestAsk->GetTopOrder();

            Limit* topBidLimit = buyTree[topBidOrder->price];
            Limit* topAskLimit = sellTree[topAskOrder->price];

            WalkBook(&topBidOrder, &topAskOrder);
            if (topBidOrder->clientId == topAskOrder->clientId){
                break;
            }

            int executionQuantity = (topBidOrder->quantity > topAskOrder->quantity) ? topAskOrder->quantity : topBidOrder->quantity;
            double executionPrice = (topBidOrder->entryTime < topAskOrder->entryTime) ? (topBidOrder->price) : (topAskOrder->price);
            long long executionTimeStamp = GetTimeStamp();
            Execution* bidLimitExecution = new Execution(currentExecutionId, topBidOrder->orderId, topBidOrder->instrumentId, topBidOrder->clientId, executionPrice, executionQuantity, executionTimeStamp);
            Execution* askLimitExecution = new Execution(currentExecutionId, topAskOrder->orderId, topAskOrder->instrumentId, topAskOrder->clientId, executionPrice, executionQuantity, executionTimeStamp);
            currentExecutionId++;
            executions.push_back(bidLimitExecution);
            executions.push_back(askLimitExecution);
            // Here there is a match and an execution needs to be created
            if (topBidOrder->quantity > topAskOrder->quantity){
                topBidLimit->ReduceOrder(topBidOrder, topAskOrder->quantity);
                RemoveLimitOrder(topAskOrder->orderId);
            }
            else if (topBidOrder->quantity < topAskOrder->quantity){
                topAskLimit->ReduceOrder(topAskOrder, topBidOrder->quantity);
                RemoveLimitOrder(topBidOrder->orderId);
            }else{
                RemoveLimitOrder(topBidOrder->orderId);
                RemoveLimitOrder(topAskOrder->orderId);
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

    while ((tempTopBidOrder->clientId == tempTopAskOrder->clientId) &&
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
    if (tempTopBidOrder->clientId != tempTopAskOrder->clientId) {
        (*topBidOrder) = tempTopBidOrder;
        (*topAskOrder) = tempTopAskOrder;
    }
}


void LOB::WalkOneLimit(Order **topOrder, Order *marketOrder) {
    Order *tempTopLimitOrder =(*topOrder);

    if (tempTopLimitOrder == nullptr){
        return;
    }

    while ((tempTopLimitOrder->clientId == marketOrder->clientId) &&
            (tempTopLimitOrder->nextOrder != nullptr)){
        tempTopLimitOrder = tempTopLimitOrder->nextOrder;
    }

    // we could still end up with a topOrder with the same clientId as the marketOrder
    if (tempTopLimitOrder->clientId != marketOrder->clientId){
        (*topOrder) = tempTopLimitOrder;
    }
}


void LOB::WalkBook(Order **topBidOrder, Order **topAskOrder) {
    if ((*topBidOrder) == nullptr || (*topAskOrder) == nullptr) {
        return;
    }

    Order* tempTopBidOrder = (*topBidOrder);
    Order* tempTopAskOrder = (*topAskOrder);

    while (tempTopBidOrder->clientId == tempTopAskOrder->clientId){
        WalkLimits (&tempTopBidOrder, &tempTopAskOrder);
        if (tempTopBidOrder->clientId == tempTopAskOrder->clientId){
            Limit* nextBestBid = FindNextHighestLimit (buyTree, tempTopBidOrder->price);
            Limit* nextBestAsk = FindNextLowestLimit(sellTree, tempTopAskOrder->price);
            if (nextBestBid == nullptr && nextBestAsk == nullptr) {
                break;
            }

            if (tempTopBidOrder->entryTime <= tempTopAskOrder->entryTime && nextBestAsk != nullptr && tempTopBidOrder->price >= nextBestAsk->GetLimitPrice()){
                tempTopAskOrder = nextBestAsk->GetTopOrder();
            } else if (tempTopBidOrder->entryTime <= tempTopAskOrder->entryTime && nextBestBid != nullptr && nextBestBid->GetLimitPrice() >= tempTopAskOrder->price){
                tempTopBidOrder = nextBestBid->GetTopOrder();
            } else if (tempTopBidOrder->entryTime >= tempTopAskOrder->entryTime && nextBestBid != nullptr && nextBestBid->GetLimitPrice() >= tempTopAskOrder->price){
                tempTopBidOrder = nextBestBid->GetTopOrder();
            }else if (tempTopBidOrder->entryTime >= tempTopAskOrder->entryTime && nextBestAsk != nullptr && tempTopBidOrder->price >= nextBestAsk->GetLimitPrice()){
                tempTopAskOrder = nextBestAsk->GetTopOrder();
            }
        }
    }
    if (tempTopBidOrder->clientId != tempTopAskOrder->clientId){
        (*topBidOrder) = tempTopBidOrder;
        (*topAskOrder) = tempTopAskOrder;
    }
}


void LOB::WalkOneBook(bool bidOrAsk, Order **topLimitOrder, Order* marketOrder) {

    // Theoretically should never happen
    if ((*topLimitOrder) == nullptr) {
        return;
    }

    Order* tempTopLimitOrder = (*topLimitOrder);
    ActiveLOBMapAndTree mat = GetActiveMapAndTree(bidOrAsk);

    while (tempTopLimitOrder->clientId == marketOrder->clientId){
        WalkOneLimit(&tempTopLimitOrder, marketOrder);
        if (tempTopLimitOrder->clientId == marketOrder->clientId){
            Limit* nextBestLimit = (bidOrAsk) ? FindNextHighestLimit ((*mat.activeTreePtr), tempTopLimitOrder->price)
                    : FindNextLowestLimit ((*mat.activeTreePtr), tempTopLimitOrder->price);

            if (nextBestLimit == nullptr) {
                break;
            }

            tempTopLimitOrder = nextBestLimit->GetTopOrder ();
        }
    }
    if (tempTopLimitOrder->clientId != marketOrder->clientId){
        (*topLimitOrder) = tempTopLimitOrder;
    }
}


Limit* LOB::FindNextLowestLimit(const std::map<double, Limit*>& tree, const double& bestPrice) {
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


Limit* LOB::FindNextHighestLimit(const std::map<double, Limit*>& tree, const double& bestPrice) {
    auto it = tree.find(bestPrice);

    // If bestPrice is not found Or it is the first element, return nullptr
    if (it == tree.end() || it == tree.begin()) {
        return nullptr;
    }

    // Move to the previous element (which exists since it is not at the beginning)
    --it;
    return it->second; // Return the Limit* for the next lowest price
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


