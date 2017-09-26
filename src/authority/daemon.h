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
/**
 * Ask the daemon to add contact to the daemon.
 * @param owner
 * @param contactUri
 */
void addContact(const api::account::Info& owner, const std::string& contactUri);
/**
 * Ask the daemon to add contact to the daemon.
 * @param owner
 * @param contactInfo
 */
void addContact(const api::account::Info& owner, api::contact::Info& contactInfo);
/**
 * Ask the daemon to remove a contact and may ban it.
 * @param owner
 * @param contactInfo
 * @param banned
 */
void removeContact(const api::account::Info& owner, const std::string& contactUri, bool banned);
/**
 * Ask the daemon to add a contact from the pending list.
 * @param owner
 * @param contactUri
 */
void addContactFromPending(const api::account::Info& owner, const std::string& contactUri);
/**
 * Ask the daemon to discard a pending.
 * @param owner
 * @param contactUri
 */
void discardFromPending(const api::account::Info& owner, const std::string& contactUri);

} // namespace daemon

} // namespace authority

} // namespace lrc
