//
// Created by Newton on 2020/6/5.
//

#ifndef SUPERDUPERSERVER_RESPONSE_H
#define SUPERDUPERSERVER_RESPONSE_H

#include "message.h"
#include "query.h"

namespace dns {
    /*
     * children class of the Message
     * used to answer the query
     */
    class Response : public Message{
    public:
        Response(){};
        virtual ~Response(){};

        /*functions*/
        int encode(char *);
        std::string to_string();
    };
}
#endif //SUPERDUPERSERVER_RESPONSE_H
