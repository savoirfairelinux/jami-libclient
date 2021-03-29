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

#include <QAbstractItemModel>

#include "lrcinstance.h"

class AbstractListModelBase : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(LRCInstance* lrcInstance MEMBER lrcInstance_ NOTIFY lrcInstanceChanged)

public:
    explicit AbstractListModelBase(QObject* parent = nullptr)
        : QAbstractListModel(parent) {};
    ~AbstractListModelBase() = default;

Q_SIGNALS:
    void lrcInstanceChanged();

protected:
    // LRCInstance pointer (set in qml)
    LRCInstance* lrcInstance_ {nullptr};
};
