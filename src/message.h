#pragma once

struct Message
{
    std::string uid_;
    std::string body_;
    long timestamp_;
    bool isOutgoing_;
};

typedef std::map<std::string, Message> Messages;
