#include "database.h"

Database::Database(QObject* parent)
: QObject(parent)
{

}

Database::~Database()
{

}

void
Database::addMessage(const std::string& account, const std::string& uid, const std::string& body, const long timestamp, const bool isOutgoing)
{

}

void
Database::removeHistory(const std::string& account, const std::string& uid)
{

}

Messages
Database::getMessages(const std::string& account, const std::string& uid) const
{
    return Messages();
}

unsigned int
Database::NumberOfUnreads(const std::string& account, const std::string& uid) const
{
    return 0;
}

void
Database::setMessageRead(const int uid)
{

}

void
Database::addContact(const std::string& contact, const QByteArray& payload)
{

}

std::string
Database::getUri(const std::string& uid) const
{
    return "";
}

std::string
Database::getAlias(const std::string& uid) const
{
    return "";
}

std::string
Database::getAvatar(const std::string& uid) const
{
    return "";
}
