#include <iostream>
#include "headers/LOB.h"

int main() {
    LOB TSLA;
    TSLA.AddLimitOrder(true, 20, 97, 1234);
    TSLA.AddLimitOrder(true, 50, 100, 1234);
    TSLA.AddLimitOrder(true, 40, 99, 1234);
    TSLA.AddLimitOrder(true, 60, 100, 1234);
    TSLA.AddLimitOrder(true, 80, 100, 1234);


    TSLA.AddLimitOrder(false, 90, 99, 1234);
    TSLA.AddLimitOrder(false, 40, 98, 1234);
    TSLA.AddLimitOrder(false, 30, 100, 1234);
    TSLA.AddLimitOrder(false, 60, 99, 1234);
    TSLA.AddLimitOrder(false, 80, 99, 1234);



    TSLA.PrintBidBook();
    std::cout << std::endl;
    TSLA.PrintAskBook();

    std::cout << std::endl;


    TSLA.Execute();

    std::cout << std::endl;
    TSLA.PrintBidBook();
    std::cout << std::endl;
    TSLA.PrintAskBook();

}
