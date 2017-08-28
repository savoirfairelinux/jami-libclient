/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
#pragma once

// Std
#include <memory>

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"

namespace lrc
{

namespace api
{
class Lrc;
}

class CallbacksHandler : public QObject {
    Q_OBJECT

public:
    CallbacksHandler(const api::Lrc& parent);
    ~CallbacksHandler();

Q_SIGNALS:
    void NewBuddySubscription(const std::string& contactUri);
    void contactRemoved(const std::string& accountId, const std::string& contactUri, bool banned) const;
    void contactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed) const;

private Q_SLOTS:
    void slotNewAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads);
    /**
     * Update the presence of a contact for an account
     * @param accountId
     * @param contactUri
     * @param status if the contact is present
     * @param message unused for now
     */
    void slotNewBuddySubscription(const QString& accountId, const QString& contactUri, bool status, const QString& message);
    /**
     * Add a contact in the contact list of an account
     * @param accountId
     * @param contactUri
     * @param confirmed
     */
    void slotContactAdded(const QString& accountId, const QString& contactUri, bool confirmed);
    /**
     * Remove a contact from a contact list of an account
     * @param accountId
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const QString& accountId, const QString& contactUri, bool banned);

private:
    const api::Lrc& parent;
};

} // namespace lrc
