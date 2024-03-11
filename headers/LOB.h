#pragma once
#include "exchange_common.h"
#include "DBConnection.h"
#include "Message.h"
#include "Limit.h"
#include "Execution.h"


/**
 * @file LOB.h
 * @brief singular limit order book for one financial instrument
 */

/**
 * @struct ActiveLobMapAndTree
 * @brief provide pointers to the active binary tree and hashmap for bid or ask orders
 */
struct ActiveLOBMapAndTree{
    std::unordered_map<double, Limit*>* active_map_ptr;
    std::map<double, Limit*>* active_tree_ptr;
};

/**
 * @class LOB
 * @brief limit order book class for one financial instrument
 */
class LOB{

public:

    /**
     * @brief LOB constructor
     */
    LOB(): current_order_id(0), current_execution_id(0), best_bid(nullptr), best_ask(nullptr){};

    int get_current_execution_id ()
    {
        return current_execution_id;
    }
    /**
     *
     * @param instrument_id
     * @param type
     * @param client_id
     * @param bid_ask
     * @param quantity
     * @param limit_price
     * @param entry_time
     * @return Order Object
     */
    Order* add_order(std::string instrument_id, std::string type, std::string client_id, bool bid_ask, int quantity, double limit_price, long long entry_time, int order_id = -1);

    /**
     * @brief execute all possible matches
     * @param is_limit whether the most recent order was a limit order or not
     * @return list of executions
     */
    std::vector<Execution*> execute(bool is_limit);

    /**
     * @brief current time with millisecond precision
     * @return current time (long long)
     */
    static long long get_time_stamp();

    /**
     * @brief cancelling an individual order
     * @param order_id the id of the order to cancel
     * @return the order object that was cancelled
     */
    Order* cancel_order(int order_id, const std::string& client_id, long long cancel_time);

    /**
     *
     * @param order_id
     */
    void update_current_order_id(int order_id);

    /**
     * @brief update the current execution id
     * @param execution_id the existing execution id
     */
    void update_current_execution_id(int execution_id);


    /**
     * @brief get the bid volume at a limit price
     * @param limit
     * @return volume
     */
    [[nodiscard]] int get_bid_volume_at_limit (double limit) const;

    /**
     * @brief get the ask volume at a limit price
     * @param limit
     * @return volume
     */
    [[nodiscard]] int get_ask_volume_at_limit (double limit) const;


    void print_bid_book ();
    void print_ask_book ();

private:


    /**
     * @brief provide the respective binary tree and hashmap for limit orders
     * @param bid_ask
     * @return active tree and hashmap (ActiveLOBMapAndTree)
     */
    ActiveLOBMapAndTree get_active_hash_map_tree(bool bid_ask);

    /**
     * @brief add a market order to the book
     * @param order order to add
     */
    void add_market_order(Order* order);

    /**
     * @brief add a limit order to the book
     * @param order order to add
     */
    void add_limit_order(Order* order);

    /**
     * @brief remove a limit order from the book
     * @param order_id the id of the order to remove
     */
    void remove_limit_order(int order_id);

    /**
     * @brief match market orders
     * @param executions new executions after the matches are made
     */
    void match_market_orders(std::vector<Execution*> &executions);

    /**
     * @brief match limit orders
     * @param executions new executions after the matches are made
     */
    void match_limit_orders(std::vector<Execution*> &executions);

    /**
     * @brief walk 2 limit linked lists to find appropriate matches
     * @param top_bid_order the top appropriate bid
     * @param top_ask_order the top appropriate ask
     */
    static void walk_limits(Order** top_bid_order, Order** top_ask_order);

    /**
     * @brief walk 1 limit linked list to find appropriate match with a market order
     * @param top_order the top appropriate limit order
     * @param market_order the market order to match with the appropriate limit order
     */
    static void walk_one_limit(Order** top_order, Order* market_order);

    /**
     * @brief walk the limit order book to find the appropriate limit price
     * @param top_bid_order the top appropriate bid
     * @param top_ask_order the top appropriate ask
     */
    void walk_book(Order** top_bid_order, Order** top_ask_order);

    /**
     * @brief walk one side (bid or ask) of the book to find the appropriate limit price
     * @param bid_ask the side of the book to walk
     * @param top_limit_order the top appropriate limit order
     * @param market_order the market order to match with the appropriate limit order
     */
    void walk_one_book(bool bid_ask, Order** top_limit_order, Order* market_order);

    /**
     * @param tree the limit tree
     * @param best_price current best price
     * @return next lower limit (next higher price)
     */
    static Limit* find_next_lowest_limit(const std::map<double, Limit*>& tree, const double& best_price);

    /**
     * @param tree the limit tree
     * @param best_price current best price
     * @return next highest limit (next lower price)
     */
    static Limit* find_next_highest_limit(const std::map<double, Limit*>& tree, const double& best_price);

    int current_order_id;
    int current_execution_id;
    Limit* best_bid;
    Limit* best_ask;
    std::queue <Order*> mkt_order_bid_q;
    std::queue <Order*> mkt_order_ask_q;
    std::unordered_map <int, Order*> orders;
    std::unordered_map <double, Limit*> bid_map;
    std::unordered_map <double, Limit*> ask_map;
    std::map <double, Limit*> bid_tree;
    std::map <double, Limit*> ask_tree;

};
