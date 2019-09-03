/******************************************************************************
 *   Copyright (C) 2014-2019 by Savoir-faire Linux                            *
 *   Author : Philippe Groarke <philippe.groarke@savoirfairelinux.com>        *
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
#ifndef CONVERSIONS_WRAP_H
#define CONVERSIONS_WRAP_H

#include <map>
#include <string>
#include <vector>

#include "../typedefs.h"

#define Q_NOREPLY

//Print all call to some signals
#ifdef VERBOSE_IPC
 #define LOG_DRING_SIGNAL(name,arg) qDebug() << "\033[22;34m >>>>>> \033[0m" << name << arg;
 #define LOG_DRING_SIGNAL2(name,arg,arg2) qDebug() << "\033[22;34m >>>>>> \033[0m" << name << arg << arg2;
 #define LOG_DRING_SIGNAL3(name,arg,arg2,arg3) qDebug() << "\033[22;34m >>>>>> \033[0m" << name << arg << arg2 << arg3;
 #define LOG_DRING_SIGNAL4(name,arg,arg2,arg3,arg4) qDebug() << "\033[22;34m >>>>>> \033[0m" << name << arg << arg2 << arg3 << arg4;
#else
 #define LOG_DRING_SIGNAL(name,args) //Nothing
 #define LOG_DRING_SIGNAL2(name,arg,arg2)
 #define LOG_DRING_SIGNAL3(name,arg,arg2,arg3)
 #define LOG_DRING_SIGNAL4(name,arg,arg2,arg3,arg4)
#endif

inline MapStringString convertMap(const std::map<std::string, std::string>& m) {
   MapStringString temp;
   for (const auto& x : m) {
#if defined(UNICODE) && defined(_WIN32)
	  temp[QString(x.first.c_str())] = QString::fromLocal8Bit(x.second.c_str());
#else
	  temp[QString(x.first.c_str())] = QString(x.second.c_str());
#endif
   }
   return temp;
}

inline std::map<std::string, std::string> convertMap(const MapStringString& m) {
   std::map<std::string, std::string> temp;
   for (const auto& x : m.toStdMap()) {
#if defined(UNICODE) && defined(_WIN32)
	   temp[x.first.toStdString()] = std::string(x.second.toLocal8Bit().constData());
#else
	   temp[x.first.toStdString()] = x.second.toStdString();
#endif
   }
   return temp;
}

inline VectorMapStringString convertVecMap(const std::vector<std::map<std::string, std::string>>& m) {
   VectorMapStringString temp;
   for (const auto& x : m) {
      temp.push_back(convertMap(x));
   }
   return temp;
}

inline QStringList convertStringList(const std::vector<std::string>& v) {
   QStringList temp;
   for (const auto& x : v) {
#if defined(UNICODE) && defined(_WIN32)
	   temp.push_back(QString::fromLocal8Bit(x.c_str()));
#else
	   temp.push_back(QString(x.c_str()));
#endif
   }
   return temp;
}

inline VectorString convertVectorString(const std::vector<std::string>& v) {
   VectorString temp;
   for (const auto& x : v) {
#if defined(UNICODE) && defined(_WIN32)
	   temp.push_back(QString::fromLocal8Bit(x.c_str()));
#else
	   temp.push_back(QString(x.c_str()));
#endif
   }
   return temp;
}

inline VectorULongLong convertVectorULongLong(const std::vector<uint64_t>& v) {
   VectorULongLong temp;
   for (const auto& x : v) {
      temp.push_back(x);
   }
   return temp;
}

inline std::vector<std::string> convertStringList(const QStringList& v) {
   std::vector<std::string> temp;
   for (const auto& x : v) {
#if defined(UNICODE) && defined(_WIN32)
	   temp.push_back(std::string(x.toLocal8Bit().constData()));
#else
	   temp.push_back(x.toStdString());
#endif
   }
   return temp;
}

inline MapStringInt  convertStringInt(const std::map<std::string, int>& m) {
   MapStringInt temp;
   for (const auto& x : m) {
      temp[QString(x.first.c_str())] = x.second;
   }
   return temp;
}

constexpr static const char* TRUE_STR = "true";
constexpr static const char* FALSE_STR = "false";

static inline QString
toQString(bool b) noexcept
{
    return b ? TRUE_STR : FALSE_STR;
}

static inline QString
toQString(const std::string& str) noexcept
{
#if defined(UNICODE) && defined(_WIN32)
	return QString::fromLocal8Bit(str.c_str());
#else
	return QString::fromStdString(str);
#endif
}

static inline QString
toQString(int i) noexcept
{
    return QString::number(i);
}

static inline bool
toBool(QString qs) noexcept
{
    return qs == TRUE_STR ? true : false;
}

static inline int
toInt(QString qs) noexcept
{
    return qs.toInt();
}

static inline std::string
toStdString(QString qs) noexcept
{
#if defined(UNICODE) && defined(_WIN32)
	return std::string(qs.toLocal8Bit().constData());
#else
	return qs.toStdString();
#endif
}

#endif //CONVERSIONS_WRAP_H
