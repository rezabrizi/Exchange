cmake_minimum_required(VERSION 3.26)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 17)

add_executable(LimitOrderBook main.cpp
        headers/Order.h
        headers/Limit.h
        src/Limit.cpp
        headers/LOB.h
        src/LOB.cpp
)
