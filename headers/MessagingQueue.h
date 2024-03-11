#pragma once
#include "exchange_common.h"

/**
 * @class MessagingQueue
 * @brief Tread-safe queue
 * @tparam T type of the queue object
 */

template<typename T>
class MessagingQueue {

public:

    /**
     * @brief push an item to the queue
     * @param message mesasge to push
     */
    void push(T message) {
       std::scoped_lock lock(mux_queue);
       queue.emplace(std::move(message));
    }


    /**
     * @brief pop an item from the queue
     * @return the first in item in the queue
     */
    T pop() {
        std::scoped_lock lock(mux_queue);
        auto t = std::move(queue.front());

        queue.pop();
        return t;
    }


    /**
     * @brief check if the queue is empty
     * @return whether the queue is empty
     */
    bool empty() {
        std::scoped_lock<std::mutex> lock(mux_queue);
        return queue.empty();
    }


private:
    std::queue<T> queue;
    std::mutex mux_queue;
};
