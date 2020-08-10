/**
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos   <aline.gondimsantos@savoirfairelinux.com>
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

#include "qmladapterbase.h"
//#include "smartlistmodel.h"
#include "mediahandleritemlistmodel.h"

#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>

class MediaHandlerAdapter : public QmlAdapterBase
{
    Q_OBJECT

public:
    explicit MediaHandlerAdapter(QObject *parent = nullptr);
    ~MediaHandlerAdapter();

    Q_INVOKABLE QVariant getMediaHandlerSelectableModel();

private:
    void initQmlObject();

    std::unique_ptr<MediaHandlerItemListModel> mediaHandlerListModel_;

};
