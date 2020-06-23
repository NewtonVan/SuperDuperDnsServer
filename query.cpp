//
// Created by Newton on 2020/6/6.
//

#include "query.h"

using namespace dns;

/*
 * format the dns query datagram
 * attention that buf is just a parameter not a reference here
 */
void Query::decode(const char *buf, const int size)
{
    if (size < sizeof(MHeader)){
        return;
    }
    decode_header(buf);
    decode_questions(buf);
}

/*
 * extract the information of the query
 */
std::string Query::to_string()
{
    std::stringstream strm;
    char type[10]={0};
    strm<<"Query:"<<std::endl;
    for (std::vector<MQuestion>::iterator iter= m_questions.begin();
    m_questions.end() != iter; ++iter){
        MQuestion question= *iter;
        get_type(question.qType, type);
        strm<<question.qName<<"\t\t"<<type<<std::endl;
    }

    return strm.str();
}
