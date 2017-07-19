/******************************************************************************
 *   Copyright (C) 2014-2017 Savoir-faire Linux                                 *
 *   Author : Philippe Groarke <philippe.groarke@savoirfairelinux.com>        *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>        *
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
 #include "videomanager_wrap.h"

VideoManagerInterface::VideoManagerInterface()
{
#ifdef ENABLE_VIDEO

    proxy = new VideoManagerSignalProxy(this);
    sender = new VideoManagerProxySender();

    QObject::connect(sender,&VideoManagerProxySender::deviceEvent,proxy,&VideoManagerSignalProxy::slotDeviceEvent, Qt::QueuedConnection);
    QObject::connect(sender,&VideoManagerProxySender::startedDecoding,proxy,&VideoManagerSignalProxy::slotStartedDecoding);
    QObject::connect(sender,&VideoManagerProxySender::stoppedDecoding,proxy,&VideoManagerSignalProxy::slotStoppedDecoding);


    using DRing::exportable_callback;
    using DRing::VideoSignal;
    videoHandlers = {
        exportable_callback<VideoSignal::DeviceEvent>(
            [this] () {
                emit sender->deviceEvent();
        }),
        exportable_callback<VideoSignal::DecodingStarted>(
            [this] (const std::string &id, const std::string &shmPath, int width, int height, bool isMixer) {
                emit sender->startedDecoding(QString(id.c_str()), QString(shmPath.c_str()), width, height, isMixer);
        }),
        exportable_callback<VideoSignal::DecodingStopped>(
            [this] (const std::string &id, const std::string &shmPath, bool isMixer) {
                emit sender->stoppedDecoding(QString(id.c_str()), QString(shmPath.c_str()), isMixer);
        })
    };
#endif
}

VideoManagerInterface::~VideoManagerInterface()
{

}

VideoManagerSignalProxy::VideoManagerSignalProxy(VideoManagerInterface* parent) : QObject(parent),
m_pParent(parent)
{}

void VideoManagerSignalProxy::slotDeviceEvent()
{
    emit m_pParent->deviceEvent();
}

void VideoManagerSignalProxy::slotStartedDecoding(const QString &id, const QString &shmPath, int width, int height, bool isMixer)
{
        emit m_pParent->startedDecoding(id,shmPath,width,height,isMixer);
}

void VideoManagerSignalProxy::slotStoppedDecoding(const QString &id, const QString &shmPath, bool isMixer)
{
        emit m_pParent->stoppedDecoding(id,shmPath,isMixer);
}
