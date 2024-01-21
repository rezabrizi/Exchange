#include <iostream>
#include "headers/CentralMessageSystem.h"
#include "headers/OrderBookManager.h"


int main() {

    CentralMessageSystem cms;
    OrderBookManager obm(cms);
    std::cout << "why" << std::endl;
    std::unique_ptr<BaseMessage> addOrderMessage1 = std::make_unique<AddOrderMessage>(
            cms.AssignMessageId(),
            "AddOrderMessage",
            1704902585457,
            "JS",
            "TSLA",
            true,
            100,
            50,
            "limit"
    );

    cms.Publish(std::move(addOrderMessage1));









}