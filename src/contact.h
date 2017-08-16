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

    // constructor
    Contact() {};

    // copy constructor
    Contact(const Contact& c) : uri(c.uri),
                                id(c.id),
                                registeredName(c.registeredName),
                                displayName(c.displayName),
                                avatar(c.avatar),
                                isPresent(c.isPresent),
                                unreadMessages(c.unreadMessages) {};

};
