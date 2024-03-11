#include "../headers/LOB.h"

ActiveLOBMapAndTree LOB::get_active_hash_map_tree(bool bid_ask){
    ActiveLOBMapAndTree mat{};
    mat.active_map_ptr = (bid_ask) ? &bid_map : &ask_map;
    mat.active_tree_ptr = (bid_ask) ? &bid_tree : &ask_tree;
    return mat;
}


long long LOB::get_time_stamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    return millis;
}


void LOB::add_market_order(Order* order) {
    if (order->bid_ask){
        mkt_order_bid_q.push(order);
    }else{
        mkt_order_ask_q.push(order);
    }
    orders[current_order_id] = order;
    current_order_id++;
}


void LOB::add_limit_order(Order* order) {
    ActiveLOBMapAndTree mat = get_active_hash_map_tree(order->bid_ask);

    if (mat.active_map_ptr->find(order->price) != mat.active_map_ptr->end()){
        (*mat.active_map_ptr)[order->price]->AddOrder(order);
    }else{
        Limit* newLimit = new Limit(order->price);
        newLimit->AddOrder(order);
        (*mat.active_tree_ptr)[order->price] = newLimit;
        (*mat.active_map_ptr)[order->price] = newLimit;
        if (order->bid_ask){
            if (best_bid == nullptr || best_bid->GetLimitPrice() < order->price){
                best_bid = newLimit;
            }
        }else{
            if (best_ask == nullptr || best_ask->GetLimitPrice() > order->price){
                best_ask = newLimit;
            }
        }
    }
    orders[current_order_id] = order;
    current_order_id++;
}


Order* LOB::add_order(std::string instrument_id, std::string type, std::string client_id, bool bid_ask, int quantity, double limit_price, long long entry_time, int order_id) {

    current_order_id = (order_id != -1) ? order_id : current_order_id;

    Order* newOrder = new Order(current_order_id, std::move(instrument_id), std::move(type), std::move(client_id), bid_ask, quantity, limit_price, entry_time, -1);

    if (newOrder->type == "market"){
        add_market_order(newOrder);
    } else if (newOrder->type == "limit"){
        add_limit_order(newOrder);
    }

    // For printing purposes

        std::cout << "-------ADDING--------" << std::endl;
        std::string type_of_order = (bid_ask) ? "bid" : "ask";
        std::cout << "ID:   " << newOrder->order_id << "    " << newOrder->client_id << "   " << newOrder->type << "    " << type_of_order << "    $" << limit_price << "    " << quantity << "  shares    order time " << entry_time << std::endl;
        if (best_bid != nullptr) {
            std::cout << "BEST BID: $" << best_bid->GetLimitPrice() << " by " << best_bid->GetTopOrder()->client_id << std::endl;
        }else {
            std::cout << "EMPTY BID BOOK"<< std::endl;
        }
        if (best_ask != nullptr) {
            std::cout << "BEST ASK: $" << best_ask->GetLimitPrice() << std::endl;
        }else {
            std::cout << "EMPTY ASK BOOK"<< std::endl;
        }
        std::cout << "--------DONE---------" << std::endl;



    return newOrder;
}


void LOB::remove_limit_order(int order_id) {
    // If the order doesn't exist exit out of the function
    if (orders.find(order_id) == orders.end()) return;

    // Find the Order pointer for the order that is being removed
    Order* orderToRemove = orders[order_id];
    ActiveLOBMapAndTree mat = get_active_hash_map_tree(orderToRemove->bid_ask);
    Limit* parentLimit = (*mat.active_map_ptr)[orderToRemove->price];
    double parentLimitPrice = parentLimit->GetLimitPrice();

    parentLimit->RemoveOrder(orderToRemove);

    if (parentLimit->GetLimitSize() == 0){
        if (orderToRemove->bid_ask){
            if (best_bid->GetLimitPrice() == parentLimitPrice){
                best_bid = find_next_highest_limit(*mat.active_tree_ptr, parentLimitPrice);
            }
        } else {
            if (best_ask->GetLimitPrice() == parentLimitPrice){
                best_ask = find_next_lowest_limit(*mat.active_tree_ptr, parentLimitPrice);
            }
        }
        mat.active_map_ptr->erase(parentLimitPrice);
        mat.active_tree_ptr->erase(parentLimitPrice);
        delete parentLimit;
    }
    orders.erase(order_id);
    delete orderToRemove;
}


std::vector<Execution*> LOB::execute(bool is_limit = true){
    std::cout << "LOB - EXECUTE() \n";
    std::vector<Execution*> executions;

    match_market_orders(executions);
    if (is_limit){
        match_limit_orders(executions);
    }
    return executions;
}


Order* LOB::cancel_order(int order_id, const std::string& client_id, long long cancel_time) {
    if (orders.find(order_id) == orders.end()) {
        throw std::runtime_error("Order not found");
    }

    Order* orderToCancel = orders[order_id];
    if (client_id != orderToCancel->client_id)
        return nullptr;
    orderToCancel->cancel_time = cancel_time;
    Order* orderCopy = new Order (*orderToCancel);

    if (orderToCancel->type == "limit"){
        remove_limit_order(order_id);
    }
    return orderCopy;
}


void LOB::update_current_order_id(int order_id) {
    current_order_id = std::max(order_id + 1, current_order_id);
}


void LOB::update_current_execution_id(int execution_id) {
    current_execution_id = execution_id + 1;
}


void LOB::match_market_orders(std::vector<Execution*> &executions){
    while(!mkt_order_bid_q.empty() && best_ask != nullptr){
        Order* currentMarketOrder = mkt_order_bid_q.front();

        // handle a cancelled market order
        if (currentMarketOrder->cancel_time != -1){
            mkt_order_bid_q.pop();
            orders.erase(currentMarketOrder->order_id);
            delete currentMarketOrder;
            continue;
        }
        Order* topLimitOrder = best_ask->GetTopOrder();
        walk_one_book(false, &topLimitOrder, currentMarketOrder);
        Limit* topLimit = ask_tree[topLimitOrder->price];

        if (currentMarketOrder->client_id == topLimitOrder->client_id){
            break;
        }
        // execution is possible now

        int executionQuantity = (currentMarketOrder->quantity > topLimitOrder->quantity) ?  topLimitOrder->quantity : currentMarketOrder->quantity;
        long long executionTimeStamp = get_time_stamp();
        Execution* marketExecution = new Execution(current_execution_id, currentMarketOrder->order_id, currentMarketOrder->instrument_id, currentMarketOrder->client_id, topLimitOrder->price, executionQuantity, executionTimeStamp);
        Execution* limitExecution = new Execution(current_execution_id, topLimitOrder->order_id, topLimitOrder->instrument_id, topLimitOrder->client_id, topLimitOrder->price, executionQuantity, executionTimeStamp);
        executions.push_back(marketExecution);
        executions.push_back(limitExecution);
        current_execution_id++;
        if (currentMarketOrder->quantity > topLimitOrder->quantity){
            currentMarketOrder->quantity -= topLimitOrder->quantity;
            remove_limit_order(topLimitOrder->order_id);
        }else if (currentMarketOrder->quantity < topLimitOrder->quantity){
            topLimit->ReduceOrder(topLimitOrder, currentMarketOrder->quantity);
            orders.erase(currentMarketOrder->order_id);
            mkt_order_bid_q.pop();
            delete currentMarketOrder;
        }else{
            remove_limit_order(topLimitOrder->order_id);
            orders.erase(currentMarketOrder->order_id);
            mkt_order_bid_q.pop();
            delete currentMarketOrder;
        }
    }

    while(!mkt_order_ask_q.empty() && best_bid != nullptr){
        Order* currentMarketOrder = mkt_order_ask_q.front();
        // handle a cancelled market order
        if (currentMarketOrder->cancel_time != -1){
            mkt_order_ask_q.pop();
            orders.erase(currentMarketOrder->order_id);
            delete currentMarketOrder;
            continue;
        }
        Order* topLimitOrder = best_bid->GetTopOrder();
        walk_one_book(true, &topLimitOrder, currentMarketOrder);
        Limit* topLimit = bid_tree[topLimitOrder->price];

        if (currentMarketOrder->client_id == topLimitOrder->client_id){
            break;
        }
        int executionQuantity = (currentMarketOrder->quantity > topLimitOrder->quantity) ?  topLimitOrder->quantity : currentMarketOrder->quantity;

        long long executionTimeStamp = get_time_stamp();
        Execution* marketExecution = new Execution(current_execution_id, currentMarketOrder->order_id, currentMarketOrder->instrument_id, currentMarketOrder->client_id, topLimitOrder->price, executionQuantity, executionTimeStamp);
        Execution* limitExecution = new Execution(current_execution_id, topLimitOrder->order_id, topLimitOrder->instrument_id, topLimitOrder->client_id, topLimitOrder->price, executionQuantity, executionTimeStamp);
        executions.push_back(marketExecution);
        executions.push_back(limitExecution);
        current_execution_id++;
        if (currentMarketOrder->quantity > topLimitOrder->quantity){
            currentMarketOrder->quantity -= topLimitOrder->quantity;
            remove_limit_order(topLimitOrder->order_id);
        }else if (currentMarketOrder->quantity < topLimitOrder->quantity){
            topLimit->ReduceOrder(topLimitOrder, currentMarketOrder->quantity);
            orders.erase(currentMarketOrder->order_id);
            mkt_order_ask_q.pop();
            delete currentMarketOrder;
        }else{
            remove_limit_order(topLimitOrder->order_id);
            orders.erase(currentMarketOrder->order_id);
            mkt_order_ask_q.pop();
            delete currentMarketOrder;
        }
    }
}


void LOB::match_limit_orders(std::vector<Execution*> &executions) {
    while (best_bid != nullptr && best_ask != nullptr){
        Order* topBidOrder = best_bid->GetTopOrder();
        Order* topAskOrder = best_ask->GetTopOrder();

        std::cout << "TOP BID: $" << std::to_string(topBidOrder->price) << "\n";
        std::cout << "TOP ASK: $" <<  std::to_string(topAskOrder->price) << "\n";


        walk_book(&topBidOrder, &topAskOrder);

        Limit* topBidLimit = bid_tree[topBidOrder->price];
        Limit* topAskLimit = ask_tree[topAskOrder->price];

        if ((topBidOrder->client_id == topAskOrder->client_id ) || topBidOrder->price < topAskOrder->price){
            break;
        }

        int executionQuantity = (topBidOrder->quantity > topAskOrder->quantity) ? topAskOrder->quantity : topBidOrder->quantity;
        double executionPrice = (topBidOrder->entry_time < topAskOrder->entry_time) ? (topBidOrder->price) : (topAskOrder->price);
        long long executionTimeStamp = get_time_stamp();
        Execution* bidLimitExecution = new Execution(current_execution_id, topBidOrder->order_id, topBidOrder->instrument_id, topBidOrder->client_id, executionPrice, executionQuantity, executionTimeStamp);
        Execution* askLimitExecution = new Execution(current_execution_id, topAskOrder->order_id, topAskOrder->instrument_id, topAskOrder->client_id, executionPrice, executionQuantity, executionTimeStamp);
        current_execution_id++;
        executions.push_back(bidLimitExecution);
        executions.push_back(askLimitExecution);
        // Here there is a match and an execution needs to be created
        if (topBidOrder->quantity > topAskOrder->quantity)
        {
            topBidLimit->ReduceOrder(topBidOrder, topAskOrder->quantity);
            remove_limit_order(topAskOrder->order_id);
        }
        else if (topBidOrder->quantity < topAskOrder->quantity)
        {
            topAskLimit->ReduceOrder(topAskOrder, topBidOrder->quantity);
            remove_limit_order(topBidOrder->order_id);

        }
        else
        {
            remove_limit_order(topBidOrder->order_id);
            remove_limit_order(topAskOrder->order_id);
        }
    }
}


void LOB::walk_limits(Order **top_bid_order, Order **top_ask_order) {

    Order* tempTopBidOrder = (*top_bid_order);
    Order* tempTopAskOrder = (*top_ask_order);

    // Ensure initial pointers are not null
    if (tempTopBidOrder == nullptr || tempTopAskOrder == nullptr) {
        return;
    }

    while ((tempTopBidOrder->client_id == tempTopAskOrder->client_id) &&
           (tempTopBidOrder->next_order != nullptr || tempTopAskOrder->next_order != nullptr )){
        if (tempTopBidOrder->next_order != nullptr && tempTopAskOrder->next_order != nullptr){
            // we are making an assumption that to orders will not have the exact same time
            if (tempTopBidOrder->next_order->entry_time <= tempTopAskOrder->next_order->entry_time){
                tempTopBidOrder = tempTopBidOrder->next_order;
            } else {
                tempTopAskOrder = tempTopAskOrder->next_order;
            }
        }
        else {
            if (tempTopBidOrder->next_order != nullptr){
                tempTopBidOrder = tempTopBidOrder->next_order;
            }else {
                tempTopAskOrder = tempTopAskOrder->next_order;
            }
        }
    }
    if (tempTopBidOrder->client_id != tempTopAskOrder->client_id) {
        (*top_bid_order) = tempTopBidOrder;
        (*top_ask_order) = tempTopAskOrder;
    }
}


void LOB::walk_one_limit(Order **top_order, Order *market_order) {
    Order *tempTopLimitOrder =(*top_order);

    if (tempTopLimitOrder == nullptr){
        return;
    }

    while ((tempTopLimitOrder->client_id == market_order->client_id) &&
           (tempTopLimitOrder->next_order != nullptr)){
        tempTopLimitOrder = tempTopLimitOrder->next_order;
    }

    // we could still end up with a top_order with the same client_id as the market_order
    if (tempTopLimitOrder->client_id != market_order->client_id){
        (*top_order) = tempTopLimitOrder;
    }
}


void LOB::walk_book(Order **top_bid_order, Order **top_ask_order) {
    if ((*top_bid_order) == nullptr || (*top_ask_order) == nullptr) {
        return;
    }

    Order* tempTopBidOrder = (*top_bid_order);
    Order* tempTopAskOrder = (*top_ask_order);

    while (tempTopBidOrder->client_id == tempTopAskOrder->client_id){
        walk_limits(&tempTopBidOrder, &tempTopAskOrder);\

        if (tempTopBidOrder->client_id == tempTopAskOrder->client_id){
            Limit* nextBestBid = find_next_highest_limit(bid_tree, tempTopBidOrder->price);
            Limit* nextBestAsk = find_next_lowest_limit(ask_tree, tempTopAskOrder->price);

            if (nextBestBid == nullptr && nextBestAsk == nullptr) {
                break;
            }

            bool bidTimeLessOrEqual = tempTopBidOrder->entry_time <= tempTopAskOrder->entry_time;
            bool bidTimeGreater = tempTopBidOrder->entry_time > tempTopAskOrder->entry_time;

            if (bidTimeLessOrEqual) {
                if (nextBestAsk != nullptr && tempTopBidOrder->price >= nextBestAsk->GetLimitPrice()) {
                    tempTopAskOrder = nextBestAsk->GetTopOrder();
                } else if (nextBestBid != nullptr && nextBestBid->GetLimitPrice() >= tempTopAskOrder->price) {
                    tempTopBidOrder = nextBestBid->GetTopOrder();
                }
            } else if (bidTimeGreater) {
                if (nextBestBid != nullptr && nextBestBid->GetLimitPrice() >= tempTopAskOrder->price) {
                    tempTopBidOrder = nextBestBid->GetTopOrder();
                } else if (nextBestAsk != nullptr && tempTopBidOrder->price >= nextBestAsk->GetLimitPrice()) {
                    tempTopAskOrder = nextBestAsk->GetTopOrder();
                }
            }

            if ((nextBestBid != nullptr && nextBestBid->GetLimitPrice() < tempTopAskOrder->price) ||
                (nextBestAsk != nullptr && tempTopBidOrder->price < nextBestAsk->GetLimitPrice())) {
                break;
            }
        }
    }
    if (tempTopBidOrder->client_id != tempTopAskOrder->client_id){
        (*top_bid_order) = tempTopBidOrder;
        (*top_ask_order) = tempTopAskOrder;
    }
}


void LOB::walk_one_book(bool bid_ask, Order **top_limit_order, Order* market_order) {

    // Theoretically should never happen
    if ((*top_limit_order) == nullptr) {
        return;
    }

    Order* tempTopLimitOrder = (*top_limit_order);
    ActiveLOBMapAndTree mat = get_active_hash_map_tree(bid_ask);

    while (tempTopLimitOrder->client_id == market_order->client_id){
        walk_one_limit(&tempTopLimitOrder, market_order);
        if (tempTopLimitOrder->client_id == market_order->client_id){
            Limit* nextBestLimit = (bid_ask) ? find_next_highest_limit((*mat.active_tree_ptr), tempTopLimitOrder->price)
                                             : find_next_lowest_limit((*mat.active_tree_ptr), tempTopLimitOrder->price);

            if (nextBestLimit == nullptr) {
                break;
            }

            tempTopLimitOrder = nextBestLimit->GetTopOrder ();
        }
    }
    if (tempTopLimitOrder->client_id != market_order->client_id){
        (*top_limit_order) = tempTopLimitOrder;
    }
}


Limit* LOB::find_next_lowest_limit(const std::map<double, Limit*>& tree, const double& best_price) {
    auto it = tree.find(best_price);

    // Check if the iterator is valid and not the last element
    if (it != tree.end()) {
        ++it; // Move to the next element

        // Check if we have not reached the end of the map
        if (it != tree.end()) {
            return it->second; // Return the Limit* for the next highest price
        }
    }

    // If best_bid is at the end or not found, return nullptr
    return nullptr;
}


Limit* LOB::find_next_highest_limit(const std::map<double, Limit*>& tree, const double& best_price) {
    auto it = tree.find(best_price);

    // If best_price is not found Or it is the first element, return nullptr
    if (it == tree.end() || it == tree.begin()) {
        return nullptr;
    }

    // Move to the previous element (which exists since it is not at the beginning)
    --it;
    return it->second; // Return the Limit* for the next lowest price
}


int LOB::get_bid_volume_at_limit(double limit) const {
    return (bid_map.find(limit) != bid_map.end()) ? bid_map.at(limit)->GetLimitVolume() : -1;
}


int LOB::get_ask_volume_at_limit(double limit) const {
    return (ask_map.find(limit) != ask_map.end()) ? ask_map.at(limit)->GetLimitVolume() : -1;
}


void LOB::print_bid_book() {
    std::cout << "----- Bid Book -----" << std::endl;
    std::cout << std::left << std::setw(10) << "Price" << std::setw(15) << "Quantity" << std::setw(15) << "Total Orders" << std::endl;
    std::cout << std::setfill('-') << std::setw(40) << "-" << std::endl; // Separator

    for (const auto& limitPair : bid_tree) {
        double price = limitPair.first;
        Limit* limit = limitPair.second;
        int totalOrders = limit->GetLimitSize(); // Assuming GetTotalOrders() returns the number of orders at this price
        int totalQuantity = limit->GetLimitVolume(); // Assuming GetTotalQuantity() returns the total quantity of orders at this price

        std::cout << std::setfill(' ') << std::left;
        std::cout << std::setw(10) << price << std::setw(15) << totalQuantity << std::setw(15) << totalOrders << std::endl;\
    }
    std::cout << std::endl;
}


void LOB::print_ask_book() {
    std::cout << "----- Ask Book -----" << std::endl;
    std::cout << std::left << std::setw(10) << "Price" << std::setw(15) << "Quantity" << std::setw(15) << "Total Orders" << std::endl;
    std::cout << std::setfill('-') << std::setw(40) << "-" << std::endl; // Separator

    for (const auto& limitPair : ask_tree) {
        double price = limitPair.first;
        Limit* limit = limitPair.second;
        int totalOrders = limit->GetLimitSize(); // Assuming GetTotalOrders() returns the number of orders at this price
        int totalQuantity = limit->GetLimitVolume(); // Assuming GetTotalQuantity() returns the total quantity of orders at this price

        std::cout << std::setfill(' ') << std::left;
        std::cout << std::setw(10) << price << std::setw(15) << totalQuantity << std::setw(15) << totalOrders << std::endl;
    }
    std::cout << std::endl;
}


