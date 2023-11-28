cmake_minimum_required(VERSION 3.26)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 17)

add_executable(LimitOrderBook main.cpp
        headers/Order.h
        headers/Limit.h
        src/Limit.cpp
        headers/LOB.h
        src/LOB.cpp
        headers/LOBLogger.h
        headers/Execution.h
        headers/OrderLog.h
        headers/ExecutionLog.h
        headers/Books.h
        headers/Exchange.h
        headers/MessagingSystem.h
        headers/Message.h
        headers/IObserver.h
        headers/ISubject.h
        headers/Firm.h
)
