//
// Created by Newton on 2020/6/14.
//

#include "toolMessage.h"

using namespace dns;

/*
 * same as the encode handle in 'Response' class
 */
int ToolMessage::encodeTool(char *buff)
{
    char *start= buff;

    encode_header(buff);
    encode_questions(buff);
    encode_answers(buff);

    int len= int(buff-start);

    return len;
}

void ToolMessage::decodeTool(const char *buff, const int size)
{
    if (size< sizeof(MHeader)){
        return;
    }
    const char *o_buff= buff;
    decode_header(buff);
    decode_questions(buff);
    // TODO check segment fault
//    printf("segment fault test point 4\n");
    // use o_buff, because there exist name pointer problem
    decode_answers(buff, o_buff);
    // TODO check segment fault
//    printf("segment fault test point 5\n");
}

std::string ToolMessage::to_string()
{
    std::stringstream sstrm;
    char type[10]={0};
    sstrm<<"\nResponse:"<<std::endl;
    sstrm<<"Status:"<<m_rcode<<std::endl;

    for (std::vector<MResource>::iterator iter= m_answers.begin();
         m_answers.end()!= iter; ++iter){
        MResource resource= *iter;
        get_type(resource.rType, type);
        sstrm<<resource.rIp<<"\t\t"<<type<<std::endl;
    }
    sstrm<<"\n";
    return sstrm.str();
}