

# Electronic Exchange 📡 📈 💵
This is a program to demonstrate key components of an electronic exchange via `C++` and `ASIO`.

## Features 💡
1. **Efficient Limit Order Book 💨**: 
   1. Add and cancel orders in `O(1)` time complexity. 
   2. Executions are done in `O(log k)`, where `k` is the size of the best matching price.
2. **Custom Server-Client interface 📶**
   1. [Net Repository](https://github.com/rezabrizi/Net) is a custom server-client interface built using `ASIO` for low-level server-client communication via `TCP`.
   2. The `ExchangeServer.h` is a sample implementation of the `net_server.h` interface of the [Net Repository](https://github.com/rezabrizi/Net).
    This file can be expanded to implement additional functionality such as more sophisticated client authentication.
   3. The `MarketParticipantSample.cpp` is a sample implementation of the `net_client.h` interface of the [Net Repository](https://github.com/rezabrizi/Net).
      This file can be expanded to implement additional functionality such as automated order placement based on quantitative models. 
3. **Database Support 💾**
   1. All orders, executions, client information, and registered instruments are synced with a database to save and recover the state of the exchange. 

## Installation 🔧

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
10. Run sample client application: `Client 1`.
11. Run sample client application: `Client 2`.
12. You should now be able to connect with registered client names to the server and send in your orders.

## Order Book Files
1. `OrderBookManager.h`: Holds pointers to all active `LOB` objects. 
2. `LOB.h`: Maintains several data structures to provide efficient operations for the limit order book. 
3. `Limit.h`: Defines a limit object as a doubly linked list of orders to be used in a binary tree.
4. `Execution.h`: Defines an execution object to store necessary information for an execution.
5. `Order.h`: Defines an order object with all necessary information to operate the limit order book.


## Some Implementation Notes

1. `MessagingQueue.h`: Thread-Safe Queue.
2. `CentralMessageSystem.h`: The application uses a publisher-subscriber pattern to distribute messages to different components. Each class can subscribe to a message topic using a callback function. The `CentralMessageSystem` acts as a dispatcher by constantly dispatching messages to the subscribers of that message type. All messages are stored as pointers, so their type can be determined at runtime.
3. `DBConnection.h`: The application uses a PostgreSQL database to store data. The `DBConnection.h` class provides an instance of a connection to the database with an API called `query`. This API writes queries to a connected database and returns the results.

## Contribution 🤝
1. **Expansion**: I would love to see new ideas to expand this project. Feel free to submit pull requests for proposed changes and expansions.
2. **Python Integration**: If anyone is interested in exposing Python API's for this library please contact me, as Python and C++ integration really interests me. 


## Contact - Issues 📬

1. If you have any issues, feel free to open an issue on the repo.
2. If your issue is not resolved, feel free to contact me via [email](mailto:rxa200026@utdallas.edu).
