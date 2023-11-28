//
// Created by Reza Tabrizi on 11/27/23.
//

#ifndef LIMITORDERBOOK_IOBSERVER_H
#define LIMITORDERBOOK_IOBSERVER_H

class Message;

class IObserver {

public:
    virtual ~IObserver();
    virtual void update(const Message& message) = 0;
};

#endif //LIMITORDERBOOK_IOBSERVER_H
