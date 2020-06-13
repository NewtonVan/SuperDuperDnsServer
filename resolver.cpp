//
// Created by Newton on 2020/6/6.
//

#include <fstream>
#include <sstream>
#include "SuperDuperLib.h"
#include "resolver.h"

using namespace dns;
using namespace std;

/*
 * read from the configure file
 * push this data into m_hosts
 */
void Resolver::init(const std::string &filename)
{
    ifstream fstrm(filename);
    char hostStr[128];

    while (fstrm.getline(hostStr, sizeof(hostStr))){
        stringstream sstrm;
        sstrm<<hostStr;
        Host host;
        sstrm>>host.ipAddr;
        sstrm>>host.name;
        m_hosts.push_back(host);
    }
}

/*
 * process the query
 * find the suitable answer
 * format the answer to dns datagram
 */
void Resolver::process(const Query &query, Response &response)
{
    response.m_questions.clear();
    response.m_answers.clear();

    vector<Response::MQuestion> questions= query.getQuestions();
    for (vector<Response::MQuestion>::iterator qIter= questions.begin();
    questions.end()!= qIter; ++qIter){
        // resolver is the friend of the Message
        Response::MQuestion question= *qIter;
        Response::MResource resource;

        int hitd= 0;

        /*
         * TODO
         * searching the cached server ip consumed O(n) time
         * maybe I will replace that method with Red-Black tree in the future
         * Depends on the mood...
         */
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
                inet_pton(AF_INET, host.ipAddr.c_str(), &adr_inet.sin_addr);
                memcpy(resource.rData, &adr_inet.sin_addr.s_addr, sizeof(uint32_t));

                response.m_answers.push_back(resource);
                hitd= 1;
                break;
            }
        }

        /*
         * this temporarily use get addrinfo() to get the ip address
         */
        if (!hitd){
            struct addrinfo hints, *result, *ptr;
            char ipaddr[INET6_ADDRSTRLEN];
            int status;
            void *tpaddr;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family= AF_UNSPEC;
            hints.ai_socktype= SOCK_DGRAM;

            if (0!= (status= getaddrinfo( question.qName, NULL, &hints, &result))){
                fprintf(stderr, "getaddrinfo: %s", gai_strerror(status));
                exit(1);
            }
            resource.rType= question.qType;
            resource.rClass= question.qClass;
            resource.rTTL= 10;
            resource.rdLength= sizeof(uint32_t);
            for (ptr= result; NULL!= ptr; ptr= ptr->ai_next){
                if (AF_INET== ptr->ai_family){
                    struct sockaddr_in *ipv4= (struct sockaddr_in*)ptr->ai_addr;
                    tpaddr= &(ipv4->sin_addr);

                    inet_ntop(ptr->ai_family, tpaddr, ipaddr, sizeof(ipaddr));
                    memcpy(resource.rIp, ipaddr, sizeof(ipaddr));
                    memcpy(resource.rData, &(ipv4->sin_addr.s_addr), sizeof(uint32_t));

                    response.m_answers.push_back(resource);
                    break;
                }
            }

            freeaddrinfo(result);
        }

        /*
         * TODO
         * this current method just try to confirm that this dns proxy server
         * will works well.
         * Gonna configure an upper dns server & sending the dns query datagram to
         * achieve the responce to the query
         */

        response.m_questions.push_back(question);
    }


    response.m_id= query.getID();
    response.m_qr= true;
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
}
