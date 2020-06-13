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

    header.hId = htons(header.hId);
    header.hFlags = htons(header.hFlags);
    header.queryCount = m_qdCount;
    header.ansCount = m_anCount;
    header.authCount = m_nsCount;
    header.addiCount = m_arCount;

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

        uint32_t nTTL= htons(resource.rTTL);
        memcpy(buff, &nTTL, sz32);
        buff+= sz32;

        uint16_t nRDLen= htons(resource.rdLength);
        memcpy(buff, &nRDLen, sz16);
        buff+= sz16;

        if (MT_A == resource.rType){
            memcpy(buff, resource.rData, sz32);
            buff+= sz32;
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