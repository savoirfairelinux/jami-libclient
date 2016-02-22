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

struct TransferPrivate
{
    Account*                    m_pAccount {nullptr};
    ContactMethod*              m_pPeer {nullptr};
    QString                     m_Id;
    QString                     m_Filename;
    int                         m_Size;
    int                         m_Progress;
    DRing::DataTransferCode     m_Status;
};

Transfer::Transfer(Account* a, const QString& id) : d_ptr(new TransferPrivate())
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

DRing::DataTransferCode
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
Transfer::progress() const
{
    return d_ptr->m_Progress;
}

void
Transfer::setStatus(DRing::DataTransferCode status)
{
    d_ptr->m_Status = status;
    emit changed();
    emit statusChanged(d_ptr->m_Id, d_ptr->m_Status);
}
void
Transfer::setContactMethod(ContactMethod* cm)
{
    d_ptr->m_pPeer = cm;
    emit changed();
    emit contactChanged(d_ptr->m_Id, d_ptr->m_pPeer);
}

void
Transfer::setFilename(QString name)
{
    d_ptr->m_Filename = name;
    emit changed();
}

void
Transfer::setSize(int size)
{
    d_ptr->m_Size = size;
    emit changed();
}

void
Transfer::setProgress(int p)
{
    d_ptr->m_Progress = p;
    emit changed();
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
