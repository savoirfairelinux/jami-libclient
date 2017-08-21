#pragma once

struct Message
{
    std::string uid_;
    std::string body_;
    long timestamp_;
    bool isOutgoing_;
    // TODO add type
};

typedef std::map<std::string, Message> Messages;
