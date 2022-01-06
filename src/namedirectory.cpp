/****************************************************************************
 *    Copyright (C) 2016-2022 Savoir-faire Linux Inc.                       *
 *   Author : Alexandre Viau <alexandre.viau@savoirfairelinux.com>          *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include <QCoreApplication>

#include "namedirectory.h"
#include "private/namedirectory_p.h"
#include "dbus/configurationmanager.h"

NameDirectoryPrivate::NameDirectoryPrivate(NameDirectory* q)
    : q_ptr(q)
{
    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();

    connect(&configurationManager,
            &ConfigurationManagerInterface::nameRegistrationEnded,
            this,
            &NameDirectoryPrivate::slotNameRegistrationEnded,
            Qt::QueuedConnection);
    connect(&configurationManager,
            &ConfigurationManagerInterface::registeredNameFound,
            this,
            &NameDirectoryPrivate::slotRegisteredNameFound,
            Qt::QueuedConnection);
    connect(&configurationManager,
            &ConfigurationManagerInterface::exportOnRingEnded,
            this,
            &NameDirectoryPrivate::slotExportOnRingEnded,
            Qt::QueuedConnection);
}

NameDirectory::NameDirectory()
    : QObject(QCoreApplication::instance())
    , d_ptr(new NameDirectoryPrivate(this))
{}

/// Singleton
NameDirectory&
NameDirectory::instance()
{
    static auto instance = new NameDirectory;
    return *instance;
}

// Name registration ended
void
NameDirectoryPrivate::slotNameRegistrationEnded(const QString& accountId,
                                                int status,
                                                const QString& name)
{
    qDebug() << "Name registration ended. Account:" << accountId << "status:" << status
             << "name:" << name;

    emit q_ptr->nameRegistrationEnded(static_cast<NameDirectory::RegisterNameStatus>(status), name);
}

// Registered Name found
void
NameDirectoryPrivate::slotRegisteredNameFound(const QString& accountId,
                                              int status,
                                              const QString& address,
                                              const QString& name)
{
    switch (static_cast<NameDirectory::LookupStatus>(status)) {
    case NameDirectory::LookupStatus::INVALID_NAME:
        qDebug() << "lookup name is INVALID:" << name << accountId;
        break;
    case NameDirectory::LookupStatus::NOT_FOUND:
        qDebug() << "lookup name NOT FOUND:" << name << accountId;
        break;
    case NameDirectory::LookupStatus::ERROR:
        qDebug() << "lookup name ERROR:" << name << accountId;
        break;
    case NameDirectory::LookupStatus::SUCCESS:
        break;
    }

    emit q_ptr->registeredNameFound(static_cast<NameDirectory::LookupStatus>(status), address, name);
}

// Export account has ended with pin generated
void
NameDirectoryPrivate::slotExportOnRingEnded(const QString& accountId, int status, const QString& pin)
{
    qDebug() << "Export on ring ended for account: " << accountId << "status: " << status
             << "PIN: " << pin;

    emit q_ptr->exportOnRingEnded(static_cast<NameDirectory::ExportOnRingStatus>(status), pin);
}

// Lookup a name
bool
NameDirectory::lookupName(const QString& nameServiceURL, const QString& name) const
{
    return ConfigurationManager::instance().lookupName("", nameServiceURL, name);
}

// Lookup an address
bool
NameDirectory::lookupAddress(const QString& nameServiceURL, const QString& address) const
{
    return ConfigurationManager::instance().lookupAddress("", nameServiceURL, address);
}

NameDirectory::~NameDirectory()
{
    delete d_ptr;
}
