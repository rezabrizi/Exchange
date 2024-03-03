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
    std::unordered_map<double, Limit*>* activeMapPtr;
    std::map<double, Limit*>* activeTreePtr;
};

/**
 * @class LOB
 * @brief limit order book class for one financial instrument
 */
class LOB{
    int currentOrderId;
    int currentExecutionId;
    Limit* bestBid;
    Limit* bestAsk;
    std::queue <Order*> marketOrderBuyQueue;
    std::queue <Order*> marketOrderSellQueue;
    std::unordered_map <int, Order*> orders;
    std::unordered_map <double, Limit*> buyMap;
    std::unordered_map <double, Limit*> sellMap;
    std::map <double, Limit*> buyTree;
    std::map <double, Limit*> sellTree;

    /**
     * @brief provide the respective binary tree and hashmap for limit orders
     * @param bidOrAsk
     * @return active tree and hashmap (ActiveLOBMapAndTree)
     */
    ActiveLOBMapAndTree GetActiveMapAndTree(bool bidOrAsk);

    /**
     * @brief add a market order to the book
     * @param order order to add
     */
    void AddMarketOrder(Order* order);

    /**
     * @brief add a limit order to the book
     * @param order order to add
     */
    void AddLimitOrder(Order* order);

    /**
     * @brief write a new order to the database
     * @param order order to save to the database
     */
    void WriteOrderToDB(const Order* order);

    /**
     * @brief write a new execution to the database
     * @param execution executions to save to the database
     */
    void WriteExecutionToDB(const Execution* execution);


    void CancelOrderInDB(const std::string& instrumentId, int orderId, long long cancelTime);

    /**
     * @brief remove a limit order from the book
     * @param orderId the id of the order to remove
     */
    void RemoveLimitOrder(int orderId);

    /**
     * @brief match market orders
     * @param executions new executions after the matches are made
     */
    void MatchMarketOrders(std::vector<Execution*> &executions);

    /**
     * @brief match limit orders
     * @param executions new executions after the matches are made
     */
    void MatchLimitOrders(std::vector<Execution*> &executions);

    /**
     * @brief walk 2 limit linked lists to find appropriate matches
     * @param topBidOrder the top appropriate bid
     * @param topAskOrder the top appropriate ask
     */
    static void WalkLimits(Order** topBidOrder, Order** topAskOrder);

    /**
     * @brief walk 1 limit linked list to find appropriate match with a market order
     * @param topOrder the top appropriate limit order
     * @param marketOrder the market order to match with the appropriate limit order
     */
    static void WalkOneLimit(Order** topOrder, Order* marketOrder);

    /**
     * @brief walk the limit order book to find the appropriate limit price
     * @param topBidOrder the top appropriate bid
     * @param topAskOrder the top appropriate ask
     */
    void WalkBook(Order** topBidOrder, Order** topAskOrder);

    /**
     * @brief walk one side (bid or ask) of the book to find the appropriate limit price
     * @param bidOrAsk the side of the book to walk
     * @param topLimitOrder the top appropriate limit order
     * @param marketOrder the market order to match with the appropriate limit order
     */
    void WalkOneBook(bool bidOrAsk, Order** topLimitOrder, Order* marketOrder);

    /**
     * @param tree the limit tree
     * @param bestPrice current best price
     * @return next lower limit (next higher price)
     */
    static Limit* FindNextLowestLimit(const std::map<double, Limit*>& tree, const double& bestPrice);

    /**
     * @param tree the limit tree
     * @param bestPrice current best price
     * @return next highest limit (next lower price)
     */
    static Limit* FindNextHighestLimit(const std::map<double, Limit*>& tree, const double& bestPrice);


public:

    /**
     * @brief LOB constructor
     */
    LOB(): currentOrderId(0), currentExecutionId(0), bestBid(nullptr), bestAsk(nullptr){};

    /**
     *
     * @param instrumentId
     * @param type
     * @param clientId
     * @param bidOrAsk
     * @param quantity
     * @param limitPrice
     * @param entryTime
     * @return Order Object
     */
    Order* AddOrder(std::string instrumentId, std::string type, std::string clientId, bool bidOrAsk, int quantity, double limitPrice, long long entryTime, int orderId = -1);

    /**
     * @brief execute all possible matches
     * @param isLimit whether the most recent order was a limit order or not
     * @return list of executions
     */
    std::vector<Execution*> Execute(bool isLimit);

    /**
     * @brief current time with millisecond precision
     * @return current time (long long)
     */
    static long long GetTimeStamp ();

    /**
     * @brief cancelling an individual order
     * @param orderId the id of the order to cancel
     * @return the order object that was cancelled
     */
    Order* CancelOrder(int orderId, long long cancelTime);


    void UpdateCurrentOrderId(int orderId);


    /**
     * @brief get the bid volume at a limit price
     * @param limit
     * @return volume
     */
    [[nodiscard]] int GetBidVolumeAtLimit (double limit) const;

    /**
     * @brief get the ask volume at a limit price
     * @param limit
     * @return volume
     */
    [[nodiscard]] int GetAskVolumeAtLimit (double limit) const;

    /**
     * @return best bid
     */
    [[nodiscard]] double GetBestBid () const;

    /**
     * @return best ask
     */
    [[nodiscard]] double GetBestAsk () const;


    void PrintBidBook ();
    void PrintAskBook ();
};
