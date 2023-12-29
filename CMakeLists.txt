cmake_minimum_required(VERSION 3.26)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 17)

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
        src/MessagingQueue.cpp
        src/CentralMessageSystem.cpp
)
