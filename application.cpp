//
// Created by Newton on 2020/6/6.
//

#include "application.h"

using namespace dns;

/*
 * simply integrate the functions in the 'server' class
 */
void Application::run()
{
    int port= 53;
    m_server.init(port);
    m_server.run();
}