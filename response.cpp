//
// Created by Newton on 2020/6/6.
//

#include <sstream>
#include "response.h"

using namespace dns;
/*
 * format the answer to response dns datagram
 * integrate the encode function in message class
 */
int Response::encode(char *buff)
{
    char *start= buff;
    encode_header(buff);
    encode_questions(buff);
    encode_answers(buff);

    int len= (int)(buff-start);

    return len;
}

/*
 * extract the remind information
 */
std::string Response::to_string()
{
    std::stringstream stream;
    stream<<"Response:"<<std::endl;
    stream<<"Status:"<<m_rcode<<std::endl;

    for (std::vector<MResource>::iterator iter= m_answers.begin();
    m_answers.end()!= iter; ++iter){
        MResource resource= *iter;
        stream<<resource.rIp<<" "<<resource.rType<<std::endl;
    }

    return stream.str();
}