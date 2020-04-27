/****************************************************************************
 *    Copyright (C) 2019-2020 Savoir-faire Linux Inc.                       *
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

// Lrc
#include "typedefs.h"

// Qt
#include <QObject>
#include <QMap>
#include <QString>
#include <QVector>

namespace lrc
{

namespace api
{

/**
 *  @brief Represent a collection's element
 */
struct LIB_EXPORT Element {
    QString uri {};
    QString avatar {};
    QString title {};
    QString subtitle {};
    QString text {};
    int notifications {0};
};

} // namespace api
} // namespace lrc
