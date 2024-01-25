//
// Created by Reza Tabrizi on 12/24/23.
//
#include "MessagingQueue.h"
#include "Message.h"
#include <thread>


#ifndef LIMITORDERBOOK_CENTRALMESSAGESYSTEM_H
#define LIMITORDERBOOK_CENTRALMESSAGESYSTEM_H

using SubscriberCallback = std::function<void (const BaseMessage& message)>;

class CentralMessageSystem{
    std::thread workerThread;

    int currentId;
    MessagingQueue systemQueue;
    std::unordered_map <std::string, std::vector<SubscriberCallback>> subscribers;
    bool queueEmpty;
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
#endif //LIMITORDERBOOK_CENTRALMESSAGESYSTEM_H