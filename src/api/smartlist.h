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

// std
#include <memory>

namespace lrc
{

namespace api
{

struct Element;
class Collection;

typedef QMap<QString, QVector<std::shared_ptr<Element>>> FilteredCollections;
class SmartListPimpl;

/**
 *  @brief Class that manages the smartlist
 */
class LIB_EXPORT SmartList : public QObject {
    Q_OBJECT
public:

    SmartList();
    ~SmartList();

    QMap<QString, QMap<QString, std::shared_ptr<Collection>>> list() const;

    void filter(const QString& category, const QString& filter);


Q_SIGNALS:
    /**
     * Connect this signal to know when a collection changed
     */
    void collectionChanged(const QString& collectionId);

private:
    std::unique_ptr<SmartListPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
