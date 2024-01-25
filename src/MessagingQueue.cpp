//
// Created by Reza Tabrizi on 12/25/23.
//
#include "../headers/MessagingQueue.h"

void MessagingQueue::push (std::unique_ptr<BaseMessage> message){
    std::lock_guard<std::mutex> lock (mtx);
    queue.push(std::move(message));
    cv.notify_one();
}

bool MessagingQueue::pop(std::unique_ptr<BaseMessage>& message){
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]{return !queue.empty(); });
    if (queue.empty()){
        return false;
    }

    message = std::move(queue.front());
    queue.pop();
    return true;
}

bool MessagingQueue::empty()  {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.empty();
}
