//
// Created by Reza Tabrizi on 12/24/23.
//
#include "../headers/OrderBookManager.h"

OrderBookManager::OrderBookManager(CentralMessageSystem &CMS) : CMS(CMS) {
    CMS.Subscribe("AddOrderMessage", [this](const BaseMessage& message) {
        this->ProcessMessage(static_cast<const AddOrderMessage&>(message));
    });

    CMS.Subscribe("CancelOrderMessage", [this](const BaseMessage& message) {
        this->ProcessMessage(static_cast<const CancelOrderMessage&>(message));
    });
    // ... subscribe to other message types as needed
}

