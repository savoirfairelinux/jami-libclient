/****************************************************************************
 *    Copyright (C) 2018-2020 Savoir-faire Linux Inc.                       *
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
#include "api/collections/temporaryItem.h"

#include "api/element.h"

namespace lrc
{

namespace api
{

class TemporaryItemPimpl
{
    
};

TemporaryItem::TemporaryItem()
{
    
}

TemporaryItem::~TemporaryItem()
{
    
}

QVector<Element>
TemporaryItem::filter(const QString& search)
{
    return {};
}

} // namespace api
} // namespace lrc

#include "api/collections/moc_temporaryItem.cpp"
#include "temporaryItem.moc"
