# Electronic Exchange
This is a program to demonstrate key components of an electronic exchange via `C++` and `ASIO`.

1. **Limit Order Book**: The record of orders in an order-driven market. Security prices are determined based on the bids and asks for a given security.
2. **Exchange Server**: The server where registered clients can connect via `TCP` and send their orders.
   1. The clients send orders or cancellations.
   2. The server sends order confirmations, trade executions, and alert messages.
3. **Database Connection**: Although the order book stays in memory of the server during trading hours, all orders and executions are stored in the database. This is crucial for disaster recovery and regulatory documentation.

## How to Use

1. Install PostgreSQL.
2. Use the SQL script located in `Exchange/DB/exchange.sql` to set up the database on your local system.
3. Install `pqxx`.
   1. Edit the `CMakeLists.txt` to correctly link the `pqxx` library to the program.
4. Install ASIO.
   1. Edit the `CMakeLists.txt` to correctly link the ASIO library to the program.
   2. Ensure you have `ASIO_STANDALONE` defined to avoid using the Boost version.
5. Clone the [Net Repository](https://github.com/rezabrizi/Net).
   1. Edit the `CMakeLists.txt` to correctly link the Net library to the program.
6. Run the Server application: `LimitOrderBook`.
7. Set up the database using `option 0`.
8. Ensure you have clients registered and instruments listed.
   1. Set the client IP as `127.0.0.1` since all connections will be local.
9. Start the server using `option 3`.
10. Run the client application: `Client`.
11. You should now be able to connect with registered client names to the server and send in your orders.

## Order Book

1. `LOB.h`: Maintains several data structures to add and cancel orders in `O(1)` time complexity. Executions are done in `O(log k)`, where `k` is the size of the best matching price.
2. `Limit.h`: Defines a limit object as a doubly linked list of orders to be used in a binary tree.
3. `Order.h`: Defines an order object with all necessary information to operate the limit order book.
4. `Execution.h`: Defines an execution object to store necessary information for an execution.

## Exchange Server (Networking via ASIO)

1. Custom Server-Client Interfaces via ASIO: [Custom Server Client Interface: Net Repository](https://github.com/rezabrizi/Net) - credit to OneLonerCoder.
2. `ExchangeServer.h`: An implementation of the server interface.
3. `MarketParticipantSample.cpp`: An implementation of the client interface.

## Some Implementation Notes

1. `MessagingQueue.h`: Thread-Safe Queue.
2. `CentralMessageSystem.h`: The application uses a publisher-subscriber pattern to distribute messages to different components. Each class can subscribe to a message topic using a callback function. The `CentralMessageSystem` acts as a dispatcher by constantly dispatching messages to the subscribers of that message type. All messages are stored as pointers, so their type can be determined at runtime.
3. `DBConnection.h`: The application uses a PostgreSQL database to store data. The `DBConnection.h` class provides an instance of a connection to the database with an API called `query`. This API writes queries to a connected database and returns the results.

## Issues

1. If you have any issues, feel free to open an issue on the repo.
2. If your issue is not resolved, feel free to contact me via [email](mailto:rxa200026@utdallas.edu).
