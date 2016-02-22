/****************************************************************************
 *   Copyright (C) 2009-2016 by Savoir-faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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

#include <QtCore/QObject>
#include <QtCore/QTimer>


#include "transfer.h"

//Ring

class ContactMethod;
class Account;

class TransferPrivate : public QObject
{
    Q_OBJECT
public:
    friend class Call;
    Account*                    m_pAccount {nullptr};
    ContactMethod*              m_pPeer {nullptr};
    QString                     m_Id;
    QString                     m_Filename;
    int                         m_Size;
    int                         m_Sent;
    int                         m_Status;
    QTimer*                     m_pTimer {nullptr};

    explicit TransferPrivate(Transfer* parent);
    ~TransferPrivate();

    private:
        Transfer* q_ptr;

    public Q_SLOTS:
        void updated();
};
