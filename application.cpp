//
// Created by Newton on 2020/6/6.
//

#include "application.h"

using namespace dns;

void Application::ui(const std::string flname)
{
    std::ifstream paint;
    paint.open(flname.c_str(), std::ios::in);
    std::string pen;

    while (getline(paint, pen)){
        std::cout<<pen<<std::endl;
    }
    paint.close();
}
/*
 * simply integrate the functions in the 'server' class
 */
void Application::run()
{
    int port= 53;
    m_server.init(port);
#ifdef _WIN32
    m_server.run(".\\dns.log");
#else
    m_server.run("./dns.log");
#endif
}