//
// Created by Reza Tabrizi on 12/24/23.
//
#include <queue>
#include <mutex>
#include <condition_variable>

#include "Message.h"


#ifndef LIMITORDERBOOK_MESSAGINGQUEUE_H
#define LIMITORDERBOOK_MESSAGINGQUEUE_H

template<typename T>
class MessagingQueue {
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    // Inline definition of push method
    void push(T message) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(message));
        cv.notify_one();
    }

    // Inline definition of pop method
    bool pop(T& message) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !queue.empty(); });

        if (!queue.empty()) {
            message = std::move(queue.front());
            queue.pop();
            return true;
        }
        return false;
    }

    // Inline definition of empty method
    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }
};

#endif //LIMITORDERBOOK_MESSAGINGQUEUE_H
