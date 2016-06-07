/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
 *   Author: Olivier Grégoire <olivier.gregoire@savoirfairelinux.com>       *
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

/* widget_p.h (_p means private) */
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QMimeData>
#include <QtCore/QItemSelectionModel>
#include <QObject>
#include <QtCore/QUrl>
#include <QList>
#include "../smartInfoHub.h"
#include "typedefs.h"

#pragma once

class SmartInfoHubPrivate;
class SmartInfoHubPrivate final : public QObject
{
    Q_OBJECT

public:
    int m_refreshTimeInformationMS =500;
    QMap<QString, QString> m_information;
    const QString m_DEFAULT_RETURN_VALUE_QSTRING  = "void";
    QList<QString> m_keyInfo;

    void setMapInfo(QMap<QString, QString> info);
    void launchSmartInfo(int refreshTimeMS);


public slots:
    void slotSmartInfo(MapStringString);
};
