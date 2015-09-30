/****************************************************************************
 *   Copyright (C) 2015 by Savoir-faire Linux                               *
 *   Author : Stepan Salenikovich <stepan.salenikovich@savoirfairelinux.com>*
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

namespace Ring {

/**
 * The purpose of this enum class is to mimic/extend the Qt::ItemDataRole in LRC so that the same
 * value is used when using a common role in the ::data() method of any model in LRC,
 * eg: the value of the Object role should not be different for the PersonModel and the CallModel.
 *
 * This is so that clients of LRC do need additional logic when trying to extract the same type of
 * data from multiple types of LRC models.
 *
 * Thus any data role which is common to multiple models in LRC should be defined here. Data roles
 * which are specific to the model can be defined within that model only and their value should
 * start with UserRole + 1
 */

enum class Role
{
    DisplayRole        = Qt::DisplayRole ,
    Object             = Qt::UserRole + 1,
    Name               ,
    Number             ,
    LastUsed           ,
    FormattedLastUsed  ,
    State              ,
    FormattedState     ,
    DropState          ,
    UserRole           = Qt::UserRole + 100  // this should always be the last role in the list
};
} // namespace Ring
