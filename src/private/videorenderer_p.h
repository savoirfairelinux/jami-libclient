/****************************************************************************
 *   Copyright (C) 2013-2017 Savoir-faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *   Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>      *
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
#include <QtCore/QObject>
#include <QtCore/QSize>

// Std
#include <atomic>
#include <memory>

class QMutex;

namespace Video {

class Renderer;
struct Frame;

class RendererPrivate final : public QObject
{
Q_OBJECT
public:
    RendererPrivate(Video::Renderer* parent);

    //Attributes
    std::atomic_bool     m_isRendering ;
    QMutex*              m_pMutex      ;
    QString              m_Id          ;
    QSize                m_pSize       ;
    std::shared_ptr<Frame> m_pFrame; // frame given by daemon for direct rendering
private:
    Video::Renderer* q_ptr;
};

} // namespace Video
