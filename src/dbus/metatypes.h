/******************************************************************************
 *   Copyright (C) 2009-2018 Savoir-faire Linux                                 *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>   *
 *            Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>            *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Lesser General Public               *
 *   License as published by the Free Software Foundation; either             *
 *   version 2.1 of the License, or (at your option) any later version.       *
 *                                                                            *
 *   This library is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *   Lesser General Public License for more details.                          *
 *                                                                            *
 *   You should have received a copy of the Lesser GNU General Public License *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/
#pragma once

#include <QtCore/QMetaType>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QString>

#include "../typedefs.h"

#ifndef ENABLE_LIBWRAP
#include <QtDBus/QtDBus>
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

Q_DECLARE_METATYPE(MapStringString)
Q_DECLARE_METATYPE(MapStringInt)
Q_DECLARE_METATYPE(VectorMapStringString)
Q_DECLARE_METATYPE(MapStringMapStringStringList)
Q_DECLARE_METATYPE(VectorInt)
Q_DECLARE_METATYPE(VectorUInt)
Q_DECLARE_METATYPE(VectorULongLong)
Q_DECLARE_METATYPE(VectorString)
Q_DECLARE_METATYPE(MapStringVectorString)
Q_DECLARE_METATYPE(VectorVectorByte)
Q_DECLARE_METATYPE(DataTransferInfo)
Q_DECLARE_METATYPE(Message)

#ifndef ENABLE_LIBWRAP
static inline QDBusArgument &operator<<(QDBusArgument& argument, const DataTransferInfo& info)
{
    argument.beginStructure();
    argument << info.accountId;
    argument << info.lastEvent;
    argument << info.flags;
    argument << info.totalSize;
    argument << info.bytesProgress;
    argument << info.peer;
    argument << info.displayName;
    argument << info.path;
    argument << info.mimetype;
    argument.endStructure();

    return argument;
}

static inline const QDBusArgument &operator>>(const QDBusArgument& argument, DataTransferInfo& info)
{
    argument.beginStructure();
    argument >> info.accountId;
    argument >> info.lastEvent;
    argument >> info.flags;
    argument >> info.totalSize;
    argument >> info.bytesProgress;
    argument >> info.peer;
    argument >> info.displayName;
    argument >> info.path;
    argument >> info.mimetype;
    argument.endStructure();

    return argument;
}

static inline QDBusArgument &operator<<(QDBusArgument& argument, const Message& m)
{
    argument.beginStructure();
    argument << m.from;
    argument << m.payloads;
    argument.endStructure();

    return argument;
}

static inline const QDBusArgument &operator>>(const QDBusArgument& argument, Message& m)
{
    argument.beginStructure();
    argument >> m.from;
    argument >> m.payloads;
    argument.endStructure();

    return argument;
}
#endif

#ifndef ENABLE_LIBWRAP
static bool dbus_metaTypeInit = false;
#endif
inline void registerCommTypes() {
#ifndef ENABLE_LIBWRAP
   qDBusRegisterMetaType<MapStringString>               ();
   qDBusRegisterMetaType<MapStringInt>                  ();
   qDBusRegisterMetaType<VectorMapStringString>         ();
   qDBusRegisterMetaType<MapStringMapStringVectorString>();
   qDBusRegisterMetaType<VectorInt>                     ();
   qDBusRegisterMetaType<VectorUInt>                    ();
   qDBusRegisterMetaType<VectorULongLong>               ();
   qDBusRegisterMetaType<VectorString>                  ();
   qDBusRegisterMetaType<MapStringVectorString>         ();
   qDBusRegisterMetaType<VectorVectorByte>              ();
   qDBusRegisterMetaType<DataTransferInfo>              ();
   qDBusRegisterMetaType<Message>                       ();
   qDBusRegisterMetaType<QVector<Message>>              ();
   dbus_metaTypeInit = true;
#endif
}

#pragma GCC diagnostic pop
