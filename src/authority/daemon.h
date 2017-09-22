#pragma once

// Lrc
#include "api/account.h"
#include "api/contact.h"
#include "dbus/configurationmanager.h"

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
    QString(contactInfo.uri.c_str()));
    // NOTE: [sb] Incorrect, should be when daemon emit slotContactAdded
    contactInfo.type = api::contact::Type::RING;
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
