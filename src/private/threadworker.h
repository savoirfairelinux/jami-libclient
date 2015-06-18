/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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
#ifndef THREADWORKER_H
#define THREADWORKER_H

#include <QtCore/QObject>

namespace std {
   template<typename T>
   class function;
}

/**
 * Small utility to avoid the Qt4/C++03 boiler plate code for workers. There
 * doesn't seem to be a Qt5/C++11 way yet.
 */
class ThreadWorker : public QObject
{
   Q_OBJECT
public:
   ThreadWorker(std::function<void()> f);
};

#endif