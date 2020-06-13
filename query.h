//
// Created by Newton on 2020/6/5.
//

#ifndef SUPERDUPERSERVER_QUERY_H
#define SUPERDUPERSERVER_QUERY_H

#include <iostream>
#include "message.h"

namespace dns{
    /*
     * children class of the Message
     * created for format the query data
     */
    class Query : public Message{
    public:
        Query(){};
        virtual ~Query(){};

        /*functions*/
        void decode(const char*, const int);
        std::string to_string();
    };
}
#endif //SUPERDUPERSERVER_QUERY_H
