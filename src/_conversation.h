#pragma once
#include "_message.h"
#include "_callinfo.h"
#include "_contactinfo.h"

typedef std::map<std::string, Message> Messages;

struct Conversation
{
    const std::string uid_= "";
    std::vector <ContactInfo> participants_;
    CallInfo call_;
    Messages messages_;
    bool isUsed_ = false;
};
