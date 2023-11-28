//
// Created by Reza Tabrizi on 11/25/23.
//
#include "Message.h"
#include "ISubject.h"
#include "Firm.h"
#include <queue>
#include <unordered_set>



#ifndef LIMITORDERBOOK_MESSAGINGSYSTEM_H
#define LIMITORDERBOOK_MESSAGINGSYSTEM_H


class MessageSystem : public ISubject, public IObserver{
    int currentSequenceId;
    std::queue <Message> messages;
    std::list <IObserver*> observers; //MessagingSystem is a subject and other objects observe its behavior
    // so if a messaging system receives a message then the order books observes that behavior by the messaging system calling
    // the update function of the order book

    // matching engine is the subject and the messaging system is observing it by having an update method.
    // the update method works by sending notification to the corresponding firm

    // matching engine create a message ssaying AAPL SLD JaneStreet 100 200
    // matching engine create a message ssaying AAPL BOUGHT GSCO 100 200

    // the messaging system's update function gets triggered by the matching engine and calls the sendMessage function to both janestreet and GSCO
    std::unordered_set <std::string> subscribers;

public:

    /**
     * receive messages () -> receive messages from different firms and put them in queue if they satisfy the validation conditions based on the message
     * process message () -> process a valid message and call notify to notify the limit order book
     * update () -> is listening to be called from the matching engine in case there are order executions
     * subscribe/unsubscribe() -> tradings firms subscribing to a the system
     * sendMessage-> this gets called in the update function and sends message to the responsible firm
     */
     void attach(IObserver* observer) override;
     void detach(IObserver* observer) override;
     void notify() override;
     void update(const Message& message) override;
     void subscribe(const Firm& firm);
     void unsubscribe(const Firm& firm);
     void receiveMessage(const Message& message);
     void processMessages();
     void sendMessage(const Message& message);


};



#endif //LIMITORDERBOOK_MESSAGINGSYSTEM_H
