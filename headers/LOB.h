//
// Created by Reza Tabrizi on 11/23/23.
//
#include <unordered_map>
#include <map>
#include "Limit.h"
#include "LOBLogger.h"

#ifndef LIMITORDERBOOK_LOB_H
#define LIMITORDERBOOK_LOB_H


class LOB{
    int curr_id;
    Limit* bestBid;
    Limit* bestAsk;
    std::unordered_map <double, Limit*> buyMap;
    std::unordered_map <double, Limit*> sellMap;
    std::unordered_map <int, Order*> orders;
    std::map <double, Limit*> buyTree;
    std::map <double, Limit*> sellTree;

    LOBLogger logger;

public:
    LOB(): curr_id(0), bestBid(nullptr), bestAsk(nullptr){};
    void AddLimitOrder(std::string instrument, std::string owner, bool bidOrAsk, int quantity, double limitPrice, int entryTime);
    void RemoveLimitOrder(int orderId);
    void Execute();
    static void WalkLimits(Order** topBidOrder, Order** topAskOrder);
    void WalkBook(Order** topBidOrder, Order** topAskOrder);
    static Limit* FindNextLowestLimit(const std::map<double, Limit*>& tree, const double& bestPrice);
    static Limit* FindNextHighestLimit(const std::map<double, Limit*>& tree, const double& bestPrice);
    [[nodiscard]] int GetBidVolumeAtLimit (double limit) const;
    [[nodiscard]] int GetAskVolumeAtLimit (double limit) const;
    [[nodiscard]] double GetBestBid () const;
    [[nodiscard]] double GetBestAsk () const;
    void PrintBidBook ();
    void PrintAskBook ();
    bool VolumeValidation (int Volume);
    void PriceValidation (double limitPrice, double marketPrice);
};

#endif //LIMITORDERBOOK_LOB_H
