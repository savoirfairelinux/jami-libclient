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
#include "api/collection.h"

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

class PeerDiscoveryPimpl;

/**
 *  @brief Represent local peer contacts
 */
class LIB_EXPORT PeerDiscovery : public Collection {
    Q_OBJECT
public:

    PeerDiscovery();
    ~PeerDiscovery();

    QVector<Element> filter(const QString& search) override;

Q_SIGNALS:
    /**
     * Connect this signal to know when a collection changed
     */
    void collectionChanged();

private:
    std::unique_ptr<PeerDiscoveryPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
