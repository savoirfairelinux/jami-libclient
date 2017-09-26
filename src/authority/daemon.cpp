// Lrc
#include "daemon.h"

namespace lrc
{

namespace authority
{

namespace daemon
{

void addContact(const api::account::Info& owner, const std::string& uri)
{
    ConfigurationManager::instance().addContact(QString(owner.id.c_str()),
    QString(uri.c_str()));
}

void addToContacts(const api::account::Info& owner, api::contact::Info& contactInfo)
{
    ConfigurationManager::instance().addContact(QString(owner.id.c_str()),
    QString(contactInfo.profileInfo.uri.c_str()));
}

void removeContact(const api::account::Info& owner, const std::string& contactUri, bool banned)
{
    ConfigurationManager::instance().removeContact(QString(owner.id.c_str()),
    QString(contactUri.c_str()), banned);
}

void addContactFromPending(const api::account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().acceptTrustRequest(QString(owner.id.c_str()),
    QString(contactUri.c_str()));
}

void discardFromPending(const api::account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().discardTrustRequest(
        QString(owner.id.c_str()),
        QString(contactUri.c_str())
    );
}

} // namespace daemon

} // namespace authority

} // namespace lrc
