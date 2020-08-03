/*
 * Copyright (C) 2017-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QColor>

#pragma once

namespace JamiAvatarTheme {
static const QColor defaultAvatarColor_ = {"#ff9e9e9e"}; //Grey
static const QColor avatarColors_[]{
    {"#fff44336"}, //Red
    {"#ffe91e63"}, //Pink
    {"#ff9c27b0"}, //Purple
    {"#ff673ab7"}, //Deep Purple
    {"#ff3f51b5"}, //Indigo
    {"#ff2196f3"}, //Blue
    {"#ff00bcd4"}, //Cyan
    {"#ff009688"}, //Teal
    {"#ff4caf50"}, //Green
    {"#ff8bc34a"}, //Light Green
    {"#ff9e9e9e"}, //Grey
    {"#ffcddc39"}, //Lime
    {"#ffffc107"}, //Amber
    {"#ffff5722"}, //Deep Orange
    {"#ff795548"}, //Brown
    {"#ff607d8b"}  //Blue Grey
};
} // namespace JamiAvatarTheme
