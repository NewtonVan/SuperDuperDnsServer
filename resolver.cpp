//
// Created by Newton on 2020/6/6.
//


#include "SuperDuperLib.h"
#include "resolver.h"

using namespace dns;
using namespace std;

/*
 * read the very beginning cache from the configure file
 * push this data into m_hosts
 */
void Resolver::init(const std::string &filename)
{
    ifstream fstrm(filename.c_str());
    unsigned long long now= clock();
    char hostStr[256];

    while (fstrm.getline(hostStr, sizeof(hostStr))){
        stringstream sstrm;
        sstrm<<hostStr;
        Host host;
        sstrm>>host.ipAddr;
        sstrm>>host.name;
        host.tictoc.UpdateTic(now, 10);
        m_hosts.push_back(host);
    }
}


void Resolver::configure(const std::string &filename)
{
    ifstream fstrm(filename.c_str());
    char proxyStr[256];

    while (fstrm.getline(proxyStr, sizeof(proxyStr))){
        stringstream sstrm;
        sstrm<<proxyStr;
        Host proxyServer;
        // TODO here need to confirm the form of the upper server configure file, so this is a temporary version
        sstrm>>proxyServer.ipAddr;
        sstrm>>proxyServer.name;
        m_UpperDnsServer.push_back(proxyServer);
    }
}


void Resolver::TicToc::UpdateTic(unsigned long long ntic, unsigned long long nTTL)
{
    tic= ntic;
    TTL= nTTL;
    toc= tic+TTL*CLOCKS_PER_SEC;
}
void Resolver::UpdateCache(void)
{
    long long now= (long long)clock();

    for (vector<Host>::iterator hIter= m_hosts.begin();
         m_hosts.end()!= hIter; ){
        if (now> hIter->tictoc.toc){
            hIter= m_hosts.erase(hIter);
        }
        else{
            ++hIter;
        }
    }
}
void Resolver::AddCache(char *buff, int lth)
{
    ToolMessage tool;
    unsigned long long now= clock();
    // TODO check segment fault
//    printf("segment fault test point 3\n");
    tool.decodeTool(buff, lth);

    for (vector<Message::MResource>::iterator tIter= tool.m_answers.begin();
    tool.m_answers.end()!= tIter; ++tIter){
        if (Message::MT_A== tIter->rType || Message::MT_AAAA== tIter->rType){
            Host newCache;
            int fnd= 0;

            newCache.name= tIter->rName;
            newCache.ipAddr= tIter->rIp;
            newCache.tictoc.UpdateTic(now, tIter->rTTL);

            for (vector<Host>::iterator hIter= m_hosts.begin();
            m_hosts.end()!= hIter; ++hIter){
                if (hIter->ipAddr== newCache.ipAddr && hIter->name== newCache.name){
                    fnd= 1;
                    if (newCache.tictoc.toc > hIter->tictoc.toc){
                        hIter->tictoc= newCache.tictoc;
                    }
                    break;
                }
            }
            if (!fnd){
                m_hosts.push_back(newCache);
            }
        }
    }
}

/*
 * newest process funtion
 * achieve the proxy function
 * achieve the cache function
 */
bool Resolver::process(const dns::Query &query, dns::Response &response, const char *rbuf, int &bufLth, char *const sbuf)
{
    response.m_answers.clear();
    response.m_questions.clear();
    int hitd;
    vector<Query::MQuestion> questions= query.getQuestions();

    for (vector<Query::MQuestion>::iterator qIter= questions.begin();
    questions.end()!= qIter; ++qIter){
        Query::MQuestion question= *qIter;
        Response::MResource resource;
        hitd= 0;

        for (vector<Host>::iterator hIter= m_hosts.begin();
        m_hosts.end()!= hIter; ++hIter){
            Host host= *hIter;

            // confirm whether the answer is suitable for the question
            // & whether it is a A type query
            if (question.qName== host.name && question.qType== Message::MT_A){
                strcpy(resource.rName, question.qName);
                resource.rType= question.qType;
                resource.rClass= question.qClass;
                resource.rTTL= 10;
                resource.rdLength= sizeof(uint32_t);
                memcpy(resource.rIp, host.ipAddr.c_str(), host.ipAddr.size());

                struct sockaddr_in adr_inet;
                memset(&adr_inet, 0, sizeof(adr_inet));
                // inet_aton(host.ipAddr.c_str(), &adr_inet.sin_addr);
                inet_pton(AF_INET, host.ipAddr.c_str(), &(adr_inet.sin_addr));
                memcpy(resource.rData, &(adr_inet.sin_addr.s_addr), sizeof(uint32_t));

                response.m_answers.push_back(resource);
                hitd= 1;
                break;
            }
            else if (question.qName== host.name && question.qType== Message::MT_AAAA){
                strcpy(resource.rName, question.qName);
                resource.rType= question.qType;
                resource.rClass= question.qClass;
                resource.rTTL= 10;
                resource.rdLength= sizeof(struct in6_addr);
                memcpy(resource.rIp, host.ipAddr.c_str(), host.ipAddr.size());

                struct sockaddr_in6 adr_inet6;
                memset(&adr_inet6, 0, sizeof(adr_inet6));
                inet_pton(AF_INET6, host.ipAddr.c_str(), &(adr_inet6.sin6_addr));
                memcpy(resource.rData, adr_inet6.sin6_addr.s6_addr, sizeof(struct in6_addr));

                response.m_answers.push_back(resource);
                hitd=1;
                break;
            }
        }
        if (!hitd){
            break;
        }
        response.m_questions.push_back(question);
    }

    if (!hitd){
        char u_sbuf[MAX_UDP_LTH], u_rbuf[MAX_UDP_LTH];
        memset(u_sbuf, 0, MAX_UDP_LTH);
        memcpy(u_sbuf, rbuf, bufLth);

        struct addrinfo hints, *serverInfo, *ptr;
        struct timeval tBound;
        // ToolMessage proxyQuery;
        int rv, numBytes, t_sec, t_usec;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family= AF_UNSPEC;
        hints.ai_socktype= SOCK_DGRAM;

        t_sec= 0;
        t_usec= 500000;
        tBound.tv_sec= t_sec;
        tBound.tv_usec= t_usec;

        for (vector<Host>::iterator uIter= m_UpperDnsServer.begin();
             m_UpperDnsServer.end()!= uIter; ++uIter){
            Host proxyServer= *uIter;
            struct sockaddr serverAddr;
            socklen_t addrLen= sizeof(serverAddr);

            // prepare for make a query to upper proxy dns server
            if (0!= (rv= getaddrinfo(proxyServer.ipAddr.c_str(), "53", &hints, &serverInfo))){
                fprintf(stderr, "getaddrinfo: %s", gai_strerror(rv));
                exit(1);
            }
            // find an ok upper server
            for (ptr= serverInfo; NULL!= ptr; ptr= ptr->ai_next){
                if (-1== (p_socketfd= socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol))){
                    perror("talker: socket");
                    continue;
                }
                break;
            }

            if (NULL == ptr){
                fprintf(stderr, "talker: failed to create socket\n");
                exit(1);
            }

            // quit if recvfrom functions meet with a timeout
            setsockopt(p_socketfd, SOL_SOCKET, SO_RCVTIMEO, &tBound, sizeof(tBound));

            if ((numBytes= sendto(p_socketfd, u_sbuf, bufLth, 0, ptr->ai_addr, ptr->ai_addrlen))< 0){
                perror("talker: sendto");
                exit(2);
            }

            memset(u_rbuf, 0, MAX_UDP_LTH);
            numBytes= recvfrom(p_socketfd, u_rbuf, MAX_UDP_LTH, 0, &serverAddr, &addrLen);

            freeaddrinfo(serverInfo);
            close(p_socketfd);

            if (numBytes< 0){   // meet with failure with upper server
                if (EWOULDBLOCK== errno){
                    fprintf(stderr, "socket time out\n");
                }
                else{
                    fprintf(stderr, "recvfrom error\n");
                }
            }
            else{
                memset(sbuf, 0, MAX_UDP_LTH);
                memcpy(sbuf, u_rbuf, numBytes);

                // TODO check segment fault
//                printf("segment fault test point 1\n");
                AddCache(u_rbuf, numBytes);
                // TODO check segment fault
//                printf("segment fault test point 2\n");
                UpdateCache();
                bufLth= numBytes;
                return false; //cache missed
            }
        }
    }

    response.m_id= query.getID();
    response.m_qr= 1;
    response.m_opcode= query.getOpcode();
    response.m_aa= 0;
    response.m_tc= 0;
    response.m_rd= 0;
    response.m_ra= 0;

    if (response.m_answers.size()!= response.m_questions.size()){
        response.m_rcode= Message::MC_SERVER_ERROR;
    }
    else{
        response.m_rcode= Message::MC_NO_ERROR;
    }

    response.m_qdCount= (int)response.m_questions.size();
    response.m_anCount= (int)response.m_answers.size();
    response.m_nsCount= 0;
    response.m_arCount= 0;

    UpdateCache();

    return true;    // cache hitd or unsuccessfully handle with upper server
}






///*
// * process the query
// * find the suitable answer
// * format the answer to dns datagram
// * this old function need to be abandoned
// * still here because it's code maybe used later
// * after the entire project done, it will be deleted
// */
//void Resolver::process(const Query &query, Response &response)
//{
//    response.m_questions.clear();
//    response.m_answers.clear();
//
//    vector<Response::MQuestion> questions= query.getQuestions();
//    for (vector<Response::MQuestion>::iterator qIter= questions.begin();
//         questions.end()!= qIter; ++qIter){
//        // resolver is the friend of the Message
//        Response::MQuestion question= *qIter;
//        Response::MResource resource;
//
//        int hitd= 0;
//
//        /*
//         * TODO
//         * searching the cached server ip consumed O(n) time
//         * maybe I will replace that method with Red-Black tree in the future
//         * Depends on the mood...
//         */
//        for (vector<Host>::iterator hIter= m_hosts.begin();
//             m_hosts.end()!= hIter; ++hIter){
//            Host host= *hIter;
//
//            // confirm whether the answer is suitable for the question
//            // & whether it is a A type query
//            if (question.qName== host.name && question.qType== Message::MT_A){
//                strcpy(resource.rName, question.qName);
//                resource.rType= question.qType;
//                resource.rClass= question.qClass;
//                resource.rTTL= 10;
//                resource.rdLength= sizeof(uint32_t);
//                memcpy(resource.rIp, host.ipAddr.c_str(), host.ipAddr.size());
//
//                struct sockaddr_in adr_inet;
//                memset(&adr_inet, 0, sizeof(adr_inet));
//                // inet_aton(host.ipAddr.c_str(), &adr_inet.sin_addr);
//                inet_pton(AF_INET, host.ipAddr.c_str(), &(adr_inet.sin_addr));
//                memcpy(resource.rData, &(adr_inet.sin_addr.s_addr), sizeof(uint32_t));
//
//                response.m_answers.push_back(resource);
//                hitd= 1;
//                break;
//            }
//        }
//
//
//        if (!hitd){
//            struct addrinfo hints, *serverInfo, *ptr;
//            struct timeval tBound;
//            ToolMessage proxyQuery;
//            int rv, numBytes, sndlth, t_sec, t_usec;
//            char rbuf[MAX_UDP_LTH], sbuf[MAX_UDP_LTH];
//
//            memset(&hints, 0, sizeof(hints));
//            hints.ai_family= AF_UNSPEC;
//            hints.ai_socktype= SOCK_DGRAM;
//
//            t_sec= 5;
//            t_usec= 0;
//            tBound.tv_sec= t_sec;
//            tBound.tv_usec= t_usec;
//
//            // header part
//            memset(sbuf, 0, sizeof(sbuf));
//            memset(rbuf, 0, sizeof(rbuf));
//            proxyQuery.m_questions.clear();
//            proxyQuery.m_answers.clear();
//            proxyQuery.m_id= query.getID();
//            proxyQuery.m_qr= 0;
//            proxyQuery.m_opcode= query.getOpcode();
//            proxyQuery.m_aa= 0;
//            proxyQuery.m_tc= 0;
//            proxyQuery.m_rd= 1;
//            proxyQuery.m_ra= 0;
//            proxyQuery.m_rcode= query.getRcode();
//            // question part
//            proxyQuery.m_questions.push_back(question);
//            // header part
//            proxyQuery.m_qdCount= 1;
//            proxyQuery.m_anCount= 0;
//            proxyQuery.m_nsCount= 0;
//            proxyQuery.m_arCount= 0;
//            // test
//            cout<<proxyQuery.to_string()<<endl;
//            sndlth= proxyQuery.encodeTool(sbuf);
//
//            // test every upper dns proxy dns server
//            for (vector<Host>::iterator uIter= m_UpperDnsServer.begin();
//                 m_UpperDnsServer.end()!= uIter; ++uIter){
//                Host proxyServer= *uIter;
//                struct sockaddr serverAddr;
//                int addrLen= sizeof(serverAddr);
//
//                // prepare for make a query to upper proxy dns server
//                if (0!= (rv= getaddrinfo(proxyServer.ipAddr.c_str(), "53", &hints, &serverInfo))){
//                    fprintf(stderr, "getaddrinfo: %s", gai_strerror(rv));
//                    exit(1);
//                }
//                for (ptr= serverInfo; NULL!= ptr; ptr= ptr->ai_next){
//                    if (-1== (p_socketfd= socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol))){
//                        perror("talker: socket");
//                        continue;
//                    }
//                    break;
//                }
//
//                if (NULL == ptr){
//                    fprintf(stderr, "talker: failed to create socket\n");
//                    exit(1);
//                }
//
//                // quit if recvfrom functions meet with a timeout
//                setsockopt(p_socketfd, SOL_SOCKET, SO_RCVTIMEO, &tBound, sizeof(tBound));
//
//                if ((numBytes= sendto(p_socketfd, sbuf, sndlth, 0, ptr->ai_addr, ptr->ai_addrlen))< 0){
//                    perror("talker: sendto");
//                    exit(2);
//                }
//
//                numBytes= recvfrom(p_socketfd, rbuf, MAX_UDP_LTH, 0, &serverAddr, &addrLen);
//
//                // handle the data receieved from the upper proxy dns server
//                if (numBytes< 0){
//                    if (EWOULDBLOCK== errno){
//                        fprintf(stderr, "socket time out\n");
//                    }
//                    else{
//                        fprintf(stderr, "recvfrom error\n");
//                    }
//                }
//                else{
//                    ToolMessage proxyResponse;
//
//                    proxyResponse.decodeTool(rbuf, numBytes);
//                    if (proxyResponse.m_answers.size()> 0){
//                        char ipaddr[INET6_ADDRSTRLEN];
//                        cout<<proxyQuery.to_string()<<endl;
//
//                        strcpy(resource.rName, question.qName);
//                        resource.rType= question.qType;
//                        resource.rClass= question.qClass;
//                        resource.rTTL= 10;
//
//                        if (Message::MT_A== resource.rType){
//                            resource.rdLength= sizeof(uint32_t);
//                            vector<ToolMessage::MResource>::iterator prIter= proxyQuery.m_answers.begin();
//                            memcpy(resource.rIp, prIter->rIp, resource.rdLength);
//                            // confused about whether uint32_t or in_addr
//                            memcpy(resource.rData, prIter->rData, sizeof(struct in_addr));
//                        }
//                        else{
//                            resource.rdLength= sizeof(uint64_t);
//                            vector<ToolMessage::MResource>::iterator prIter= proxyQuery.m_answers.begin();
//                            memcpy(resource.rIp, prIter->rIp, resource.rdLength);
//                            // confused here
//                            memcpy(resource.rData, prIter->rData, sizeof(struct in6_addr));
//                        }
//
//                        /*
//                         * TODO I came up with an idea why not just copy the entire
//                         * information from the message I received
//                         * may be I will achieve this method later
//                         */
//                        freeaddrinfo(serverInfo);
//                        close(p_socketfd);
//                        hitd= 1;
//                        break;
//                    }
//                }
//                freeaddrinfo(serverInfo);
//                close(p_socketfd);
//            }
//
//        }
//
//        /*
//         * this temporarily use get addrinfo() to get the ip address
//         * used as plan 3
//         */
//        if (!hitd){
//            struct addrinfo hints, *result, *ptr;
//            char ipaddr[INET6_ADDRSTRLEN];
//            int status;
//            void *tpaddr;
//
//            memset(&hints, 0, sizeof(hints));
//            hints.ai_family= AF_UNSPEC;
//            hints.ai_socktype= SOCK_DGRAM;
//
//            if (0!= (status= getaddrinfo(question.qName, NULL, &hints, &result))){
//                fprintf(stderr, "getaddrinfo: %s", gai_strerror(status));
//                exit(1);
//            }
//            resource.rType= question.qType;
//            resource.rClass= question.qClass;
//            resource.rTTL= 10;
//            resource.rdLength= sizeof(uint32_t);
//            for (ptr= result; NULL!= ptr; ptr= ptr->ai_next){
//                if (AF_INET== ptr->ai_family){
//                    struct sockaddr_in *ipv4= (struct sockaddr_in*)ptr->ai_addr;
//                    tpaddr= &(ipv4->sin_addr);
//
//                    inet_ntop(ptr->ai_family, tpaddr, ipaddr, sizeof(ipaddr));
//                    memcpy(resource.rIp, ipaddr, sizeof(ipaddr));
//                    // I'm quite confused about here, using uint32_t or in_addr
//                    memcpy(resource.rData, &(ipv4->sin_addr.s_addr), sizeof(struct in_addr));
//
//                    response.m_answers.push_back(resource);
//                    break;
//                }
//                    // TODO complete the situation in ipv6
//                else if (AF_INET6== ptr->ai_family){
//                    struct sockaddr_in6 *ipv6= (struct sockaddr_in6*)ptr->ai_addr;
//                    tpaddr= &(ipv6->sin6_addr);
//
//                    inet_ntop(ptr->ai_family, tpaddr, ipaddr, sizeof(ipaddr));
//                    memcpy(resource.rIp, ipaddr, sizeof(ipaddr));
//                    memcpy(resource.rData, &(ipv6->sin6_addr.s6_addr), sizeof(struct in6_addr));
//                }
//            }
//
//            freeaddrinfo(result);
//        }
//
//        response.m_questions.push_back(question);
//    }
//
//
//    response.m_id= query.getID();
//    response.m_qr= 1;
//    response.m_opcode= query.getOpcode();
//    response.m_aa= 0;
//    response.m_tc= 0;
//    response.m_rd= 0;
//    response.m_ra= 0;
//
//    if (response.m_answers.size()!= response.m_questions.size()){
//        response.m_rcode= Message::MC_SERVER_ERROR;
//    }
//    else{
//        response.m_rcode= Message::MC_NO_ERROR;
//    }
//
//    response.m_qdCount= (int)response.m_questions.size();
//    response.m_anCount= (int)response.m_answers.size();
//    response.m_nsCount= 0;
//    response.m_arCount= 0;
//}