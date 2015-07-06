/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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
#include "renderer.h"

//Ring
#include "private/videorenderer_p.h"

//Qt
#include <QtCore/QMutex>

Video::RendererPrivate::RendererPrivate(Video::Renderer* parent)
   : QObject(parent), q_ptr(parent)
   , m_pMutex(new QMutex()),m_pFrame(nullptr),m_FrameSize(0),m_isRendering(false)
{
}

Video::Renderer::Renderer(const QByteArray& id, const QSize& res) : d_ptr(new RendererPrivate(this))
{
   setObjectName("Renderer:"+id);
   d_ptr->m_FrameSize = res.width() * res.height() * 4;
   d_ptr->m_pSize     = res;
   d_ptr->m_Id        = id;
}

Video::Renderer::~Renderer()
{
   delete d_ptr;
}

/*****************************************************************************
*                                                                           *
*                                 Getters                                   *
*                                                                           *
****************************************************************************/

///Return if the rendering is currently active or not
bool Video::Renderer::isRendering() const
{
  return d_ptr->m_isRendering;
}

///Get mutex, in case renderer and views are not in the same thread
QMutex* Video::Renderer::mutex() const
{
  return d_ptr->m_pMutex;
}

///Return the current resolution
QSize Video::Renderer::size() const
{
  return d_ptr->m_pSize;
}

const std::shared_ptr<std::vector<unsigned char> >& Video::Renderer::currentFrame() const
{
   return d_ptr->m_iFrame;
}

/*****************************************************************************
 *                                                                           *
 *                                 Setters                                   *
 *                                                                           *
 ****************************************************************************/

void Video::Renderer::setSize(const QSize& size) const
{
  d_ptr->m_pSize = size;
}

#include <renderer.moc>
