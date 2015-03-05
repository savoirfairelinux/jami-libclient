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

   //Attributes

private:
   Video::DirectRenderer* q_ptr;

};

}

Video::DirectRendererPrivate::DirectRendererPrivate(Video::DirectRenderer* parent) : QObject(parent), q_ptr(parent)
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

    qWarning() << "STARTING RENDERING";
    setRendering(true);
    emit started();
}
void Video::DirectRenderer::stopRendering ()
{
    qWarning() << "STOPPING RENDERING";
    setRendering(false);
    emit stopped();
}

void Video::DirectRenderer::onNewFrame(const QByteArray& frame)
{
//    qDebug("Frame received by DirectRenderer, size: w %d, h %d", size().width(), size().height());
    if (!isRendering()) {
       return;
    }
    if (static_cast<Video::Renderer*>(this)->d_ptr->otherFrame().size() != (size().height() * size().width()))
    static_cast<Video::Renderer*>(this)->d_ptr->otherFrame().resize(size().height() * size().width()*4);
    ::memcpy(static_cast<Video::Renderer*>(this)->d_ptr->otherFrame().data(),frame,static_cast<Video::Renderer*>(this)->d_ptr->otherFrame().size());
    static_cast<Video::Renderer*>(this)->d_ptr->updateFrameIndex();
    emit frameUpdated();
}

#include <directrenderer.moc>
