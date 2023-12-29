//
// Created by Reza Tabrizi on 12/24/23.
//
#include "MessagingQueue.h"
#include "Message.h"


#ifndef LIMITORDERBOOK_CENTRALMESSAGESYSTEM_H
#define LIMITORDERBOOK_CENTRALMESSAGESYSTEM_H

using SubscriberCallback = std::function<void (const BaseMessage& message)>;

class CentralMessageSystem{
    MessagingQueue systemQueue;
    std::unordered_map <std::string, std::vector<SubscriberCallback>> subscribers;
    bool running;

    void Worker();
    void HandleMessage(std::unique_ptr <BaseMessage> message);

public:
    CentralMessageSystem();
    void Publish (std::unique_ptr<BaseMessage> message);
    void Subscribe (const std::string& topic, const SubscriberCallback& callback);
};
#endif //LIMITORDERBOOK_CENTRALMESSAGESYSTEM_H