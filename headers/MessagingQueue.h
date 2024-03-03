#pragma once
#include "exchange_common.h"
#include "Message.h"



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
