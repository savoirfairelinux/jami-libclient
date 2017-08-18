#pragma once

struct Message
{
    std::string body;
    std::string timestamp;
    bool is_outgoing;
};

typedef std::vector<Message> Messages;
