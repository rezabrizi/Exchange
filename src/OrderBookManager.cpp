//
// Created by Reza Tabrizi on 12/24/23.
//

#include "../headers/OrderBookManager.h"

OrderBookManager::OrderBookManager(CentralMessageSystem &CMS) : cms(CMS) {
    StartUpOrderBook();
    CMS.Subscribe("AddOrderMessage", [this](const BaseMessage& message) {
        this->ProcessMessage(message);
    });

    CMS.Subscribe("CancelOrderMessage", [this](const BaseMessage& message) {
        this->ProcessMessage(message);
    });
}


void OrderBookManager::StartUpOrderBook() {
    //std::lock_guard<std::mutex> lock(orderBookMutex);
    std::string ordersQuery = R"(SELECT o.orderid,
                                    o.quantity,
                                    o.clientid,
                                    o.bidorask,
                                    o.instrumentid,
                                    o.price,
                                    o.type,
                                    o.entrytime,
                                    COALESCE(o.canceltime, 0) as canceltime,
                                    COALESCE(SUM(e.quantity), 0) as executed_quantity
                                FROM orders o
                                LEFT JOIN executions e ON o.orderkey = e.orderkey
                                GROUP BY o.orderkey
                                ORDER BY o.entrytime ASC)";

    pqxx::result R = db.query(ordersQuery);
    std::unordered_map<std::string, int> currOrderIds;

    for (auto row: R){
        int orderId = row["orderid"].as<int>();
        std::string instrumentId = row["instrumentid"].as<std::string>();
        std::string clientId = row["clientid"].as<std::string>();
        int quantity = row["quantity"].as<int>();
        int executedQuantity = row["executed_quantity"].as<int>();
        double limit = row["price"].as<double>();
        long long entryTime = row["entrytime"].as<long long>();
        long long cancelTime = row["canceltime"].as<long long>();
        bool bidOrAsk = row["bidorask"].as<bool>();
        std::string type = row["type"].as<std::string>();

        if (orderBook.find(instrumentId) == orderBook.end()) {
            orderBook[instrumentId] = new LOB();
        }

        LOB* currentLOB = orderBook[instrumentId];

        if (cancelTime == 0){
            if (executedQuantity < quantity){

                currentLOB->AddOrder(instrumentId, type, clientId, bidOrAsk, quantity-executedQuantity, limit, entryTime, orderId);

            }
        }
        currOrderIds[instrumentId] = std::max(currOrderIds[instrumentId], orderId);
    }


    for (const auto& item: currOrderIds){
        LOB* currentLOB = orderBook[item.first];
        currentLOB->UpdateCurrentOrderId(item.second);
    }

}


void OrderBookManager::ProcessMessage(const BaseMessage &message) {
    std::cout << "ORDER BOOK MANAGER - PROCESS () \n";

    if (message.messageType == "AddOrderMessage") {
        // we use a pointer to not have to worry about exception checking as the cast will result in a nullptr
        // if it is not valid
        // alternatively we could use a reference but we have to use a try catch blocks in case of a failed cast
        const AddOrderMessage *newOrderMessage = dynamic_cast <const AddOrderMessage *>(&message);

        if (newOrderMessage != nullptr) {
            try {
                if (orderBook.find(newOrderMessage->instrumentId) == orderBook.end()) {
                    std::cout<<newOrderMessage->instrumentId  << std::endl;
                    orderBook[newOrderMessage->instrumentId] = new LOB();
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception caught: " << e.what() << std::endl;
            }


            std::unique_ptr<BaseMessage> confirmedOrderMessage = AddOrder(newOrderMessage);
            cms.Publish(std::move(confirmedOrderMessage));

            std::vector<std::unique_ptr<TradeExecutionMessage>> executionMessages = CheckForMatch(
                    newOrderMessage->instrumentId, newOrderMessage->orderType);

            for (std::unique_ptr<TradeExecutionMessage> &execution: executionMessages) {
                std::unique_ptr<BaseMessage> executionMessage = std::move(execution);
                cms.Publish(std::move(executionMessage));
            }
        }
    }

    else if (message.messageType == "CancelOrderMessage"){
        if (const CancelOrderMessage* cancelOrderMessage = dynamic_cast<const CancelOrderMessage*> (&message)){
            std::unique_ptr<BaseMessage> cancellationConfirmationMessage = CancelOrder(cancelOrderMessage->instrumentId, cancelOrderMessage->orderId);
            cms.Publish(std::move(cancellationConfirmationMessage));
        }
    }
}


std::unique_ptr<OrderConfirmationMessage> OrderBookManager::AddOrder(const AddOrderMessage *message) {
    LOB* currentLOB = orderBook[message->instrumentId];
    Order* newOrder = currentLOB->AddOrder(message->instrumentId, message->orderType, message->clientId, message->bidOrAsk, message->quantity, message->limit, message->timestamp);
    WriteOrderToDB(newOrder);
    std::unique_ptr<OrderConfirmationMessage> confirmedOrderMessage = std::make_unique<OrderConfirmationMessage>(
            cms.AssignMessageId(), // int
            "OrderConfirmationMessage", // std::string
            LOB::GetTimeStamp(), // long long
            newOrder->clientId, // std::string
            newOrder->instrumentId, // std::string
            newOrder->bidOrAsk, // bool
            newOrder->price, // double
            newOrder->quantity, // int
            newOrder->type, // std::string
            newOrder->orderId,// int
            -1
    );
    return confirmedOrderMessage;
}


std::unique_ptr<OrderConfirmationMessage> OrderBookManager::CancelOrder(const std::string& instrumentId, int orderId) {
    try {
        LOB* currentLOB = orderBook[instrumentId];
        Order* canceledOrder = currentLOB->CancelOrder(orderId, LOB::GetTimeStamp());

        // Perform additional operations if the order is successfully canceled
        CancelOrderInDB(instrumentId, orderId, canceledOrder->cancelTime);
        std::unique_ptr<OrderConfirmationMessage> cancellationConfirmationMessage = std::make_unique<OrderConfirmationMessage>(
                cms.AssignMessageId(), // int
                "OrderConfirmationMessage", // std::string
                LOB::GetTimeStamp(), // long long
                canceledOrder->clientId, // std::string
                canceledOrder->instrumentId, // std::string
                canceledOrder->bidOrAsk, // bool
                canceledOrder->price, // double
                canceledOrder->quantity, // int
                canceledOrder->type, // std::string
                canceledOrder->orderId, // int
                canceledOrder->cancelTime
        );
        delete canceledOrder;
        return cancellationConfirmationMessage;

    } catch (const std::exception& e) {
        std::cerr << "Failed to cancel order: " << e.what() << std::endl;
        // Handle the exception here (e.g., return a null pointer or an error message)
        // For example, return a null pointer:
        return nullptr;
    }
}


std::vector<std::unique_ptr<TradeExecutionMessage>> OrderBookManager::CheckForMatch(const std::string& instrumentId, const std::string& type) {
    LOB* currentLOB = orderBook[instrumentId];
    bool isLimit = (type == "limit");
    std::vector<Execution*> executions = currentLOB->Execute(isLimit);
    std::vector<std::unique_ptr<TradeExecutionMessage>> executionMessages;
    executionMessages.reserve(executions.size());

    for (Execution* const& execution: executions){
        WriteExecutionToDB(execution);
        executionMessages.push_back(std::make_unique<TradeExecutionMessage>(
                cms.AssignMessageId(),
                "execution",
                execution->GetTimestamp(),
                execution->GetOrderId(),
                execution->GetClientId(),
                execution->GetInstrumentId(),
                execution->GetPrice(),
                execution->GetQuantity()
                ));
    }

    return executionMessages;
}


void OrderBookManager::WriteOrderToDB(const Order *order) {
    try {
        std::string orderKey = order->instrumentId+std::to_string(order->orderId);
        std::string orderQuery = "INSERT INTO Orders (OrderKey, OrderID, InstrumentID, ClientID, Price, Quantity, BidOrAsk, Type, EntryTime, CancelTime) VALUES ('"
                                 + orderKey + "', "
                                 + std::to_string(order->orderId) + ", '"
                                 + order->instrumentId + "', '"
                                 + order->clientId + "', "
                                 + std::to_string(order->price) + ", "
                                 + std::to_string(order->quantity) + ", "
                                 + (order->bidOrAsk ? "TRUE" : "FALSE") + ", '"
                                 + order->type + "', "
                                 + std::to_string(order->entryTime) + ", "
                                 + ((order->cancelTime != -1) ? std::to_string(order->cancelTime) : "NULL")
                                 + ")";

        db.query(orderQuery);
    } catch (const std::exception& e) {
        std::cerr << "Database query failed: " << e.what() << std::endl;
    }
}


void OrderBookManager::WriteExecutionToDB(const Execution *execution) {
    try {
        std::string orderKey = execution->GetInstrumentId()+std::to_string(execution->GetOrderId());
        std::string executionQuery = "INSERT INTO Executions (ExecutionID, OrderKey, ExecutionTime, Price, Quantity) VALUES ("
                                     + std::to_string(execution->GetExecutionId()) + ", '"
                                     + orderKey + "', "
                                     + std::to_string(execution->GetTimestamp()) + ", "
                                     + std::to_string(execution->GetPrice()) + ", "
                                     + std::to_string(execution->GetQuantity())
                                     + ")";
        db.query(executionQuery);
    } catch (const std::exception& e) {
        std::cerr << "Database query failed: " << e.what() << std::endl;
    }
}


void OrderBookManager::CancelOrderInDB(const std::string &instrumentId, int orderId, long long cancelTime) {
    try {
        std::string orderkey = instrumentId + std::to_string(orderId);
        std::string updateOrderQuery = "UPDATE Orders SET CancelTime = "
                                       + std::to_string(cancelTime)
                                       + " WHERE OrderKey = '"
                                       + orderkey
                                       + "'";

        db.query(updateOrderQuery);
    } catch (const std::exception& e) {
        std::cerr << "Database query failed: " << e.what() << std::endl;
    }
}