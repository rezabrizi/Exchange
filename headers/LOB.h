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
    void AddLimitOrder(bool bidOrAsk, int quantity, double limitPrice, int entryTime);
    void RemoveLimitOrder(int orderId);
    void Execute();
    int GetBidVolumeAtLimit (double limit) const;
    int GetAskVolumeAtLimit (double limit) const;
    double GetBestBid () const;
    double GetBestAsk () const;
    void PrintBidBook ();
    void PrintAskBook ();
    bool VolumeValidation (int Volume);
    void PriceValidation (double limitPrice, double marketPrice);



};

#endif //LIMITORDERBOOK_LOB_H
