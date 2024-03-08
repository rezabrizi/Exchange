#pragma once

#include "exchange_common.h"
#include "MessagingQueue.h"
#include "Message.h"


using SubscriberCallback = std::function<void (const BaseMessage& message)>;

class CentralMessageSystem
{

public:
    /**
     * @brief CentralMessageSystem Constructor
     */
    CentralMessageSystem();


    /**
     * @brief CentralMessageSystem Destructor
     */
    ~CentralMessageSystem();


    /**
     * @brief Assign a system wide message id to a message
     * @return messaging id to be used for the next message
     */
    int assign_messaging_id();


    /**
     * @brief publish a message to the system messaging queue
     * @param message message to be published
     */
    void publish (std::unique_ptr<BaseMessage> message);


    /**
     * @brief allow a component to subscribe to a subscription list
     * @param topic the messaging topic to subscribe to
     * @param callback the function to be called once there is a publication for that message type
     */
    void subscribe (const std::string& topic, const SubscriberCallback& callback);


    /**
     * @brief turn off the central messaging system
     */
    void shutdown();

    long long GetCurrentTimeStamp ();


private:
    /**
     * @brief method to handle messages from the system messaging queue
     */
    void worker();


    /**
     * @brief dispatch messages to the subscribers
     * @param message message to dispatch
     */
    void handle_message(std::unique_ptr <BaseMessage> message);


    int id;
    bool b_shutdown;
    MessagingQueue<std::unique_ptr<BaseMessage>> m_system_queue;
    std::unordered_map <std::string, std::vector<SubscriberCallback>> subscribers;
    std::thread worker_thread;
    std::mutex m_id_mux;
    std::mutex blocking_mux;
    std::condition_variable cv;
};
