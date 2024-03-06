//
// Created by Reza Tabrizi on 12/29/23.
//
#include "../headers/CentralMessageSystem.h"


CentralMessageSystem::CentralMessageSystem() : shouldShutdown(false), currentId(0){
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


void CentralMessageSystem::Shutdown() {

    {
        std::unique_lock<std::mutex> lock(mtx);
        shouldShutdown = true;
        cv.notify_one();
    }


    if (workerThread.joinable()) {
        workerThread.join();
    }
}


int CentralMessageSystem::AssignMessageId() {
    return currentId++;
}


void CentralMessageSystem::Worker(){
    while (true){
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] {
            return !systemQueue.empty() || shouldShutdown;
        });

        // If shutdown signal received, exit the loop
        if (shouldShutdown) break;

        systemQueue.wait();

        while (!systemQueue.empty())
        {
            std::unique_ptr<BaseMessage> message;
            message = systemQueue.pop();
            HandleMessage(std::move(message));
        }
    }
}


void CentralMessageSystem::HandleMessage(std::unique_ptr<BaseMessage> message) {
    std::cout << "CENTRAL MESSAGING SYSTEM - HANDLE MESSAGE () \n";

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
    std::cout << "CENTRAL MESSAGING SYSTEM - PUBLISH () - " << message->messageType << "\n";
    auto messageSubscribers = subscribers.find(messageType);

    std::cout << "pushed the message";
    if (messageSubscribers != subscribers.end()){
        systemQueue.push(std::move(message));
    }

    cv.notify_one();
}


void CentralMessageSystem::Subscribe(const std::string &topic, const SubscriberCallback& callback) {
    auto messageSubscribers = subscribers.find(topic);

    if (messageSubscribers != subscribers.end()){
       messageSubscribers->second.push_back(callback);
    }
}