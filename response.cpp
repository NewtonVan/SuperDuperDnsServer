//
// Created by Newton on 2020/6/6.
//


#include "response.h"

using namespace dns;
/*
 * format the answer to response dns datagram
 * integrate the encode function in message class
 * remember that buff is just a parameter and not a reference here
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
    char type[10]={0};
    stream<<"\nResponse:"<<std::endl;
    stream<<"Status:"<<m_rcode<<std::endl;

    for (std::vector<MResource>::iterator iter= m_answers.begin();
    m_answers.end()!= iter; ++iter){
        MResource resource= *iter;
        get_type(resource.rType, type);
        stream<<resource.rIp<<"\r\t\t\t\t"<<type<<std::endl;
    }
    stream<<"\n";
    return stream.str();
}