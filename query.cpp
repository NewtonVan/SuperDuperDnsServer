//
// Created by Newton on 2020/6/6.
//

#include "query.h"
#include <sstream>

using namespace dns;

/*
 * format the dns query datagram
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
    strm<<"Query:"<<std::endl;
    for (std::vector<MQuestion>::iterator iter= m_questions.begin();
    m_questions.end() != iter; ++iter){
        MQuestion question= *iter;
        strm<<question.qName<<" "<<question.qType<<std::endl;
    }

    return strm.str();
}
