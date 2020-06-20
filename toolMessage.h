//
// Created by Newton on 2020/6/14.
//

#ifndef SUPERDUPERSERVER_TOOLMESSAGE_H
#define SUPERDUPERSERVER_TOOLMESSAGE_H

#include "message.h"

namespace dns{
    /*
     * child class of Message
     * created for exchange message with the upper proxy server
     */
    class ToolMessage : public Message{
    public:
        ToolMessage(){};
        virtual ~ToolMessage(){};

        // functions
        int encodeTool(char *);
        // void decode_answers(const char *&);
        void decodeTool(const char *, const int);
        std::string to_string();
    };
}
#endif //SUPERDUPERSERVER_TOOLMESSAGE_H
