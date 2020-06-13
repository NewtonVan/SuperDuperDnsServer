//
// Created by Newton on 2020/6/5.
//

#ifndef SUPERDUPERSERVER_SERVER_H
#define SUPERDUPERSERVER_SERVER_H

#include <iostream>
#include "query.h"
#include "response.h"
#include "resolver.h"

namespace dns{
    class Server{
    public:
        Server();
        virtual ~Server(){};

        /*functions*/
        void init(int&);
        void run();
    private:
        int m_socketfd;
        Query m_query;
        Response m_response;
        Resolver m_resolver;
    };
}
#endif //SUPERDUPERSERVER_SERVER_H
