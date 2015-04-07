/****************************************************************************
 *   Copyright (C) 2009-2015 by Savoir-Faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

//Qt
#include <QtCore/QMetaType>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QDebug>

//Typedefs (required to avoid '<' and '>' in the DBus XML)
typedef QMap<QString, QString>                              MapStringString               ;
typedef QMap<QString, int>                                  MapStringInt                  ;
typedef QVector<int>                                        VectorInt                     ;
typedef QVector<uint>                                       VectorUInt                    ;
typedef QVector< QMap<QString, QString> >                   VectorMapStringString         ;
typedef QVector< QString >                                  VectorString                  ;
typedef QMap< QString, QMap< QString, QVector<QString> > >  MapStringMapStringVectorString;
typedef QMap< QString, QVector<QString> >                   MapStringVectorString         ;
typedef QMap< QString, QMap< QString, QStringList > >       MapStringMapStringStringList  ;
typedef QMap< QString, QStringList >                        MapStringStringList           ;

/**
 * This function add a safe way to get an enum class size
 * @note it cannot be "const" due to some compiler issues
 * @note it cannot be unsigned to avoid some compiler warnings
 */
template<typename A> constexpr int enum_class_size() {
   return static_cast<int>(A::COUNT__);
}

#define LIB_EXPORT Q_DECL_EXPORT
#define LIB_IMPORT Q_DECL_IMPORT

//Doesn't work
#if ((__GNUC_MINOR__ > 8) || (__GNUC_MINOR__ == 8))
   #define STRINGIFY(x) #x
   #define IGNORE_NULL(content)\
   _Pragma(STRINGIFY(GCC diagnostic ignored "-Wzero-as-null-pointer-constant")) \
      content
#else
   #define IGNORE_NULL(content) content
#endif //ENABLE_IGNORE_NULL

#endif //TYPEDEFS_H

