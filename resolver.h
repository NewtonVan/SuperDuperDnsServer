//
// Created by Newton on 2020/6/5.
//

#ifndef SUPERDUPERSERVER_RESOLVER_H
#define SUPERDUPERSERVER_RESOLVER_H

#include <iostream>
#include "query.h"
#include "response.h"
#include "toolMessage.h"

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
        void configure(const std::string&);
//        void process(const Query &, Response &);
        bool process(const Query &, Response &, const char *, int &, char *const);
    protected:
        struct TicToc{
            unsigned long long tic;  // unit is clock
            unsigned long long TTL;  // unit is second
            unsigned long long toc;  // unit is clock

            TicToc() : tic(0), TTL(0), toc(0){}
            void UpdateTic(unsigned long long ntic, unsigned long long nTTL);
        };
        /*
         * store the rr information
         */
        struct Host{
            std::string ipAddr;
            std::string name;
            TicToc tictoc;
        };

        std::vector<Host> m_hosts;
        std::vector<Host> m_UpperDnsServer;
        int p_socketfd; // p respresent proxy

        void AddCache(char *buff, int lth);
        void UpdateCache(void);
    };
}
#endif //SUPERDUPERSERVER_RESOLVER_H
