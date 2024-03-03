#include "net.h"
#include <iostream>
#include <string>
#include <utility>
#include <limits>

enum class ExchangeMessages : uint32_t {
    Connect,
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
        msg.header.id = ExchangeMessages::Connect;
        msg << client_name;
        Send(msg);
    }

    void send_order(const std::string& instrument_id, bool bid_or_ask, double price, int quantity, const std::string& order_type) {
        net::message<ExchangeMessages> msg;
        msg.header.id = ExchangeMessages::AddOrder;
        msg << order_type << quantity << price << bid_or_ask << instrument_id;
        Send(msg);
    }

private:
    std::string client_name;
};

void send_add_order(ClientTest& ct) {
    std::string instrument_id, order_type;
    bool bid_or_ask;
    double price;
    int quantity;

    std::cout << "Instrument ID: ";
    std::getline(std::cin, instrument_id);

    std::cout << "Bid or Ask (0 for bid, 1 for ask): ";
    std::cin >> bid_or_ask;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the newline character from the stream

    std::cout << "Price: ";
    std::cin >> price;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the newline character from the stream

    std::cout << "Quantity: ";
    std::cin >> quantity;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the newline character from the stream

    std::cout << "Order Type: ";
    std::getline(std::cin, order_type);

    ct.send_order(instrument_id, bid_or_ask, price, quantity, order_type);
}

int main() {
    std::string client_name;
    std::cout << "Enter client name: ";
    std::getline(std::cin, client_name);

    ClientTest ct(client_name);
    if (!ct.Connect("127.0.0.1", 60000)) {
        std::cout << "Failed to connect to server.\n";
        return 1;
    }

    ct.register_company(); // Register upon connecting

    bool bQuit = false;
    while (!bQuit) {
        if (ct.IsConnected()) {
            while (!ct.Incoming().empty()) {
                auto msg = ct.Incoming().pop_front().msg;

                switch (msg.header.id) {
                    case ExchangeMessages::OrderConfirmation: {
                        std::string client_id, instrument_id, order_type;
                        int order_id;
                        long long cancel_time;

                        msg >> client_id >> instrument_id >> order_id >> cancel_time;
                        std::cout << "Order Confirmation - Client ID: " << client_id << ", Instrument ID: " << instrument_id
                                  << ", Order ID: " << order_id << ", Cancel Time: " << cancel_time << std::endl;
                        break;
                    }
                    case ExchangeMessages::Execution: {
                        std::string client_id, instrument_id;
                        int order_id, quantity;
                        double price;
                        long long executionTime;

                        msg >> client_id >> instrument_id >> order_id >> price >> quantity >> executionTime;
                        std::cout << "Execution - Client ID: " << client_id << ", Instrument ID: " << instrument_id
                                  << ", Order ID: " << order_id << ", Price: " << price << ", Quantity: " << quantity
                                  << ", Execution Time: " << executionTime << std::endl;
                        break;
                    }
                        // Handle other cases as needed
                }
            }
        } else {
            std::cout << "Server Down\n";
            bQuit = true;
        }

        std::string cmd;
        std::cout << "Enter command (add, quit): ";
        std::getline(std::cin, cmd);

        if (cmd == "add") {
            send_add_order(ct);
        } else if (cmd == "quit") {
            bQuit = true;
        }
    }

    return 0;
}
