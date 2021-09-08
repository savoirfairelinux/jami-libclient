/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

#pragma once

#include <QImage>
#include <QObject>
#include <QQuickImageProvider>

class LRCInstance;

class QuickImageProviderBase : public QQuickImageProvider
{
public:
    QuickImageProviderBase(QQuickImageProvider::ImageType type,
                           QQmlImageProviderBase::Flag flag,
                           LRCInstance* instance = nullptr)
        : QQuickImageProvider(type, flag)
        , lrcInstance_(instance)
    {}

protected:
    // LRCInstance pointer
    LRCInstance* lrcInstance_ {nullptr};
};
