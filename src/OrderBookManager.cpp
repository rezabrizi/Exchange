//
// Created by Reza Tabrizi on 12/24/23.
//
#include "../headers/OrderBookManager.h"

OrderBookManager::OrderBookManager(CentralMessageSystem &CMS) : CMS(CMS) {
    CMS.Subscribe("AddOrderMessage", [this](const BaseMessage& message) {
        this->ProcessMessage(message);
    });

    CMS.Subscribe("CancelOrderMessage", [this](const BaseMessage& message) {
        this->ProcessMessage(message);
    });
}


void OrderBookManager::ProcessMessage(const BaseMessage &message) {
    if (const auto* addOrderMessage = dynamic_cast<const AddOrderMessage*>(&message)){
        if (OrderBook.find(addOrderMessage->instrumentId) == OrderBook.end()){
            OrderBook[addOrderMessage->instrumentId] = new LOB();
        }
        OrderConfirmationMessage newOrder = AddOrder(*addOrderMessage);
        std::unique_ptr<BaseMessage> newOrderPtr = std::make_unique<OrderConfirmationMessage>(newOrder);
        CMS.Publish(std::move(newOrderPtr));

        std::vector<TradeExecutionMessage> executions = CheckForMatch(addOrderMessage->instrumentId);

        if(!executions.empty()){
            for (const TradeExecutionMessage& execution: executions){
                std::unique_ptr<BaseMessage> newExecuionPtr = std::make_unique<TradeExecutionMessage>(execution);
                CMS.Publish(std::move(newExecuionPtr));
            }
        }
    }

    else if (const auto* cancelOrderMessage = dynamic_cast <const CancelOrderMessage*>(&message)){
        if (OrderBook.find(cancelOrderMessage->instrumentId) != OrderBook.end()){
            OrderConfirmationMessage canceledOrder = CancelOrder(cancelOrderMessage->orderId);
            std::unique_ptr<BaseMessage> newOrderPtr = std::make_unique<OrderConfirmationMessage>(canceledOrder);
            CMS.Publish(std::move(newOrderPtr));
        }
    }
}

