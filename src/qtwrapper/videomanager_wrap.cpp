#include "videomanager_wrap.h"

VideoManagerInterface::VideoManagerInterface()
{
#ifdef ENABLE_VIDEO

    proxy = new VideoManagerSignalProxy(this);
    sender = new VideoManagerProxySender();

    QObject::connect(sender,&VideoManagerProxySender::deviceEvent,proxy,&VideoManagerSignalProxy::slotDeviceEvent);
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

VideoManagerSignalProxy::VideoManagerSignalProxy(VideoManagerInterface* parent) : QObject(parent), m_pParent(parent)
{}

void VideoManagerSignalProxy::slotDeviceEvent()
{
    QTimer::singleShot(0, [=] {
        emit m_pParent->deviceEvent();
    });
}

void VideoManagerSignalProxy::slotStartedDecoding(const QString &id, const QString &shmPath, int width, int height, bool isMixer)
{
    QTimer::singleShot(0, [=] {
        emit m_pParent->startedDecoding(id,shmPath,width,height,isMixer);
    });
}

void VideoManagerSignalProxy::slotStoppedDecoding(const QString &id, const QString &shmPath, bool isMixer)
{
    QTimer::singleShot(0, [=] {
        emit m_pParent->stoppedDecoding(id,shmPath,isMixer);
    });
}
