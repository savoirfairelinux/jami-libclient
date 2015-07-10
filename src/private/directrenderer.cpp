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
#include "directrenderer.h"

#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtCore/QTimer>

#include <cstring>

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

#include "private/videorenderermanager.h"
#include "video/resolution.h"
#include "private/videorenderer_p.h"

namespace Video {

class DirectRendererPrivate : public QObject
{
   Q_OBJECT
public:
   DirectRendererPrivate(Video::DirectRenderer* parent);
private:
//    Video::DirectRenderer* q_ptr;
};

}

Video::DirectRendererPrivate::DirectRendererPrivate(Video::DirectRenderer* parent) : QObject(parent)/*, q_ptr(parent)*/
{
}

///Constructor
Video::DirectRenderer::DirectRenderer(const QByteArray& id, const QSize& res): Renderer(id, res), d_ptr(new DirectRendererPrivate(this))
{
   setObjectName("Video::DirectRenderer:"+id);
}

///Destructor
Video::DirectRenderer::~DirectRenderer()
{
}

void Video::DirectRenderer::startRendering()
{
   Video::Renderer::d_ptr->m_isRendering = true;
   emit started();
}
void Video::DirectRenderer::stopRendering ()
{
   Video::Renderer::d_ptr->m_isRendering = false;
   emit stopped();
}

void Video::DirectRenderer::onNewFrame(const std::shared_ptr<std::vector<unsigned char> >& frame, int w, int h)
{
   if (!isRendering()) {
      return;
   }

   Video::Renderer::d_ptr->m_pSize.setWidth(w);
   Video::Renderer::d_ptr->m_pSize.setHeight(h);
   Video::Renderer::d_ptr->m_pSFrame = frame;
   emit frameUpdated();
}

Video::Renderer::ColorSpace Video::DirectRenderer::colorSpace() const
{
   return Video::Renderer::ColorSpace::RGBA;
}

#include <directrenderer.moc>
