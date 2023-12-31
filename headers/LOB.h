//
// Created by Reza Tabrizi on 11/23/23.
//
#include <unordered_map>
#include <map>
#include <queue>
#include "Message.h"
#include <chrono>
#include "Limit.h"
#include "Execution.h"


#ifndef LIMITORDERBOOK_LOB_H
#define LIMITORDERBOOK_LOB_H

struct ActiveLOBMapAndTree{
    std::unordered_map<double, Limit*>* activeMapPtr;
    std::map<double, Limit*>* activeTreePtr;
};


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


    ActiveLOBMapAndTree GetActiveMapAndTree(bool bidOrAsk);
    long long GetTimeStamp ();
    void AddMarketOrder(Order* order);
    void AddLimitOrder(Order* order);
    void WriteOrderToDB(Order* order);
    void WriteExecuteToDB(Execution* execution);
    void RemoveLimitOrder(int orderId);
    void MatchMarketOrders(std::vector<Execution*> &executions);
    void MatchLimitOrders(std::vector<Execution*> &executions);
    static void WalkLimits(Order** topBidOrder, Order** topAskOrder);
    static void WalkOneLimit(Order** topOrder, Order* marketOrder);
    void WalkBook(Order** topBidOrder, Order** topAskOrder);
    void WalkOneBook(bool bidOrAsk, Order** topLimitOrder, Order* marketOrder);
    static Limit* FindNextLowestLimit(const std::map<double, Limit*>& tree, const double& bestPrice);
    static Limit* FindNextHighestLimit(const std::map<double, Limit*>& tree, const double& bestPrice);

public:
    LOB(): currentOrderId(0), currentExecutionId(0), bestBid(nullptr), bestAsk(nullptr){};
    Order* AddOrder(std::string instrumentId, std::string type, std::string clientId, bool bidOrAsk, int quantity, double limitPrice, long long entryTime, long long cancelTime);
    std::vector<Execution*> Execute();
    Order* CancelOrder(int orderId);
    [[nodiscard]] int GetBidVolumeAtLimit (double limit) const;
    [[nodiscard]] int GetAskVolumeAtLimit (double limit) const;
    [[nodiscard]] double GetBestBid () const;
    [[nodiscard]] double GetBestAsk () const;
    void PrintBidBook ();
    void PrintAskBook ();
};

#endif //LIMITORDERBOOK_LOB_H
