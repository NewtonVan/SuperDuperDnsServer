//
// Created by Newton on 2020/6/5.
//
#include "message.h"
#include "SuperDuperLib.h"


using namespace dns;
using namespace std;
/*
 * convert '.' separation to number separation
 * assist for the following function
 * set the pointer point to the byte after the end
 */
void encode_address(char *addr, char *&buff)
{
    string address(addr);
    int pos, curt = 0, sz;

    while (string::npos != (pos = (int)address.find('.'))){
        address.erase(0, pos+1);
        *buff++ = pos;
        memcpy(buff, addr+curt, pos);
        buff += pos;
        curt += pos+1;
    }
    *buff++ = sz = address.size();
    memcpy(buff, addr+curt, size_t(sz));
    buff += sz;

    *buff++ = '\0';
}

/*
 * push formative header part assist into header structure
 * push the formative header structure into the 'buff', and
 * set the pointer to the byte after the end
 * header is a temporary variable, it doesn't change anything
 */
void Message::encode_header(char *&buff)
{
    MHeader header={0};

    header.hId = m_id;
    header.hFlags += ((m_qr ? 1 : 0) << 15);
    header.hFlags += (m_opcode << 11);
    header.hFlags += (m_aa << 10);
    header.hFlags += (m_tc << 9);
    header.hFlags += (m_rd << 8);
    header.hFlags += (m_ra << 7);
    header.hFlags += m_rcode;

    header.queryCount = m_qdCount;
    header.ansCount = m_anCount;
    header.authCount = m_nsCount;
    header.addiCount = m_arCount;

    // convert the header part to network byte order
    header.hId = htons(header.hId);
    header.hFlags = htons(header.hFlags);
    header.queryCount= htons(header.queryCount);
    header.ansCount= htons(header.ansCount);
    header.authCount= htons(header.authCount);
    header.addiCount= htons(header.addiCount);

    memcpy(buff, &header, sizeof(MHeader));
    buff += sizeof(MHeader);
}

/*
 * push the formative question structure into the 'buff' &
 * set the pointer to the byte after the end
 */
void Message::encode_questions(char *&buff)
{
    size_t sz16= sizeof(uint16_t);
    for (int i= 0; i< m_qdCount; ++i){
        MQuestion question= m_questions[i];
        encode_address(question.qName, buff);

        // convert qtype and qclass to network byte order
        uint16_t nQType = htons(question.qType);
        memcpy(buff, &nQType, sz16);
        buff+= sz16;

        uint16_t nQClass =  htons(question.qClass);
        memcpy(buff, &nQClass, sz16);
        buff+= sz16;
    }
}

/*
 * push the formative answer stucture into the 'buff' &
 * set the pointer to the byte after the end
 */
void Message::encode_answers(char *&buff)
{
    size_t sz16= sizeof(uint16_t);
    size_t sz32= sizeof(uint32_t);
    for (int i= 0; i< m_anCount; ++i){
        MResource resource= m_answers[i];
        encode_address(resource.rName, buff);

        uint16_t nRType= htons(resource.rType);
        memcpy(buff, &nRType, sz16);
        buff+= sz16;

        uint16_t nRClass= htons(resource.rClass);
        memcpy(buff, &nRClass, sz16);
        buff+= sz16;

        uint32_t nTTL= htonl(resource.rTTL);
        memcpy(buff, &nTTL, sz32);
        buff+= sz32;

        uint16_t nRDLen= htons(resource.rdLength);
        memcpy(buff, &nRDLen, sz16);
        buff+= sz16;

        if (MT_A == resource.rType){
            memcpy(buff, resource.rData, sz32);
            buff+= sz32;
        }
        else if (MT_AAAA == resource.rType){
            // not quite sure about in6_addr here
            memcpy(buff, resource.rData, sizeof(struct in6_addr));
            buff+= sizeof(struct in6_addr);
        }
        else{
            memcpy(buff, resource.rData, resource.rdLength);
            buff+= resource.rdLength;
        }
    }
}

/*
 * transfer the data contained in the buff to formative form
 * push the formative data into header structure
 * distract the information from the header structure to the header assist part
 * set the pointer to the byte after the end
 */
void Message::decode_header(const char *&buff)
{
    MHeader header;
    memcpy(&header, buff, sizeof(MHeader));

    // convert network byte order to host byte order
    header.hId= ntohs(header.hId);
    header.hFlags= ntohs(header.hFlags);
    header.queryCount= ntohs(header.queryCount);
    header.ansCount= ntohs(header.ansCount);
    header.authCount= ntohs(header.authCount);
    header.addiCount= ntohs(header.addiCount);

    m_id= header.hId;

    // extract flags
    m_qr= header.hFlags & QR_MASK;
    m_opcode= header.hFlags & OPCODE_MASK;
    m_aa= header.hFlags & AA_MASK;
    m_tc= header.hFlags & TC_MASK;
    m_rd= header.hFlags & RD_MASK;
    m_ra= header.hFlags & RA_MASK;
    m_rcode= header.hFlags & RCODE_MASK;

    m_qdCount= header.queryCount;
    m_anCount= header.ansCount;
    m_nsCount= header.authCount;
    m_arCount= header.addiCount;

    buff+= sizeof(MHeader);
}

/*
 * TODO maybe handle the name server pointer problem
 * transfer the query data to formative data
 * push the formative data into m_questions which record the questions
 * set the pointer to the byte after the end
 */
void Message::decode_questions(const char *&buff)
{
    m_questions.clear();
    size_t sz16= sizeof(uint16_t);
    for (int i= 0; i< m_qdCount; ++i){
        MQuestion question= {0};
        while (1){
            unsigned int len= *buff++;
            if (0== len){
                break;
            }
            if (0!= strlen(question.qName)){
                strcat(question.qName, ".");
            }
            memcpy(question.qName+strlen(question.qName), buff, len);
            buff+= len;
        }
        question.qType= ntohs(*((uint16_t *)buff));
        buff+= sz16;

        question.qClass= ntohs(*((uint16_t *)buff));
        buff+= sz16;

        m_questions.push_back(question);
    }
}


/*
 * TODO
 * handle the name server problem
 */
void Message::decode_answers(const char *&buff, const char *&obuf)
{
    m_answers.clear();
    size_t sz16= sizeof(uint16_t);
    size_t sz32= sizeof(uint32_t);

    for (int i= 0; i< m_anCount; ++i){
        MResource resource= {0};
        while (1){
            unsigned int len= (unsigned char)(*buff++);

            // handle the name pointer
            if (0xC0 == len){ // name pointer
                unsigned int pos= (unsigned char)(*buff++);
                const char *nmAddr= obuf+pos;

                while (1){
                    unsigned int nmL= (unsigned char)(*nmAddr++);
                    if (0== nmL){
                        break;
                    }
                    if (0!= strlen(resource.rName)){
                        strcat(resource.rName, ".");
                    }
                    memcpy(resource.rName+strlen(resource.rName), nmAddr, nmL);
                    nmAddr+= nmL;
                }

                break;
            }

            if (0== len){
                break;
            }
            if (0!= strlen(resource.rName)){
                strcat(resource.rName, ".");
            }

            memcpy(resource.rName+strlen(resource.rName), buff, len);
            buff+= len;
        }

        resource.rType= ntohs(*((uint16_t*)buff));
        buff+= sz16;

        resource.rClass= ntohs(*((uint16_t*)buff));
        buff+= sz16;

        resource.rTTL= ntohl(*((uint32_t*)buff));
        buff+= sz32;

        resource.rdLength= ntohs(*((uint16_t*)buff));
        buff+= sz16;

        if (MT_A == resource.rType){
            // resource.rData= *((uint32_t*)buff);
            memcpy(resource.rData, buff, sz32);
            inet_ntop(AF_INET, resource.rData, resource.rIp, INET_ADDRSTRLEN);
            buff+= sz32;
        }
        else if (MT_AAAA== resource.rType){
            size_t in6_lth= sizeof(struct in6_addr);
            memcpy(resource.rData, buff, in6_lth);
            inet_ntop(AF_INET6, resource.rData, resource.rIp, INET6_ADDRSTRLEN);
            buff+= in6_lth;
        }
        else{
            size_t dlth= (size_t)resource.rdLength;
            memcpy(resource.rData, buff, dlth);
            buff+= dlth;
            memset(resource.rIp, 0, sizeof(resource.rIp));
        }

        m_answers.push_back(resource);
    }

}

void Message::get_type(unsigned int type, char *result)
{
    switch(type){
        case MT_A:
            strncpy(result, "A", 1);
            break;
        case MT_NS:
            strncpy(result, "NS", 2);
            break;
        case MT_CNAME:
            strncpy(result, "CNAME", 5);
            break;
        case MT_SOA:
            strncpy(result, "SOA", 3);
            break;
        case MT_WKS:
            strncpy(result, "WKS", 3);
            break;
        case MT_PTR:
            strncpy(result, "PTR", 3);
            break;
        case MT_HINFO:
            strncpy(result, "HINFO", 5);
            break;
        case MT_MINFO:
            strncpy(result, "MINFO", 5);
            break;
        case MT_MX:
            strncpy(result, "MX", 2);
            break;
        case MT_TXT:
            strncpy(result, "TXT", 3);
            break;
        case MT_AAAA:
            strncpy(result, "AAAA", 4);
            break;
        case MT_UINFO:
            strncpy(result, "UINFO", 5);
            break;
        case MT_UID:
            strncpy(result, "UID", 3);
            break;
        case MT_GID:
            strncpy(result, "GID", 3);
            break;
        case MT_AXFR:
            strncpy(result, "AXFR", 4);
            break;
        case MT_ANY:
            strncpy(result, "ANY", 3);
            break;
        default:
            memset(result, 0, 10);
            break;
    }
}