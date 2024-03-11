#include "../headers/CentralMessageSystem.h"

CentralMessageSystem::CentralMessageSystem() : b_shutdown(false), id(0)
{
    /**
     * Create subscription list
     */
    subscribers["AddOrderMessage"];
    subscribers["CancelOrderMessage"];
    subscribers["TradeExecutionMessage"];
    subscribers["OrderConfirmationMessage"];
    subscribers["AlertMessage"];

    // Run the worker method on a separate thread
    worker_thread = std::thread(&CentralMessageSystem::worker, this);
}


CentralMessageSystem::~CentralMessageSystem()
{
    if (worker_thread.joinable())
    {
        worker_thread.join();
    }
}


void CentralMessageSystem::shutdown()
{
    {
        std::unique_lock<std::mutex> lock(blocking_mux);
        b_shutdown = true;
        cv.notify_one();
    }

    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}


int CentralMessageSystem::assign_messaging_id()
{
    std::scoped_lock lock(m_id_mux);
    return id++;
}


void CentralMessageSystem::worker()
{
    while (true)
    {
        /**
         * The lock is made for the conditional variable
         * This mechanism allows for non-busy waiting by blocking the thread
         * until another message is published
         */
        {
            std::unique_lock<std::mutex> lock(blocking_mux);
            cv.wait(lock, [this] {
                return !m_system_queue.empty() || b_shutdown;
            });
        }

        if (b_shutdown)
        {
            break;
        }

        if (!m_system_queue.empty())
        {
            std::unique_ptr<BaseMessage> message;
            message = m_system_queue.pop();
            handle_message(std::move(message));
        }
    }
}


void CentralMessageSystem::handle_message(std::unique_ptr<BaseMessage> message)
{
    BaseMessage* messageToSend = nullptr;
    if (message->message_type == "AddOrderMessage")
    {
        messageToSend = dynamic_cast<AddOrderMessage*>(message.get());
    } else if (message->message_type == "CancelOrderMessage")
    {
        messageToSend = dynamic_cast<CancelOrderMessage*>(message.get());
    } else if (message->message_type == "TradeExecutionMessage")
    {
        messageToSend = dynamic_cast<TradeExecutionMessage*>(message.get());
    } else if (message->message_type == "OrderConfirmationMessage")
    {
        messageToSend = dynamic_cast<OrderConfirmationMessage*>(message.get());
    } else if (message->message_type == "AlertMessage")
    {
        messageToSend = dynamic_cast<SystemMessage*>(message.get());
    }

    if (messageToSend)
    {
        auto it = subscribers.find(messageToSend->message_type);
        for (const auto& subscriber: it->second)
        {
            subscriber(*messageToSend);
        }
    }
}


void CentralMessageSystem::publish(std::unique_ptr<BaseMessage> message)
{
    std::string messageType = message->message_type;
    auto messageSubscribers = subscribers.find(messageType);

    // If there is a subscription vector for message_type...
    if (messageSubscribers != subscribers.end())
    {
        // ... push the message to the queue
        m_system_queue.push(std::move(message));
    }

    cv.notify_one();
}


void CentralMessageSystem::subscribe(const std::string &topic, const SubscriberCallback& callback)
{
    auto messageSubscribers = subscribers.find(topic);

    if (messageSubscribers != subscribers.end())
    {
       messageSubscribers->second.push_back(callback);
    }
}

long long CentralMessageSystem::GetCurrentTimeStamp ()
{
    auto now = std::chrono::system_clock::now();

    // Convert time point to duration since epoch, then to milliseconds
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();


    long long currentTimestamp = static_cast<long long>(millis);
    return currentTimestamp;
}