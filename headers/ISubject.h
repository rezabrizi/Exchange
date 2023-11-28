//
// Created by Reza Tabrizi on 11/27/23.
//

#ifndef LIMITORDERBOOK_ISUBJECT_H
#define LIMITORDERBOOK_ISUBJECT_H

#include "IObserver.h"
#include <list>

class ISubject{

public:

    virtual ~ISubject(){}
    virtual void attach(IObserver* observer) = 0;
    virtual void detach(IObserver* observer) = 0;
    virtual void notify() = 0;
};


#endif //LIMITORDERBOOK_ISUBJECT_H
