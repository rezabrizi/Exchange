//
// Created by Reza Tabrizi on 11/24/23.
//
#include <unordered_map>
#include "OrderLog.h"
#include "Order.h"
#include "Execution.h"
#include "ExecutionLog.h"


#ifndef LIMITORDERBOOK_LOBLOGGER_H
#define LIMITORDERBOOK_LOBLOGGER_H

class LOBLogger{
    std::unordered_map <int, OrderLog> orderHistory;
    std::unordered_map <int, ExecutionLog> executionHistory;

public:
    void LogOrder(const Order& order);
    void LogExecution (const Execution& execution);
    void RecoverCurrentState();
};

#endif //LIMITORDERBOOK_LOBLOGGER_H
