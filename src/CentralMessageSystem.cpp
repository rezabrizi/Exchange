//
// Created by Reza Tabrizi on 12/29/23.
//
#include "../headers/CentralMessageSystem.h"
#include <iostream>

CentralMessageSystem::CentralMessageSystem() : running(true), currentId(0){
    subscribers["AddOrderMessage"];
    subscribers["CancelOrderMessage"];
    subscribers["TradeExecutionMessage"];
    subscribers["OrderConfirmationMessage"];
    subscribers["AlertMessage"];

    workerThread = std::thread(&CentralMessageSystem::Worker, this);
}

CentralMessageSystem::~CentralMessageSystem() {
    if (workerThread.joinable()) {
        workerThread.join();
    }
}


int CentralMessageSystem::AssignMessageId() {
    return currentId++;
}


void CentralMessageSystem::Worker(){
    while (running){
        std::cout << "in worker method" << std::endl;
        std::unique_ptr<BaseMessage> message;
        if (systemQueue.pop(message)){
            std::cout << "popped" << std::endl;
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
        messageToSend = dynamic_cast<SystemMessage*>(message.get());
    }

    if (messageToSend){
        auto it = subscribers.find(messageToSend->messageType);
        for (const auto& subscriber: it->second){
            subscriber(*messageToSend);
        }
    }
}

void CentralMessageSystem::Publish(std::unique_ptr<BaseMessage> message) {
    std::string messageType = message->messageType;
    std::cout << "message with topic: " << messageType << " was received" << std::endl;
    auto messageSubscribers = subscribers.find(messageType);

    if (messageSubscribers != subscribers.end()){
        std::cout << "trying to push to queue"<< std::endl;
        systemQueue.push(std::move(message));

    }
}

void CentralMessageSystem::Subscribe(const std::string &topic, const SubscriberCallback& callback) {
    auto messageSubscribers = subscribers.find(topic);

    if (messageSubscribers != subscribers.end()){
       messageSubscribers->second.push_back(callback);
    }
}