#pragma once


#include "exchange_common.h"
#include "MessagingQueue.h"
#include "Message.h"


using SubscriberCallback = std::function<void (const BaseMessage& message)>;

class CentralMessageSystem{
    std::thread workerThread;

    int currentId;
    MessagingQueue<std::unique_ptr<BaseMessage>> systemQueue;
    std::unordered_map <std::string, std::vector<SubscriberCallback>> subscribers;
    bool shouldShutdown;
    std::mutex mtx;
    std::condition_variable cv;


    void Worker();
    void HandleMessage(std::unique_ptr <BaseMessage> message);

public:
    CentralMessageSystem();
    ~CentralMessageSystem();
    int AssignMessageId();
    void Publish (std::unique_ptr<BaseMessage> message);
    void Subscribe (const std::string& topic, const SubscriberCallback& callback);
    void Shutdown();
};
