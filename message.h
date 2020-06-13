//
// Created by Newton on 2020/6/5.
//

#ifndef SUPERDUPERSERVER_MESSAGE_H
#define SUPERDUPERSERVER_MESSAGE_H

#include <iostream>
#include <vector>

namespace dns{
    class Message{
        friend class Resolver;

    private:
        // used to extract the information from flags
        static const uint16_t QR_MASK = 0x8000;
        static const uint16_t OPCODE_MASK = 0x7800;
        static const uint16_t AA_MASK = 0x0400;
        static const uint16_t TC_MASK = 0x0200;
        static const uint16_t RD_MASK = 0x0100;
        static const uint16_t RA_MASK = 0x8000;
        static const uint16_t RCODE_MASK = 0x000F;

        enum MResponseCode //the status code server response
        {
            MC_NO_ERROR = 0, //no error
            MC_FORMAT_ERROR = 1, //format error
            MC_SERVER_ERROR = 2, //server error
            MC_NAME_ERROR = 3, //name error
            MC_NOT_SUPPORT = 4, //server not support
            MC_REFUSE = 5 //server refuse

        };
        enum MQueryType{    //the resource info which query
            MT_A = 0x01,    // ipv4 addr
            MT_NS = 0x02,   // dns name server
            MT_CNAME = 0x05,    // host name
            MT_SOA = 0x06,  // the 1st auth server
            MT_WKS = 0x0B,  // well known service
            MT_PTR = 0x0C,  // reverse
            MT_HINFO = 0x0D,    // host info
            MT_MINFO = 0x0E,    // main info
            MT_MX = 0x0F,   // mail exchange
            MT_TXT = 0x10,  // text
            MT_AAAA = 0x1C, // ipv6 addr
            MT_UINFO = 0x64,    //user info
            MT_UID = 0x65,  // user id
            MT_GID = 0x66,  // group id
            MT_AXFR = 0xFC, 
            MT_ANY= 0xFF    // any type
        };

    protected:
/*
                                   1  1  1  1  1  1
     0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    header structure
    source: rfc 1035
*/
        struct MHeader{
            uint16_t hId;
            uint16_t hFlags;
            uint16_t queryCount;
            uint16_t ansCount;
            uint16_t authCount;
            uint16_t addiCount;
        };

/*
                                  1  1  1  1  1  1
    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    question structure
    source: rfc1035
*/
        struct MQuestion{
            char qName[64];
            uint16_t qType;
            uint16_t qClass;
        };

/*
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                                               /
    /                      NAME                     /
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    resource structure
    source rfc1035
*/
        struct MResource{
            char rName[64];
            uint16_t rType;
            uint16_t rClass;
            uint32_t rTTL;
            uint16_t rdLength;
            char rData[64];
            char rIp[64];  // this is setted to be human readable term
        };

        // header part assist
        unsigned int m_id;
        bool m_qr;
        unsigned int m_opcode;
        unsigned int m_aa;
        unsigned int m_tc;
        unsigned int m_rd;
        unsigned int m_ra;
        unsigned int m_rcode;

        unsigned int m_qdCount;  // the number of entries in the question section.
        unsigned int m_anCount;  // the number of resource records in the answer section.
        unsigned int m_nsCount;  // the number of name server resource records in the authority records
        unsigned int m_arCount;  // the number of resource records in the additional records section.

        // questions part
        std::vector<MQuestion> m_questions;
        //resource part
        std::vector<MResource> m_answers;

        /*functions*/
        void encode_header(char *&);
        void encode_questions(char *&);
        void encode_answers(char *&);
        void decode_header(const char*&);
        void decode_questions(const char*&);
    public:
        Message(){};
        virtual ~Message() {};

        /*functions*/
        // get header's information
        unsigned getID() const
        {
            return m_id;
        }
        unsigned getOpcode() const
        {
            return m_opcode;
        }
        unsigned getQdCount() const
        {
            return m_qdCount;
        }
        unsigned getAnCount() const
        {
            return m_anCount;
        }
        unsigned getNsCount() const
        {
            return m_nsCount;
        }
        unsigned getArCount() const
        {
            return m_arCount;
        }
        // get questions
        std::vector<MQuestion> getQuestions() const
        {
            return m_questions;
        }
    };
}
#endif //SUPERDUPERSERVER_MESSAGE_H
