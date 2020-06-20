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
    // use o_buff, because there exist name pointer problem
    decode_answers(buff, o_buff);
}

std::string ToolMessage::to_string()
{
    std::stringstream sstrm;
    sstrm<<"ToolMessage:"<<std::endl;

    for (std::vector<MQuestion>::iterator iter= m_questions.begin();
         m_questions.end() != iter; ++iter){
        MQuestion question= *iter;
        sstrm<<question.qName<<" "<<question.qType<<std::endl;
    }
}