//
// Created by Newton on 2020/6/5.
//

#ifndef SUPERDUPERSERVER_APPLICATION_H
#define SUPERDUPERSERVER_APPLICATION_H

#include <iostream>
#include "server.h"

namespace dns {
    /*
     * this class is created for integration of the server's functions
     */
    class Application{
    private:
        Server m_server;
    public:
        Application() : m_server() {};
        virtual ~Application(){};
        /*function*/
        void run();
    };
}
#endif //SUPERDUPERSERVER_APPLICATION_H
