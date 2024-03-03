#include "headers/CentralMessageSystem.h"
#include "headers/OrderBookManager.h"
#include "headers/ExchangeServer.h"
#include "headers/exchange_common.h"
#include <thread>
#include <iostream>

bool register_instrument(std::string instrument)
{
    // add an instrument to the database
    return true; // Placeholder for actual implementation
}

bool register_mkt_ptc(std::string mkt_ptc)
{
    // add a market participant to the database
    return true; // Placeholder for actual implementation
}

int main() {
    // Create the central messaging system
    CentralMessageSystem cms;

    // Create the order book
    OrderBookManager obm(cms);

    // Create the server
    std::unordered_set<std::string> white_list = {"127.0.0.1"};
    ExchangeServer server(60000, cms, white_list);

    // Start the server in a separate thread
    std::thread serverThread([&server]() {
        server.Start();
        while (true) {
            server.Update(-1, true);
        }
    });

    std::cout << "Server is running. Press enter to quit...\n";
    std::cin.get();

    server.Stop(); // Ensure you have a Stop function to cleanly shutdown the server

    if (serverThread.joinable()) {
        serverThread.join();
    }

    return 0;
}
