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
#include "cmcallsmodel.h"

// Ring
#include <contactmethod.h>
#include <categorizedhistorymodel.h>
#include <call.h>

CMCallsModel::CMCallsModel(const ContactMethod* parent) :
    QAbstractListModel(const_cast<ContactMethod*>(parent)), m_pCM(parent)
{
    connect(parent, &ContactMethod::callAdded, [this]() {
        beginInsertRows({}, m_pCM->calls().size()-1, m_pCM->calls().size()-1);
        endInsertRows();
    });
}

CMCallsModel::~CMCallsModel()
{}

QVariant CMCallsModel::data( const QModelIndex& index, int role ) const
{
    return index.isValid() ? m_pCM->calls()[index.row()]->roleData(role) : QVariant();
}

int CMCallsModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : m_pCM->calls().size();
}

QHash<int,QByteArray> CMCallsModel::roleNames() const
{
    return CategorizedHistoryModel::instance().roleNames();
}
