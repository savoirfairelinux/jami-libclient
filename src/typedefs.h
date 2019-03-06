/****************************************************************************
 *    Copyright (C) 2009-2019 Savoir-faire Linux Inc.                          *
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
#pragma once

//Qt
#include <QtCore/QMetaType>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>

//Typedefs (required to avoid '<' and '>' in the DBus XML)
typedef QMap<QString, QString>                              MapStringString               ;
typedef QMap<QString, int>                                  MapStringInt                  ;
typedef QVector<int>                                        VectorInt                     ;
typedef QVector<uint>                                       VectorUInt                    ;
typedef QVector<qulonglong>                                 VectorULongLong               ;
typedef QVector< QMap<QString, QString> >                   VectorMapStringString         ;
typedef QVector< QString >                                  VectorString                  ;
typedef QMap< QString, QMap< QString, QVector<QString> > >  MapStringMapStringVectorString;
typedef QMap< QString, QVector<QString> >                   MapStringVectorString         ;
typedef QMap< QString, QMap< QString, QStringList > >       MapStringMapStringStringList  ;
typedef QMap< QString, QStringList >                        MapStringStringList           ;
typedef QVector< QByteArray >                               VectorVectorByte              ;

// Adapted from libring DRing::DataTransferInfo
struct DataTransferInfo
{
    QString accountId;
    quint32 lastEvent;
    quint32 flags;
    qlonglong totalSize;
    qlonglong bytesProgress;
    QString peer;
    QString displayName;
    QString path;
    QString mimetype;
};

struct Message {
    QString from;
    MapStringString payloads;
    quint64 received;
};

typedef QVector<Message> messages;

/**
 * This function add a safe way to get an enum class size
 * @note it cannot be "const" due to some compiler issues
 * @note it cannot be unsigned to avoid some compiler warnings
 */
template<typename A> constexpr int enum_class_size() {
   return static_cast<int>(A::COUNT__);
}

#ifdef LRC_IMPORT
#define LIB_EXPORT Q_DECL_IMPORT
#else
#if defined(_MSC_VER)
#define LIB_EXPORT
#else
#define LIB_EXPORT Q_DECL_EXPORT
#endif
#endif

//Doesn't work
#if ((__GNUC_MINOR__ > 8) || (__GNUC_MINOR__ == 8))
   #define STRINGIFY(x) #x
   #define IGNORE_NULL(content)\
   _Pragma(STRINGIFY(GCC diagnostic ignored "-Wzero-as-null-pointer-constant")) \
      content
#else
   #define IGNORE_NULL(content) content
#endif //ENABLE_IGNORE_NULL

/**
 * Create a safe pack of flags from an enum class.
 *
 * This class exist to ensure all sources come from the same enum and that it is
 * never accidentally accidentally into an integer.
 *
 * This assume that the enum has been setup as flags.
 */
template<class T>
class LIB_EXPORT FlagPack
{
public:
   FlagPack() : m_Flags(0) {}
   FlagPack(const T& base) : m_Flags(static_cast<uint>(base)) {}
   FlagPack(const FlagPack<T>& other) : m_Flags(other.m_Flags) {}

   //Operator
   FlagPack<T>& operator|(const T& other) {
      m_Flags |= static_cast<uint>(other);
      return *this;
   }

   FlagPack<T>& operator|(const FlagPack<T>& other) {
      m_Flags |= other.m_Flags;
      return *this;
   }

   FlagPack<T>& operator|=(const T& other) {
      m_Flags |= static_cast<uint>(other);
      return *this;
   }

   FlagPack<T>& operator|=(const FlagPack<T>& other) {
      m_Flags |= other.m_Flags;
      return *this;
   }

   FlagPack<T>& operator^=(const T& other) {
      m_Flags ^= static_cast<uint>(other);
      return *this;
   }

   FlagPack<T>& operator^=(const FlagPack<T>& other) {
      m_Flags ^= other.m_Flags;
      return *this;
   }

   FlagPack<T> operator&(const T& other) const {
      return FlagPack<T>(m_Flags & static_cast<uint>(other));
   }

   FlagPack<T> operator&(const FlagPack<T>& other) const {
      return FlagPack<T>(m_Flags & other.m_Flags);
   }

   FlagPack<T>&operator=(const FlagPack<T>& other) {
      m_Flags = other.m_Flags;
      return *this;
   }

   bool operator!=(const T& other) const {
      return  m_Flags != static_cast<uint>(other);
   }

   bool operator==(const T& other) const {
      return  m_Flags == static_cast<uint>(other);
   }

   bool operator==(const FlagPack<T>& other) const {
      return  m_Flags == other.m_Flags;
   }

   bool operator!() const {
      return !m_Flags;
   }

   operator bool() const {
      return m_Flags != 0;
   }

   uint value() const {
      return m_Flags;
   }

private:
   FlagPack(uint base) : m_Flags(base) {}
   uint m_Flags;
};

#ifdef _MSC_VER
#define DO_PRAGMA(x) /*do nothing*/
#define __attribute__(A) /*do nothing*/
#include <ciso646>
#else
#define DO_PRAGMA(x) _Pragma (#x)
#endif // _MSC_VER

//Globally disable the "-Wunused-function" warning for GCC
//refs: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55578
#if ((__GNUC_MINOR__ > 8) || (__GNUC_MINOR__ == 8))
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define DECLARE_ENUM_FLAGS(T)\
DO_PRAGMA(GCC diagnostic push)\
DO_PRAGMA(GCC diagnostic ignored "-Wunused-function")\
__attribute__ ((unused)) static FlagPack<T> operator|(const T& first, const T& second) { \
   FlagPack<T> p (first); \
   return p | second; \
} \
DO_PRAGMA(GCC diagnostic pop)

#include <functional>
typedef std::function<void()> MigrationCb;