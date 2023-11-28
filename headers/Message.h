//
// Created by Reza Tabrizi on 11/25/23.
//
#include <string>
#include <chrono>
#include <utility>

#ifndef LIMITORDERBOOK_MESSAGE_H
#define LIMITORDERBOOK_MESSAGE_H

enum class MessageAction{
    ADD, // can place bid or ask ex. janestreet ADD BID AAPL 100.00 200
    CANCEL, // can cancel an order
    REDUCE, // can reduce quantity
    CANREJ, // if you cancel incorrectly this then do canrej
    NOTIF, // notify certain firms
    ALERT, // alert certain firms
    CONFIG // configuration
};


struct Message {
    int sequenceId; /** Required - Message ID */
    std::string publisherId; /** Required - Who generates the message - ex. NASDAQ */
    std::string topic; /** Required - Who the message is intended for - ex. JaneStreet */
    MessageAction action; /** Required - the action of the message */
    long long timestamp; /** Required - timestamp of the message */
    std::string instrumentId; /** Optional - If the message action is an ADD, an instrument name is required */
    int orderId; /** Optional - If the message action is a CANCEL, CANCELREJ, or REDUCE, the orderId is required */
    double price; /** Optional - If the message action is an ADD, then the price is required */
    int quantity; /** Optional - If the message action is an ADD or REDUCE then the price is required */
    Message(int sequenceId, std::string publisherId, std::string topic, MessageAction action, long long timestamp,
            std::string instrumentId = "", int orderId = 0, double price = 0.0, int quantity = 0);
};

Message::Message(int sequenceId, std::string publisherId, std::string topic, MessageAction action,
                 long long timestamp, std::string instrumentId, int orderId, double price, int quantity)
        : sequenceId(sequenceId), publisherId(std::move(publisherId)), topic(std::move(topic)), action(action),
          timestamp(timestamp), instrumentId(std::move(instrumentId)), orderId(orderId), price(price), quantity(quantity) {}


#endif //LIMITORDERBOOK_MESSAGE_H
