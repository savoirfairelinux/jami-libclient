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

void addContact(const api::account::Info& owner, const std::string& uri);

void addToContacts(const api::account::Info& owner, api::contact::Info& contactInfo);

void removeContact(const api::account::Info& owner, const std::string& contactUri, bool banned);

void addContactFromPending(const api::account::Info& owner, const std::string& contactUri);

void discardFromPending(const api::account::Info& owner, const std::string& contactUri);

} // namespace daemon

} // namespace authority

} // namespace lrc
