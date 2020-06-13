//
// Created by Newton on 2020/6/5.
//

#ifndef SUPERDUPERSERVER_RESOLVER_H
#define SUPERDUPERSERVER_RESOLVER_H

#include <iostream>
#include "query.h"
#include "response.h"

namespace dns{
    /*
     * resolver part of the proxy dns server
     */
    class Resolver{
    public:
        Resolver(){};
        virtual ~Resolver(){};

        /*functions*/
        void init(const std::string&);
        void process(const Query&, Response&);
    protected:
        /*
         * store the rr information
         */
        struct Host{
            std::string ipAddr;
            std::string name;
        };

        std::vector<Host> m_hosts;
    };
}
#endif //SUPERDUPERSERVER_RESOLVER_H
