#pragma once

#include "Order.h"

class Limit {
public:
    /**
     * @brief constructor of a limit linked list
     * @param limitPrice price of the limit
     */
    Limit(double limitPrice) : limit_price(limitPrice), size(0), volume(0), head_order(nullptr), tail_order(nullptr) {};


    /**
     * @brief adding a new order to the linked list
     * @param order order to add
     */
    void add_order(Order *order);


    /**
     * @brief removing an order from the linked list
     * @param order order to remove
     */
    void remove_order(Order *order);


    /**
     * @brief reducing quantity of an order from the linked list
     * @param order order to reduce from
     * @param reduceQuantity quantity to reduce
     */
    void reduce_order(Order *order, int reduceQuantity);


    /**
     * @brief get price of limit
     * @return limit_price
     */
    double get_limit_price() const;


    /**
     * @brief get size of limit
     * @return size
     */
    int get_limit_size() const;


    /**
     * @brief get volume of limit
     * @return volume
     */
    int get_limit_volume() const;


    /**
     * @brief get the first order of the limit
     * @return head_order
     */
    Order *get_top_order() const;

private:
    double limit_price;
    int size;
    int volume;
    Order *head_order;
    Order *tail_order;
};

