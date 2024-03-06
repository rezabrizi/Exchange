#include "headers/CentralMessageSystem.h"
#include "headers/OrderBookManager.h"
#include "headers/ExchangeServer.h"
#include "headers/exchange_common.h"
#include <thread>
#include <iostream>

DBConnection& DB = DBConnection::getInstance("dbname=exchange user=rezatabrizi password=1123 host=localhost port=5432");

bool register_instrument(const std::string& instrument, const std::string& name, const std::string& type) {
    try {
        // Get the current system time as UNIX timestamp for ListingDate
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        // Create the SQL query
        std::string sql = "INSERT INTO Instruments (ID, Name, Type, ListingDate) VALUES ('" +
                          instrument + "', '" + name + "', '" + type + "', " + std::to_string(millis) + ");";

        // Execute the query
        pqxx::result r = DB.query(sql);

        std::cout << "Instrument registered successfully: " << instrument << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return false;
    }
}

bool register_mkt_ptc(const std::string& mkt_ptc, const std::string& id, const std::string& ip, const std::string& pass) {
    try {
        // Create the SQL query
        std::string sql = "INSERT INTO Clients (ID, Name, TCPIPAddress, Passkey) VALUES ('" +
                          id + "', '" + mkt_ptc + "', '" + ip + "', '" + pass + "');";

        // Execute the query
        pqxx::result r = DB.query(sql);

        std::cout << "Market participant registered successfully: " << mkt_ptc << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return false;
    }
}

void show_menu() {
    std::cout << "1. Register new market participant\n";
    std::cout << "2. Register new instrument\n";
    std::cout << "3. Start the server\n";
    std::cout << "Enter your choice (1-3): ";
}

int main() {
    std::string input;
    bool server_started = false;

    while (!server_started) {
        show_menu();
        std::getline(std::cin, input);
        int choice = std::stoi(input);

        switch (choice) {
            case 1: {
                std::string name, id, ip, pass;
                std::cout << "Enter market participant name: ";
                std::getline(std::cin, name);
                std::cout << "Enter market participant ID: ";
                std::getline(std::cin, id);
                std::cout << "Enter market participant IP: ";
                std::getline(std::cin, ip);
                std::cout << "Enter market participant Passkey: ";
                std::getline(std::cin, pass);
                register_mkt_ptc(name, id, ip, pass);
                break;
            }
            case 2: {
                std::string instrument, name, type;
                std::cout << "Enter instrument ID: ";
                std::getline(std::cin, instrument);
                std::cout << "Enter instrument Name: ";
                std::getline(std::cin, name);
                std::cout << "Enter instrument Type: ";
                std::getline(std::cin, type);
                register_instrument(instrument, name, type);
                break;
            }
            case 3:
                server_started = true;
                break;
            default:
                std::cout << "Invalid choice. Please enter a number between 1 and 3.\n";
        }
    }

    // Server initialization code follows...
    CentralMessageSystem cms;
    OrderBookManager obm(cms);
    ExchangeServer server(60000, cms);

    // Start the server in a separate thread
    std::thread serverThread([&server]() {
        server.Start();
        while (true) {
            server.Update(-1, true);
        }
    });

    std::cout << "Server is running. Press enter to quit...\n";
    std::cin.get();

    server.Stop();

    if (serverThread.joinable()) {
        serverThread.join();
    }

    return 0;
}