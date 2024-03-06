cmake_minimum_required(VERSION 3.26)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 17)
set(ASIO_INCLUDE_DIR /Users/rezatabrizi/dev/sdk/asio/include)

# Locate the PostgreSQL library
find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS} ${ASIO_INCLUDE_DIR})

# Manually set the include and library paths for libpqxx
include_directories(/opt/homebrew/include) # replace with the actual path
link_directories(/opt/homebrew/lib)       # replace with the actual path
include_directories(../Net/include)
add_compile_definitions(ASIO_STANDALONE)


add_executable(LimitOrderBook exchange_drive.cpp
        headers/Order.h
        headers/Limit.h
        src/Limit.cpp
        headers/LOB.h
        src/LOB.cpp
        headers/OrderBookManager.h
        src/OrderBookManager.cpp
        headers/CentralMessageSystem.h
        headers/MessagingQueue.h
        headers/Message.h
        src/CentralMessageSystem.cpp
        headers/Execution.h
        headers/DBConnection.h
        headers/ExchangeServer.h
)

add_executable(Client marketParticipant.cpp)

# Link the PostgreSQL and libpqxx libraries
target_link_libraries(LimitOrderBook ${PostgreSQL_LIBRARIES} pqxx)
