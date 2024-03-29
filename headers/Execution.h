#pragma once

#include "exchange_common.h"


/**
 * @class Execution
 * @brief defines an execution class
 */
class Execution {
public:
    /**
     * @brief construciton of the execution
     * @param executionId
     * @param orderId
     * @param instrumentId
     * @param clientId
     * @param price
     * @param quantity
     * @param timestamp
     */
    Execution(int executionId, int orderId, std::string instrumentId, std::string clientId, double price, int quantity,
              long long timestamp) :
            execution_id(executionId), order_id(orderId), instrument_id(std::move(instrumentId)),
            client_id(std::move(clientId)), price(price),
            quantity(quantity), timestamp(timestamp) {};

    /**
     * @return execution_id
     */
    int get_execution_id() const { return execution_id; }


    /**
     * @return order_id
     */
    int get_order_id() const { return order_id; }


    /**
     * @return instrument_id
     */
    std::string get_instrument_id() const { return instrument_id; }


    /**
    * @return client_id
    */
    std::string get_client_id() const { return client_id; }


    /**
    * @return price
    */
    double get_price() const { return price; }


    /**
    * @return quantity
    */
    int get_quantity() const { return quantity; }


    /**
    * @return timestamp
    */
    long long get_timestamp() const { return timestamp; }

private:

    int execution_id;
    int order_id;
    std::string instrument_id;
    std::string client_id;
    double price;
    int quantity;
    long long timestamp;

};


