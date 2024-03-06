#include "net.h"
#include <iostream>
#include <string>
#include <utility>
#include <limits>

enum class ExchangeMessages : uint32_t {
    Authenticate,
    AddOrder,
    CancelOrder,
    Execution,
    OrderConfirmation,
    AlertMessage
};

class ClientTest : public net::client_interface<ExchangeMessages> {
public:
    explicit ClientTest(std::string client_name) : client_name(std::move(client_name)) {}

    void register_company() {
        net::message<ExchangeMessages> msg;
        msg.header.id = ExchangeMessages::Authenticate;
        msg << client_name;
        Send(msg);
    }

    void send_order(const std::string& instrument_id, bool bid_or_ask, double price, int quantity, const std::string& order_type) {
        net::message<ExchangeMessages> msg;
        msg.header.id = ExchangeMessages::AddOrder;
        msg << quantity << order_type << bid_or_ask << instrument_id;
        if (price != -1)
        {
            msg << price;
        }
        Send(msg);
    }

private:
    std::string client_name;
};

void send_add_order(ClientTest& ct) {
    std::string instrument_id, order_type;
    int bid_or_ask_input;
    bool bid_or_ask;
    double price = -1; // Default to -1 for limit orders
    int quantity;

    std::cout << "Instrument ID: ";
    std::getline(std::cin, instrument_id);

    std::cout << "Order Type (limit/market): ";
    while (true) {
        std::getline(std::cin, order_type);
        if (order_type != "limit" && order_type != "market") {
            std::cout << "Invalid input. Please enter either 'limit' or 'market' for order type.\n";
        } else {
            break; // Valid input
        }
    }

    std::cout << "Bid or Ask (1 for bid, 0 for ask): ";
    while (!(std::cin >> bid_or_ask_input) || (bid_or_ask_input != 1 && bid_or_ask_input != 0)) {
        std::cout << "Invalid input. Please enter 1 for bid or 0 for ask.\n";
        std::cin.clear(); // Clear error flags
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard the input
    }
    bid_or_ask = static_cast<bool>(bid_or_ask_input);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the newline character from the stream

    if (order_type == "limit") {
        std::cout << "Price: ";
        while (!(std::cin >> price) || price <= 0) {
            std::cout << "Invalid input. Please enter a positive number for price.\n";
            std::cin.clear(); // Clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard the input
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the newline character from the stream
    }

    std::cout << "Quantity: ";
    while (!(std::cin >> quantity) || quantity <= 0) {
        std::cout << "Invalid input. Please enter a positive integer for quantity.\n";
        std::cin.clear(); // Clear error flags
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard the input
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the newline character from the stream

    ct.send_order(instrument_id, bid_or_ask, price, quantity, order_type);
}

int main() {
    std::string client_name;
    std::cout << "Enter client name: ";
    std::getline(std::cin, client_name);

    ClientTest ct(client_name);
    if (!ct.Connect("127.0.0.1", 60000))
    {
        std::cout << "Failed to connect to server.\n";
        return 1;
    }

    ct.register_company();

    bool bQuit = false;
    while (!bQuit) {
        if (ct.IsConnected()) {
            while (!ct.Incoming().empty()) {
                auto msg = ct.Incoming().pop_front().msg;
                std::cout << "there is a message \n";
                switch (msg.header.id) {
                    case ExchangeMessages::OrderConfirmation: {
                        std::string client_id, instrument_id, order_type;
                        int order_id;
                        long long cancel_time;

                        msg >> cancel_time >> order_id >> instrument_id >> client_id;
                        std::cout << "Order Confirmation - Client ID: " << client_id << ", Instrument ID: " << instrument_id
                                  << ", Order ID: " << order_id << ", Cancel Time: " << cancel_time << std::endl;
                        break;
                    }
                    case ExchangeMessages::Execution: {
                        std::string client_id, instrument_id;
                        int order_id, quantity;
                        double price;
                        long long executionTime;

                        msg >> executionTime >> quantity >> price >> order_id >> instrument_id >> client_name;
                        std::cout << "Execution - Client ID: " << client_id << ", Instrument ID: " << instrument_id
                                  << ", Order ID: " << order_id << ", Price: " << price << ", Quantity: " << quantity
                                  << ", Execution Time: " << executionTime << std::endl;
                        break;
                    }
                }
            }

        } else {
            std::cout << "Server Down\n";
            bQuit = true;
        }

        std::string cmd;
        std::cout << "Enter command (add, quit): ";
        std::getline(std::cin, cmd);

        if (cmd == "add")
        {
            send_add_order(ct);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else if (cmd == "quit")
        {
            bQuit = true;
        }
    }

    return 0;
}
