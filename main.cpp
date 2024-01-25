#include <iostream>
#include "headers/CentralMessageSystem.h"
#include "headers/OrderBookManager.h"
#include <thread>

int main() {
    CentralMessageSystem cms;
    OrderBookManager obm(cms);


    std::unique_ptr<BaseMessage> addOrderMessage1 = std::make_unique<AddOrderMessage>(
            cms.AssignMessageId(),
            "AddOrderMessage",
            1704902585401,
            "JS",
            "TSLA",
            true,
            100,
            50,
            "limit"
    );
    std::unique_ptr<BaseMessage> addOrderMessage2 = std::make_unique<AddOrderMessage>(
            cms.AssignMessageId(),
            "AddOrderMessage",
            1704902585402,
            "JS",
            "AAPL",
            true,
            100,
            50,
            "limit"
    );
    std::unique_ptr<BaseMessage> addOrderMessage3 = std::make_unique<AddOrderMessage>(
            cms.AssignMessageId(),
            "AddOrderMessage",
            1704902585402,
            "JS",
            "AAPL",
            true,
            100,
            50,
            "limit"
    );
    std::unique_ptr<BaseMessage> addOrderMessage4 = std::make_unique<AddOrderMessage>(
            cms.AssignMessageId(),
            "AddOrderMessage",
            1704902585402,
            "JS",
            "AAPL",
            true,
            100,
            50,
            "limit"
    );



    cms.Publish(std::move(addOrderMessage1));
    cms.Publish(std::move(addOrderMessage2));
    cms.Publish(std::move(addOrderMessage3));
    cms.Publish(std::move(addOrderMessage4));
    cms.Shutdown();
}