/****************************************************************************
 *   Copyright (C) 2015 by Savoir-faire Linux                               *
 *   Author : Emmanuel Lepage <emmanuel.lepage@savoirfairelinux.com>        *
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

#include <typedefs.h>

//Qt
#include <QtCore/QVariant>

//Ring
#include "interfaces/shortcutcreatori.h"
class Macro;

namespace Interfaces {

///Default implementation of the the ShortcutCreator interfaces; does nothing
class LIB_EXPORT ShortcutCreatorDefault : public ShortcutCreatorI {
public:
    QVariant createAction(Macro* macro) override;
};

} // namespace Interfaces
