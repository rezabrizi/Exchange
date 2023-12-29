//
// Created by Reza Tabrizi on 12/24/23.
//
#include "MessagingQueue.h"
#include "Message.h"


#ifndef LIMITORDERBOOK_CENTRALMESSAGESYSTEM_H
#define LIMITORDERBOOK_CENTRALMESSAGESYSTEM_H

using SubscriberCallback = std::function<void (const std::string& message)>;

class CentralMessageSystem{
    MessagingQueue systemQueue;
    std::vector <std::thread> workers;
    std::unordered_map <std::string, std::vector<SubscriberCallback>> subscribers;
    bool running;

public:
    void publish (const BaseMessage& message);
    void subscribe (const std::string& topic, SubscriberCallback callback);
};
#endif //LIMITORDERBOOK_CENTRALMESSAGESYSTEM_H