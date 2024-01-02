#include <iostream>
#include "headers/LOB.h"


int main() {
    LOB TSLA;
    TSLA.AddOrder("TSLA", "limit", "JS", true, 100, 95, 10);
    TSLA.AddOrder("TSLA", "limit", "GS", true, 100, 95, 11);
    TSLA.AddOrder("TSLA", "limit", "FID", true, 100, 95, 12);
    TSLA.AddOrder("TSLA", "limit", "GS", true, 80, 100, 13);
    TSLA.AddOrder("TSLA", "limit", "FID", true, 100, 100, 14);
    TSLA.AddOrder("TSLA", "limit", "JS", true, 100, 95, 15);


    TSLA.AddOrder("TSLA", "limit", "GS", false, 100, 105, 10);
    TSLA.AddOrder("TSLA", "limit", "GS", false, 100, 103, 11);
    TSLA.AddOrder("TSLA", "limit", "FID", false, 100, 103, 12);
    TSLA.AddOrder("TSLA", "limit", "JS", false, 100, 100, 13);
    TSLA.AddOrder("TSLA", "limit", "FID", false, 100, 98, 14);
    TSLA.AddOrder("TSLA", "limit", "JS", false, 100, 95, 15);



    std::vector<Execution*> executions = TSLA.Execute(true);

    for (const auto& exec: executions){
        std::cout << "Exec ID: " << exec->GetExecutionId() << "   Order ID: " << exec->GetOrderId() << "   Instrument ID: " << exec->GetInstrumentId() <<
        "   Client ID: " << exec->GetClientId() << "   Price: $" << exec->GetPrice() << "    shares: " << exec->GetQuantity() <<  "   timestamp: " << exec->GetTimestamp() << std::endl;
    }
    std::cout << std::endl;


    TSLA.AddOrder("TSLA", "market", "JS", true, 50, -1, 16);
    std::vector<Execution*> executionsMarket = TSLA.Execute(false);

    for (const auto& exec: executionsMarket){
        std::cout << "Exec ID: " << exec->GetExecutionId() << "   Order ID: " << exec->GetOrderId() << "   Instrument ID: " << exec->GetInstrumentId() <<
                  "   Client ID: " << exec->GetClientId() << "   Price: $" << exec->GetPrice() << "    shares: " << exec->GetQuantity() <<  "   timestamp: " << exec->GetTimestamp() << std::endl;
    }
    std::cout << std::endl;

    TSLA.PrintBidBook();
    TSLA.PrintAskBook();
}
