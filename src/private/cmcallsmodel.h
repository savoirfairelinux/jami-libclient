/************************************************************************************
 *   Copyright (C) 2017 by BlueSystems GmbH                                         *
 *   Author : Emmanuel Lepage Vallee <elv1313@gmail.com>                            *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/
#pragma once

#include <QtCore/QAbstractListModel>

class ContactMethod;

/**
 * Simple class to access the calls of a ContactMethods as a model. It doesn't
 * implement sorting or any complex operations. If sorting is required, a proxy
 * is the way to go.
 *
 * This model is useful, for example, to build a "view contact" page with all
 * the metadata provided by LibRingClient.
 */
class CMCallsModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit CMCallsModel(const ContactMethod* parent);
    virtual ~CMCallsModel();

    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    virtual int rowCount( const QModelIndex& parent = {} ) const override;
    virtual QHash<int,QByteArray> roleNames() const override;

private:
    // This is a private class, no need for d_ptr
    const ContactMethod* m_pCM;
};
