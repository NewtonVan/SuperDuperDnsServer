//
// Created by Newton on 2020/6/6.
//


#include "SuperDuperLib.h"
#include "server.h"



using namespace dns;

Server::Server()
{
#ifdef _WIN32
    m_resolver.init(".\\server.cache");
    m_resolver.configure(".\\upperproxy.confg");
#else
    m_resolver.init("./server.cache");
    m_resolver.configure("./upperproxy.confg");
#endif
}

/*
 * init the server
 * create socket used for dns proxy server
 * set it to listening mode
 */
void Server::init(int &port)
{
    int status;
    struct addrinfo hints;
    struct addrinfo *servInfo;
    char str_port[1<<5];
    unsigned value= 1;

    sprintf(str_port, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family= AF_INET;
    hints.ai_socktype= SOCK_DGRAM;
    hints.ai_flags= AI_PASSIVE;

    if (0!= (status= getaddrinfo(NULL, str_port, &hints, &servInfo))){
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
            exit(1);
    }

    if (-1== (m_socketfd= socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol))){
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        exit(1);
    }


#ifdef _WIN32
    setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&value), sizeof(value));
#else
    setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
#endif

    if (bind(m_socketfd, servInfo->ai_addr, servInfo->ai_addrlen)){
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        exit(1);
    }
    if (port == 0) {
        socklen_t namelen = servInfo->ai_addrlen;
        if (getsockname(m_socketfd, servInfo->ai_addr, &namelen) == -1) {
            printf("getsockname error: %s(errno: %d)\n",strerror(errno),errno);
            return;
        }
        struct sockaddr_in *servAddr= (struct sockaddr_in*)(servInfo->ai_addr);
        port = ntohs(servAddr->sin_port);
    }

    std::cout<<"server running on port:"<<port<<std::endl;

}

/*
 * wait for the query datagram
 * process the datagram
 * solve the datagram
 */
void Server::run(const std::string logNm)
{
    char rbuf[MAX_UDP_LTH], sbuf[MAX_UDP_LTH];
    // TODO information function here, so make some change, if something wrong, recover sockaddr to sockaddr_in
    struct sockaddr clientAddr;
    char p_clientAddr[INET6_ADDRSTRLEN];
    socklen_t len= sizeof(struct sockaddr_in);

    // TODO log function
//    logFile.open(logNm.c_str(), std::ios::app);
    m_resolver.getLog(logNm);

    while (true){
        int lth= (int)recvfrom(m_socketfd, rbuf, sizeof(rbuf), 0, (struct sockaddr*)&clientAddr, &len);
        if (lth <= 0){
            continue;
        }


        logFile.open(logNm.c_str(), std::ios::app);
        std::cout<<"******************************************"<<std::endl;
        logFile<<"******************************************"<<std::endl;
        // TODO information function show received from where
        if (AF_INET== clientAddr.sa_family){
            struct sockaddr_in *v4CltAddr= (struct sockaddr_in*)&clientAddr;
            inet_ntop(AF_INET, &(v4CltAddr->sin_addr), p_clientAddr, INET_ADDRSTRLEN);
            std::cout<<"Received from: "<<p_clientAddr<<std::endl;
            logFile<<"Received form: "<<p_clientAddr<<std::endl;
        }
        else{
            struct sockaddr_in6 *v6CltAddr= (struct sockaddr_in6*)&clientAddr;
            inet_ntop(AF_INET6, &(v6CltAddr->sin6_addr), p_clientAddr, INET6_ADDRSTRLEN);
            std::cout<<"Received from: "<<p_clientAddr<<std::endl;
            logFile<<"Received form: "<<p_clientAddr<<std::endl;
        }

        m_query.decode(rbuf, lth);
        logData= m_query.to_string();
        std::cout<<logData;
        logFile<<logData;

        logFile.close();

        if(m_resolver.process(m_query, m_response, rbuf, lth, sbuf)){   // cache hit
            memset(sbuf, 0, sizeof(sbuf));
            lth= m_response.encode(sbuf);
            logData= m_response.to_string();

            logFile.open(logNm.c_str(), std::ios::app);

            std::cout<<logData;
            logFile<<logData;
            std::cout<<"Cache was hit"<<std::endl;
            std::cout<<"******************************************\n\n"<<std::endl;
            logFile<<"Cache was hit"<<std::endl;
            logFile<<"******************************************\n\n"<<std::endl;

            logFile.close();
        }

        sendto(m_socketfd, sbuf, lth, 0, (struct sockaddr*)&clientAddr, len);

        std::cout<<std::endl;
    }

}