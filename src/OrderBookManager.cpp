#include "../headers/OrderBookManager.h"

OrderBookManager::OrderBookManager(CentralMessageSystem &CMS, DBConnection& db) : cms(CMS), db(db){
    start_up_order_book();
    CMS.subscribe("AddOrderMessage", [this](const BaseMessage &message) {
        this->process_message(message);
    });

    CMS.subscribe("CancelOrderMessage", [this](const BaseMessage &message) {
        this->process_message(message);
    });
}


void OrderBookManager::start_up_order_book() {
    //std::lock_guard<std::mutex> lock(order_book_mutex);
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


    pqxx::result order_db_result = db.query(ordersQuery);
    std::unordered_map<std::string, int> currOrderIds;

    for (auto row: order_db_result){
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

        if (order_book.find(instrumentId) == order_book.end()) {
            order_book[instrumentId] = new LOB();
        }

        LOB* currentLOB = order_book[instrumentId];

        if (cancelTime == 0)
        {
            if (executedQuantity < quantity)
            {
                currentLOB->add_order(instrumentId, type, clientId, bidOrAsk, quantity - executedQuantity, limit,
                                      entryTime, orderId);
            }
        }
        currOrderIds[instrumentId] = std::max(currOrderIds[instrumentId], orderId);
    }

    std::string execution_query = "SELECT o.InstrumentID, MAX(e.ExecutionID) AS LastExecutionID "
                                  "FROM Executions e JOIN Orders o ON e.OrderKey = o.OrderKey "
                                  "GROUP BY o.InstrumentID;";
    pqxx::result execution_db_result = db.query (execution_query);
    std::unordered_map<std::string, int> currExecutionIds;

    for (auto row: execution_db_result)
    {
        std::string instrument_id = row["InstrumentID"].as<std::string>();
        int lastExecution_id = row["LastExecutionID"].as<int>();
        currExecutionIds[instrument_id] = lastExecution_id;
    }

    for (const auto& item: currOrderIds){
        LOB* currentLOB = order_book[item.first];
        currentLOB->update_current_order_id(item.second);

        // if there are any executions for the current instrument id then update the execution id
        if (currExecutionIds.find(item.first) != currExecutionIds.end())
        {
            currentLOB->update_current_execution_id(currExecutionIds[item.first]);
        }
        std::cout << "Instrument " << item.first << " - Current Execution ID: " << currentLOB->get_current_execution_id() << "\n";
        currentLOB->print_ask_book();
        currentLOB->print_bid_book();
    }

}


void OrderBookManager::process_message(const BaseMessage &message) {
    std::cout << "ORDER BOOK MANAGER - PROCESS () \n";

    if (message.message_type == "AddOrderMessage") {
        const AddOrderMessage *newOrderMessage = dynamic_cast <const AddOrderMessage *>(&message);

        if (newOrderMessage != nullptr) {
            try {
                if (order_book.find(newOrderMessage->instrument_id) == order_book.end()) {
                    std::cout << newOrderMessage->instrument_id << std::endl;
                    order_book[newOrderMessage->instrument_id] = new LOB();
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception caught: " << e.what() << std::endl;
            }


            std::unique_ptr<BaseMessage> confirmedOrderMessage = add_order(newOrderMessage);
            cms.publish(std::move(confirmedOrderMessage));

            std::vector<std::unique_ptr<TradeExecutionMessage>> executionMessages = check_for_match(
                    newOrderMessage->instrument_id, newOrderMessage->order_type);

            for (std::unique_ptr<TradeExecutionMessage> &execution: executionMessages) {
                std::unique_ptr<BaseMessage> executionMessage = std::move(execution);
                cms.publish(std::move(executionMessage));
            }
        }
    }

    else if (message.message_type == "CancelOrderMessage"){
        if (const CancelOrderMessage* cancelOrderMessage = dynamic_cast<const CancelOrderMessage*> (&message)){
            std::unique_ptr<BaseMessage> cancellationConfirmationMessage = cancel_order(
                    cancelOrderMessage->instrument_id, cancelOrderMessage->client_id, cancelOrderMessage->order_id);
            if (cancellationConfirmationMessage != nullptr)
            {
                cms.publish(std::move(cancellationConfirmationMessage));
            }
        }
    }
}


std::unique_ptr<OrderConfirmationMessage> OrderBookManager::add_order(const AddOrderMessage *message) {
    LOB* currentLOB = order_book[message->instrument_id];
    Order* newOrder = currentLOB->add_order(message->instrument_id, message->order_type, message->client_id,
                                            message->bid_ask, message->quantity, message->limit, message->timestamp);
    write_order_to_db(newOrder);
    std::unique_ptr<OrderConfirmationMessage> confirmedOrderMessage = std::make_unique<OrderConfirmationMessage>(
            cms.assign_messaging_id(), // int
            "OrderConfirmationMessage", // std::string
            LOB::get_time_stamp(), // long long
            newOrder->client_id, // std::string
            newOrder->instrument_id, // std::string
            newOrder->bid_ask, // bool
            newOrder->price, // double
            newOrder->quantity, // int
            newOrder->type, // std::string
            newOrder->order_id,// int
            -1
    );
    return confirmedOrderMessage;
}


std::unique_ptr<OrderConfirmationMessage> OrderBookManager::cancel_order(const std::string& instrument_id, const std::string& client_id, int order_id) {
    try {
        if (order_book.find(instrument_id) == order_book.end())
        {
            return nullptr;
        }
        LOB* currentLOB = order_book[instrument_id];
        Order* canceledOrder = currentLOB->cancel_order(order_id, client_id, LOB::get_time_stamp());
        if (canceledOrder == nullptr)
            return nullptr;

        // Perform additional operations if the order is successfully canceled
        cancel_order_in_db(instrument_id, order_id, canceledOrder->cancel_time);
        std::unique_ptr<OrderConfirmationMessage> cancellationConfirmationMessage = std::make_unique<OrderConfirmationMessage>(
                cms.assign_messaging_id(), // int
                "OrderConfirmationMessage", // std::string
                LOB::get_time_stamp(), // long long
                canceledOrder->client_id, // std::string
                canceledOrder->instrument_id, // std::string
                canceledOrder->bid_ask, // bool
                canceledOrder->price, // double
                canceledOrder->quantity, // int
                canceledOrder->type, // std::string
                canceledOrder->order_id, // int
                canceledOrder->cancel_time
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


std::vector<std::unique_ptr<TradeExecutionMessage>> OrderBookManager::check_for_match(const std::string& instrument_id, const std::string& type) {
    std::cout << "ORDER BOOK MANAGER - CHEKFORMATCH() \n";
    LOB* currentLOB = order_book[instrument_id];
    bool isLimit = (type == "limit");
    std::vector<Execution*> executions = currentLOB->execute(isLimit);
    std::vector<std::unique_ptr<TradeExecutionMessage>> executionMessages;
    executionMessages.reserve(executions.size());

    for (Execution* const& execution: executions){
        write_execution_to_db(execution);
        executionMessages.push_back(std::make_unique<TradeExecutionMessage>(
                cms.assign_messaging_id(),
                "TradeExecutionMessage",
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


void OrderBookManager::write_order_to_db(const Order *order) {
    try {
        std::string orderKey = order->instrument_id + std::to_string(order->order_id);
        std::string orderQuery = "INSERT INTO Orders (OrderKey, OrderID, InstrumentID, ClientID, Price, Quantity, BidOrAsk, Type, EntryTime, CancelTime) VALUES ('"
                                 + orderKey + "', "
                                 + std::to_string(order->order_id) + ", '"
                                 + order->instrument_id + "', '"
                                 + order->client_id + "', "
                                 + std::to_string(order->price) + ", "
                                 + std::to_string(order->quantity) + ", "
                                 + (order->bid_ask ? "TRUE" : "FALSE") + ", '"
                                 + order->type + "', "
                                 + std::to_string(order->entry_time) + ", "
                                 + ((order->cancel_time != -1) ? std::to_string(order->cancel_time) : "NULL")
                                 + ")";

        db.query(orderQuery);
    } catch (const std::exception& e) {
        std::cerr << "Database query failed: " << e.what() << std::endl;
    }
}


void OrderBookManager::write_execution_to_db(const Execution *execution) {
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


void OrderBookManager::cancel_order_in_db(const std::string &instrument_id, int order_id, long long cancel_time) {
    try {
        std::string orderkey = instrument_id + std::to_string(order_id);
        std::string updateOrderQuery = "UPDATE Orders SET CancelTime = "
                                       + std::to_string(cancel_time)
                                       + " WHERE OrderKey = '"
                                       + orderkey
                                       + "'";

        db.query(updateOrderQuery);
    } catch (const std::exception& e) {
        std::cerr << "Database query failed: " << e.what() << std::endl;
    }
}