#pragma once
#include "exchange_common.h"
#include "Message.h"



template<typename T>
class MessagingQueue {

public:

    void push(T message) {
       std::scoped_lock lock(muxQueue);
       queue.emplace(std::move(message));

       std::unique_lock<std::mutex> ul(muxBlocking);
       cvBlocking.notify_one();
    }

    T pop() {
        std::scoped_lock lock(muxQueue);
        auto t = std::move(queue.front());

        queue.pop();
        return t;
    }

    // Inline definition of empty method
    bool empty() {
        std::scoped_lock<std::mutex> lock(muxQueue);
        return queue.empty();
    }

    void wait()
    {
        while (empty())
        {
            std::unique_lock<std::mutex> ul (muxBlocking);
            cvBlocking.wait(ul);
        }
    }


private:
    std::queue<T> queue;
    std::mutex muxQueue;


    std::condition_variable cvBlocking;
    std::mutex muxBlocking;
};
