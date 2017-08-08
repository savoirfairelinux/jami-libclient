#pragma once

struct Contact
{
    std::string uri;
    std::string id;
    std::string registeredName;
    std::string displayName;
    std::string avatar;
    bool isPresent;
    unsigned int unreadMessages;
    //TODO add avatar + lastUsed
};
