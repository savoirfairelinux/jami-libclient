/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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
#include <transfer.h>

#include <private/transfer_p.h>

#include "dbus/datatransfermanager.h"

TransferPrivate::TransferPrivate(Transfer* parent) : q_ptr(parent)
{}

void TransferPrivate::updated()
{
   emit q_ptr->changed();
}

TransferPrivate::~TransferPrivate()
{

}

Transfer::Transfer(Account* a, const QString& id) : d_ptr(new TransferPrivate(this))
{
    d_ptr->m_pAccount = a;
    d_ptr->m_Id = id;
}

Transfer::~Transfer()
{
   delete d_ptr;
}

QString
Transfer::id() const
{
    return d_ptr->m_Id;
}

ContactMethod*
Transfer::contactMethod() const
{
   return d_ptr->m_pPeer;
}

QString
Transfer::filename() const
{
   return d_ptr->m_Filename;
}

Account*
Transfer::account() const
{
   return d_ptr->m_pAccount;
}

int
Transfer::status() const
{
    return d_ptr->m_Status;
}

int
Transfer::size() const
{
    return d_ptr->m_Size;
}

int
Transfer::sent() const
{
    return d_ptr->m_Sent;
}

int
Transfer::progress() const
{
    return 100.0f * d_ptr->m_Sent / d_ptr->m_Size;
}

void
Transfer::setStatus(int status)
{
    d_ptr->m_Status = (DRing::DataTransferCode) status;

    using DRing::DataTransferCode;
    switch (d_ptr->m_Status) {
        case DataTransferCode::CODE_UNKNOWN:
        // 1xx Informational
        case DataTransferCode::CODE_TRYING:
        case DataTransferCode::CODE_NOTIFYING:
        case DataTransferCode::CODE_QUEUED:
            break;
        case DataTransferCode::CODE_PROGRESSING:
            if (!d_ptr->m_pTimer) {
               d_ptr->m_pTimer = new QTimer(this);
               d_ptr->m_pTimer->setInterval(1000);
               connect(d_ptr->m_pTimer, &QTimer::timeout, d_ptr, &TransferPrivate::updated);
            }
            if (!d_ptr->m_pTimer->isActive())
                d_ptr->m_pTimer->start();
            break;

        // 2xx Success
        case DataTransferCode::CODE_OK:
        case DataTransferCode::CODE_CREATED:
        case DataTransferCode::CODE_ACCEPTED:

        // 4xx Request Error
        case DataTransferCode::CODE_BAD_REQUEST:
        case DataTransferCode::CODE_UNAUTHORIZED:
        case DataTransferCode::CODE_NOT_FOUND:

        // 5xx Process Error
        case DataTransferCode::CODE_INTERNAL:
        case DataTransferCode::CODE_NOT_IMPLEMENTED:
        case DataTransferCode::CODE_SERVICE_UNAVAILABLE:
            if (d_ptr->m_pTimer) {
                d_ptr->m_pTimer->stop();
                d_ptr->m_pTimer = nullptr;
            }
            break;
    }

    emit statusChanged(d_ptr->m_Id, d_ptr->m_Status);
}

void
Transfer::setContactMethod(ContactMethod* cm)
{
    d_ptr->m_pPeer = cm;
    emit contactChanged(d_ptr->m_Id, d_ptr->m_pPeer);
}

void
Transfer::setFilename(QString name)
{
    d_ptr->m_Filename = name;
}

void
Transfer::setSize(int size)
{
    d_ptr->m_Size = size;
}

void
Transfer::setSent(int s)
{
    d_ptr->m_Sent = s;
}

void
Transfer::cancel()
{
    auto& dataTransferManager = DataTransferManager::instance();
    dataTransferManager.cancelDataTransfer(d_ptr->m_Id);
}

void
Transfer::accept(const QString& pathname)
{
    auto& dataTransferManager = DataTransferManager::instance();
    dataTransferManager.acceptFileTransfer(d_ptr->m_Id, pathname);
}

QString
Transfer::dataTransferCodeToString()
{
    using DRing::DataTransferCode;
    switch (d_ptr->m_Status) {
        case DataTransferCode::CODE_UNKNOWN:
            return tr("Unknown");
        // 1xx Informational
        case DataTransferCode::CODE_TRYING:
            return tr("Connecting");
        case DataTransferCode::CODE_NOTIFYING:
            return tr("Notifying");
        case DataTransferCode::CODE_QUEUED:
            return tr("In queue");
        case DataTransferCode::CODE_PROGRESSING:
            return tr("Progressing");

        // 2xx Success
        case DataTransferCode::CODE_OK:
            return tr("Transfer complete");
        case DataTransferCode::CODE_CREATED:
            return tr("Transfer created");
        case DataTransferCode::CODE_ACCEPTED:
            return tr("Transfer accepted");

        // 4xx Request Error
        case DataTransferCode::CODE_BAD_REQUEST:
            return tr("Bad request");
        case DataTransferCode::CODE_UNAUTHORIZED:
            return tr("Unauthorized");
        case DataTransferCode::CODE_NOT_FOUND:
            return tr("Not found");

        // 5xx Process Error
        case DataTransferCode::CODE_INTERNAL:
            return tr("Internal error");
        case DataTransferCode::CODE_NOT_IMPLEMENTED:
            return tr("Not implemented");
        case DataTransferCode::CODE_SERVICE_UNAVAILABLE:
            return tr("Service unavailable");
        default:
            return tr("Unknown");
    }
}
