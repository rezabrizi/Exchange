#include "headers/CentralMessageSystem.h"
#include "headers/OrderBookManager.h"
#include "headers/ExchangeServer.h"
#include "headers/exchange_common.h"
#include <thread>
#include <iostream>



DBConnection& setup_db (const std::string& db_name, const std::string& user, const std::string& pass, const std::string& host, const std::string& port)
{
    std::string connectionString = "dbname=" + db_name + " user=" + user + " password=" + pass + " host=" + host + " port=" + port;
    return DBConnection::getInstance(connectionString);
}

DBConnection& DB = setup_db("exchange", "rezatabrizi", "1123", "localhost", "5432");


bool register_instrument(const std::string& instrument, const std::string& name, const std::string& type, DBConnection& DB) {
    try {
        // Get the current system time as UNIX timestamp for ListingDate
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        // Create the SQL query
        std::string sql = "INSERT INTO Instruments (ID, Name, Type, ListingDate) VALUES ('" +
                          instrument + "', '" + name + "', '" + type + "', " + std::to_string(millis) + ");";

        // execute the query
        pqxx::result r = DB.query(sql);

        std::cout << "Instrument registered successfully: " << instrument << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return false;
    }
}


bool register_mkt_ptc(const std::string& mkt_ptc, const std::string& id, const std::string& ip, const std::string& pass, DBConnection& DB) {
    try {
        // Create the SQL query
        std::string sql = "INSERT INTO Clients (ID, Name, TCPIPAddress, Passkey) VALUES ('" +
                          id + "', '" + mkt_ptc + "', '" + ip + "', '" + pass + "');";

        // execute the query
        pqxx::result r = DB.query(sql);

        std::cout << "Market participant registered successfully: " << mkt_ptc << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return false;
    }
}


void send_alert(CentralMessageSystem &cms, const std::string& instrument_id, const std::string& alert, std::string& description)
{
    std::unique_ptr<BaseMessage> system_message = std::make_unique<SystemMessage>(
            cms.assign_messaging_id(),
            "AlertMessage",
            cms.GetCurrentTimeStamp(),
            instrument_id,
            alert,
            description
            );
    cms.publish(std::move(system_message));
}


void show_menu() {
    std::cout << "0. Set up database connection\n";
    std::cout << "1. Register new market participant\n";
    std::cout << "2. Register new instrument\n";
    std::cout << "3. Start the server\n";
    std::cout << "4. Send an alert\n"; // New option for sending alerts
    std::cout << "Enter your choice (1-4): ";
}


int main() {
    std::string input;
    bool server_started = false;
    bool db_set_up = false;
    DBConnection* DBPtr = nullptr; // Use a pointer to delay the construction

    CentralMessageSystem cms;
    std::unique_ptr<OrderBookManager> obm;
    std::unique_ptr<ExchangeServer> server; // Use a unique_ptr to delay the instantiation
    std::thread serverThread; // Declare the thread but don't start it immediately

    while (true) {
        show_menu();
        std::getline(std::cin, input);
        int choice = std::stoi(input);

        switch (choice) {
            case 0:
                if (!db_set_up) {
                    std::string db_name, user, pass, host, port;
                    std::cout << "Enter database name: ";
                    std::getline(std::cin, db_name);
                    std::cout << "Enter user: ";
                    std::getline(std::cin, user);
                    std::cout << "Enter password: ";
                    std::getline(std::cin, pass);
                    std::cout << "Enter host: ";
                    std::getline(std::cin, host);
                    std::cout << "Enter port: ";
                    std::getline(std::cin, port);
                    DBPtr = &setup_db(db_name, user, pass, host, port);
                    db_set_up = true;
                    std::cout << "Database connection setup complete.\n";
                } else {
                    std::cout << "Database connection has already been set up.\n";
                }
                break;


            case 1:
                if (!server_started && db_set_up) {
                    std::string name, id, ip, pass;
                    std::cout << "Enter market participant name: ";
                    std::getline(std::cin, name);
                    std::cout << "Enter market participant ID: ";
                    std::getline(std::cin, id);
                    std::cout << "Enter market participant IP: ";
                    std::getline(std::cin, ip);
                    std::cout << "Enter market participant Passkey: ";
                    std::getline(std::cin, pass);
                    register_mkt_ptc(name, id, ip, pass, *DBPtr);
                } else {
                    std::cout << "Operation not allowed while the server is running.\n";
                }
                break;
            case 2:
                if (!server_started && db_set_up) {
                    std::string instrument, name, type;
                    std::cout << "Enter instrument ID: ";
                    std::getline(std::cin, instrument);
                    std::cout << "Enter instrument Name: ";
                    std::getline(std::cin, name);
                    std::cout << "Enter instrument Type: ";
                    std::getline(std::cin, type);
                    register_instrument(instrument, name, type, *DBPtr);
                } else {
                    std::cout << "Operation not allowed while the server is running.\n";
                }
                break;
            case 3:
                if (!server_started && db_set_up) {
                    // Start the server in a separate thread if not already started
                    obm = std::make_unique<OrderBookManager>(cms, *DBPtr);
                    server = std::make_unique<ExchangeServer>(60000, cms, *DBPtr);
                    server_started = true;
                    serverThread = std::thread([&server]() {
                        server->Start();
                        while (true) {
                            server->Update(-1, true);
                        }
                    });
                    std::cout << "Server is running. Operations 1 and 2 are now disabled.\n";
                }
                break;
            case 4:
                if (server_started) {
                    std::string instrument_id, alert, description;
                    std::cout << "Enter Instrument ID: ";
                    std::getline(std::cin, instrument_id);
                    std::cout << "Enter Alert: ";
                    std::getline(std::cin, alert);
                    std::cout << "Enter Description: ";
                    std::getline(std::cin, description);
                    send_alert(cms, instrument_id, alert, description);
                } else {
                    std::cout << "Please start the server first (Option 3).\n";
                }
                break;
            case 5:
                if (server_started) {
                    server->Stop();
                    if (serverThread.joinable()) {
                        serverThread.join();
                    }
                    std::cout << "Server stopped.\n";
                }
                return 0; // Exit the application
            default:
                std::cout << "Invalid choice. Please enter a number between 1 and 5.\n";
        }
    }
}