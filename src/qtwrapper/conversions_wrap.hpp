/******************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                                 *
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

#include "../dbus/metatypes.h"

#define Q_NOREPLY

inline MapStringString convertMap(const std::map<std::string, std::string>& m) {
    MapStringString temp;
    for (const auto& x : m) {
        temp[QString(x.first.c_str())] = QString(x.second.c_str());
    }
    return temp;
}

inline std::map<std::string, std::string> convertMap(const MapStringString& m) {
    std::map<std::string, std::string> temp;
    for (const auto& x : m.toStdMap()) {
        temp[x.first.toStdString()] = x.second.toStdString();
    }
    return temp;
}

inline QStringList convertStringList(const std::vector<std::string>& v) {
    QStringList temp;
    for (const auto& x : v) {
        temp.push_back(QString(x.c_str()));
    }
    return temp;
}

inline std::vector<std::string> convertStringList(const QStringList& v) {
    std::vector<std::string> temp;
    for (const auto& x : v) {
        temp.push_back(x.toStdString());
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

#endif //CONVERSIONS_WRAP_H
