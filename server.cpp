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
 * old fashioned
 void Server::init(int &port) {
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    //protocol domain
    servAddr.sin_family = AF_INET;
    //default ip
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //port
    servAddr.sin_port = htons(port);
    //create socket
    if ((m_socketfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        return;
    }
    unsigned value = 1;
    setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
    //bind socket to port
    if (bind(m_socketfd, (struct sockaddr *)&servAddr, sizeof(servAddr))) {
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return;
    }
    //dynamically allocating a port
    if (port == 0) {
        socklen_t namelen = sizeof(servAddr);
        if (getsockname(m_socketfd, (struct sockaddr *)&servAddr, &namelen) == -1) {
            printf("getsockname error: %s(errno: %d)\n",strerror(errno),errno);
            return;
        }
        port = ntohs(servAddr.sin_port);
    }
    std::cout<<"server running on port:"<<port<<std::endl;
}
 */
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

//    if (NULL== itoa(port, str_port, 10)){
//        fprintf(stderr, "damn it, convert failure\n");
//        exit(2);
//    } failed because linux doesn't support itoa
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

    /*
     * this part still have some fxxking problem, I'm not quite sure whether it can work well
     */
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
void Server::run()
{
    char rbuf[MAX_UDP_LTH], sbuf[MAX_UDP_LTH];
    struct sockaddr_in clientAddr;
    socklen_t len= sizeof(struct sockaddr_in);

    while (true){
        int lth= (int)recvfrom(m_socketfd, rbuf, sizeof(rbuf), 0, (struct sockaddr*)&clientAddr, &len);
        if (lth <= 0){
            continue;
        }
        m_query.decode(rbuf, lth);
        std::cout<<m_query.to_string();

        if(m_resolver.process(m_query, m_response, rbuf, lth, sbuf)){   // cache hit
            memset(sbuf, 0, sizeof(sbuf));
            lth= m_response.encode(sbuf);
            std::cout<<m_response.to_string();
        }

        sendto(m_socketfd, sbuf, lth, 0, (struct sockaddr*)&clientAddr, len);

        std::cout<<std::endl;
    }

}