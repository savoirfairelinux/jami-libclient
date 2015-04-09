/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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
#ifndef RENDERERPRIVATE_H
#define RENDERERPRIVATE_H

//Qt
#include <QtCore/QObject>
#include <QtCore/QSize>

#include <atomic>

class QMutex;

namespace Video {

class Renderer;

class RendererPrivate : public QObject
{
Q_OBJECT
public:
   RendererPrivate(Video::Renderer* parent);

   //Attributes
   std::atomic_bool  m_isRendering;
   QMutex*           m_pMutex     ;
   QString           m_Id         ;
   QSize             m_pSize      ;
   void*             m_framePtr   ;

private:
   Video::Renderer* q_ptr;

};

}

#endif
