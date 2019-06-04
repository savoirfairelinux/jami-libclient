/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                       *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"


// Qt
#include <QEventLoop>

class Daemon : public QObject {
    Q_OBJECT

public:
    Daemon();
    ~Daemon();

Q_SIGNALS:
    /**
     * Connect this signal to know when the account details have changed
     * @param accountId the one who changes
     * @param details the new details
     */
    void accountDetailsChanged(const std::string& accountId,
                               const std::map<std::string,std::string>& details);
    /**
     * Connect this signal to know when the accounts list changed
     */
    void accountsChanged();

private Q_SLOTS:
    /**
     * Emit accountDetailsChanged
     * @param accountId
     * @param details
     */
    void slotAccountDetailsChanged(const QString& accountId,
                                   const MapStringString& details);
    /**
     * Emit accountsChanged
     */
    void slotAccountsChanged();


private:
    QEventLoop eventLoop_;

};
