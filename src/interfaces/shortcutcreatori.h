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
#ifndef SHORTCUTCREATORI_H
#define SHORTCUTCREATORI_H

#include <typedefs.h>

//Qt
// #include <QtCore/QVariant>
class QVariant;

//Ring
class Macro;

namespace Interfaces {

/**
 * Interface for attaching a QAction/GAction/MASShortcut to a Macro
 */
class LIB_EXPORT ShortcutCreatorI {
public:
    virtual ~ShortcutCreatorI(){}

    virtual QVariant createAction(Macro* macro) = 0;
};

} // namespace Interfaces

#endif // SHORTCUTCREATORI_H
