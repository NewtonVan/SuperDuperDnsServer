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
    fstrm.close();
}


void Resolver::configure(const std::string &filename)
{
    ifstream fstrm(filename.c_str());
    char proxyStr[256];

    while (fstrm.getline(proxyStr, sizeof(proxyStr))){
        stringstream sstrm;
        sstrm<<proxyStr;
        Host proxyServer;

        sstrm>>proxyServer.ipAddr;
        sstrm>>proxyServer.name;
        m_UpperDnsServer.push_back(proxyServer);
    }
    fstrm.close();
}

void Resolver::getLog(const std::string logNm)
{
    logName= logNm;
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

    tool.decodeTool(buff, lth);
    logData= tool.to_string();
    cout<<logData;
    logFile<<logData;

    for (vector<Message::MResource>::iterator tIter= tool.m_answers.begin();
    tool.m_answers.end()!= tIter; ++tIter){
        if (Message::MT_A== tIter->rType || Message::MT_AAAA== tIter->rType){
            Host newCache;
            int fnd= 0;

            newCache.name= tIter->rName;
            newCache.ipAddr= tIter->rIp;
            newCache.tictoc.UpdateTic(now, tIter->rTTL);
            newCache.af= Message::MT_A== tIter->rType ? AF_INET : AF_INET6;


            for (vector<Host>::iterator hIter= m_hosts.begin();
            m_hosts.end()!= hIter; ++hIter){
                if (hIter->ipAddr== newCache.ipAddr && hIter->name== newCache.name
                && hIter->af == newCache.af){
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
/*        else if (Message::MT_CNAME== tIter->rType){
            Alias newAlias;
        }*/
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
            if (question.qName== host.name && Message::MT_A == question.qType
            && AF_INET == host.af){
                strcpy(resource.rName, question.qName);
                resource.rType= question.qType;
                resource.rClass= question.qClass;
                resource.rTTL= host.tictoc.TTL;
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
            else if (question.qName== host.name && Message::MT_AAAA == question.qType
            && AF_INET6 == host.af){
                strcpy(resource.rName, question.qName);
                resource.rType= question.qType;
                resource.rClass= question.qClass;
                resource.rTTL= host.tictoc.TTL;
                resource.rdLength= sizeof(struct in6_addr);
                memcpy(resource.rIp, host.ipAddr.c_str(), host.ipAddr.size());

                struct sockaddr_in6 adr_inet6;
                memset(&adr_inet6, 0, sizeof(adr_inet6));
                inet_pton(AF_INET6, host.ipAddr.c_str(), &(adr_inet6.sin6_addr));
                memcpy(resource.rData, &(adr_inet6.sin6_addr.s6_addr), sizeof(struct in6_addr));

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
        logFile.open(logName.c_str(), std::ios::app);
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


                AddCache(u_rbuf, numBytes);
                UpdateCache();
                bufLth= numBytes;

                logData= "Cache wasn't hit\n";
                cout<<logData;
                cout<<"******************************************\n\n"<<endl;
                logFile<<logData<<endl;
                logFile<<"******************************************\n\n"<<endl;
                logFile.close();
                return false; //cache missed
            }
        }
        logFile.close();
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
