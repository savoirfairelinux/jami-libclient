#pragma once
#include "message.h"
#include "callinfo.h"
#include "contactinfo.h"

struct Conversation
{
    const std::string uid_= "";
    std::vector <ContactInfo> participants_;
    CallInfo call_;
    Messages messages_;
    bool isUsed_ = false;
};
