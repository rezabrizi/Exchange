//
// Created by Reza Tabrizi on 12/24/23.
//
#include <queue>
#include <mutex>
#include <condition_variable>

#include "Message.h"


#ifndef LIMITORDERBOOK_MESSAGINGQUEUE_H
#define LIMITORDERBOOK_MESSAGINGQUEUE_H

class MessagingQueue{
    std::queue <std::unique_ptr<BaseMessage>> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push (std::unique_ptr<BaseMessage> message);
    bool pop(std::unique_ptr<BaseMessage>& message);
};

#endif //LIMITORDERBOOK_MESSAGINGQUEUE_H
