cmake_minimum_required(VERSION 3.26)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 17)

# Locate the PostgreSQL library
find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS})

# Manually set the include and library paths for libpqxx
include_directories(/opt/homebrew/include) # replace with the actual path
link_directories(/opt/homebrew/lib)       # replace with the actual path


add_executable(LimitOrderBook main.cpp
        headers/Order.h
        headers/Limit.h
        src/Limit.cpp
        headers/LOB.h
        src/LOB.cpp
        headers/Exchange.h
        headers/IObserver.h
        headers/ISubject.h
        headers/OrderBookManager.h
        src/OrderBookManager.cpp
        headers/CentralMessageSystem.h
        headers/MessagingQueue.h
        headers/Message.h
        src/CentralMessageSystem.cpp
        headers/Execution.h
        headers/DBConnection.h
        src/DBConnection.cpp
        headers/Connection.h
        headers/Client.h
        headers/NetMessage.h
        headers/NetCommon.h
        headers/NetConnection.h
)

# Link the PostgreSQL and libpqxx libraries
target_link_libraries(LimitOrderBook ${PostgreSQL_LIBRARIES} pqxx)