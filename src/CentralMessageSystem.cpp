//
// Created by Reza Tabrizi on 12/29/23.
//
#include "../headers/CentralMessageSystem.h"

CentralMessageSystem::CentralMessageSystem() : running(true){
    Worker();
    subscribers["AddOrderMessage"];
    subscribers["CancelOrderMessage"];
    subscribers["TradeExecutionMessage"];
    subscribers["OrderConfirmationMessage"];
    subscribers["AlertMessage"];
}

void CentralMessageSystem::Worker(){
        while (running){
            std::unique_ptr<BaseMessage> message;
            if (systemQueue.pop(message)){
                HandleMessage(std::move(message));
            }
        }
    }

void CentralMessageSystem::HandleMessage(std::unique_ptr<BaseMessage> message) {
        BaseMessage* messageToSend = nullptr;

        if (message->messageType == "AddOrderMessage"){
            messageToSend = dynamic_cast<AddOrderMessage*>(message.get());
        } else if (message->messageType == "CancelOrderMessage"){
            messageToSend = dynamic_cast<CancelOrderMessage*>(message.get());
        } else if (message->messageType == "TradeExecutionMessage"){
            messageToSend = dynamic_cast<TradeExecutionMessage*>(message.get());
        } else if (message->messageType == "OrderConfirmationMessage"){
            messageToSend = dynamic_cast<OrderConfirmationMessage*>(message.get());
        } else if (message->messageType == "AlertMessage"){
            messageToSend = dynamic_cast<AlertMessage*>(message.get());
        }

        if (messageToSend){
            auto it = subscribers.find(messageToSend->messageType);
            for (const auto& subscriber: it->second){
                subscriber(*messageToSend);
            }
        }
    }

void CentralMessageSystem::Publish(std::unique_ptr<BaseMessage> message) {
    std::string topic = message->messageType;
    auto messageSubscribers = subscribers.find(topic);

    if (messageSubscribers != subscribers.end()){
        systemQueue.push(std::move(message));
    }
}

void CentralMessageSystem::Subscribe(const std::string &topic, const SubscriberCallback& callback) {
    auto messageSubscribers = subscribers.find(topic);

    if (messageSubscribers != subscribers.end()){
       messageSubscribers->second.push_back(callback);
    }
}