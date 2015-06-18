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
#include "threadworker.h"

//Qt
#include <QtCore/QThread>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

//LibStdC++
#include <functional>

ThreadWorker::ThreadWorker(std::function<void()> f) : QObject(nullptr)
{
   QThread* t = new QThread();

   connect(t, &QThread::finished, t, &QObject::deleteLater);

   moveToThread(t);

   QObject::connect(t, &QThread::started, [this,f]() {

      f();

      thread()->exit();

      moveToThread(QCoreApplication::instance()->thread());

      delete this;
   });

   t->start();
}