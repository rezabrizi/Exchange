//
// Created by Reza Tabrizi on 1/26/24.
//

#include "NetCommon.h"


#ifndef LIMITORDERBOOK_NETMESSAGE_H
#define LIMITORDERBOOK_NETMESSAGE_H

namespace client {
    namespace net {
        template <typename T>
        struct messageHeader {
            T id{};
            uint32_t size = 0;
        };

        template <typename T>
        struct message {
            messageHeader<T> header{};
            std::vector<uint8_t> body;

            size_t size() const{
                return sizeof(messageHeader<T>) + body.size();
            }
        };
    }
}


#endif //LIMITORDERBOOK_NETMESSAGE_H
