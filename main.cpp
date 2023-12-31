#include <iostream>
#include "headers/LOB.h"

int main() {
    LOB TSLA;
    TSLA.AddOrder("TSLA", "JS", true, 20, 97, 1);
    TSLA.AddOrder("TSLA", "JS", true, 50, 100, 2);
    TSLA.AddOrder("TSLA", "JS", true, 40, 99, 3);
    TSLA.AddOrder("TSLA", "JS", true, 60, 100, 5);
    TSLA.AddOrder("TSLA", "JS", true, 80, 100, 4);


    TSLA.AddOrder("TSLA", "JS", false, 90, 99, 3);
    TSLA.AddOrder("TSLA", "JS", false, 40, 98, 5);
    TSLA.AddOrder("TSLA", "JS", false, 30, 100, 7);
    TSLA.AddOrder("TSLA", "JS", false, 60, 99, 6);
    TSLA.AddOrder("TSLA", "JS", false, 80, 99, 8);



    TSLA.PrintBidBook();
    std::cout << std::endl;
    TSLA.PrintAskBook();

    TSLA.MatchLimitOrders();

    std::cout << std::endl;
    TSLA.PrintBidBook();
    std::cout << std::endl;
    TSLA.PrintAskBook();

}
