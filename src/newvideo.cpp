///*
// *  Copyright (C) 2018-2022 Savoir-faire Linux Inc.
// *
// *  Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>  
// *
// *  This library is free software; you can redistribute it and/or
// *  modify it under the terms of the GNU Lesser General Public
// *  License as published by the Free Software Foundation; either
// *  version 2.1 of the License, or (at your option) any later version.
// *
// *  This library is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// *  Lesser General Public License for more details.
// *
// *  You should have received a copy of the GNU General Public License
// *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// */
//
//#include "api/newvideo.h"
//
//#include "dbus/videomanager.h"
//
//#include <QSize>
//#include <QMutex>
//
///**
//* Convert a string (WxH) to a QSize
//* @param res the string to convert
//* @return the QSize object
//*/
//static QSize stringToQSize(const QString& res)
//{
//    QString sizeStr = res;
//    auto sizeSplit = sizeStr.split('x');
//    if (sizeSplit.size() != 2)
//        return {};
//    auto width = sizeSplit.at(0).toInt();
//    auto height = sizeSplit.at(1).toInt();
//    return QSize(width, height);
//}
//
//namespace lrc {
//namespace video {
//
//using namespace lrc::api::video;
//
//struct Renderer::Impl : public QObject
//{
//    Q_OBJECT
//public:
//    Impl(Renderer& linked,
//         const QString& id,
//         Settings videoSettings,
//         const QString& shmPath)
//        : linked_(linked)
//        , id(id)
//        , videoSettings(videoSettings)
//        , frame {nullptr, AVFrameDeleter}
//    {
//        QSize size = stringToQSize(videoSettings.size);
//        moveToThread(&thread);
//
//        
//
//    /*#ifdef ENABLE_LIBWRAP
//        if (usingAVFrame_) {
//            VideoManager::instance().registerAVSinkTarget(id_, renderer->avTarget());
//        } else {
//            VideoManager::instance().registerSinkTarget(id_, renderer->target());
//        }
//    #else
//        VideoManager::instance().startShmSink(id_, true);
//    #endif*/
//
//        thread.start();
//    };
//    ~Impl();
//
//    QString id;
//    Settings videoSettings;
//    QThread thread;
//    std::atomic_bool isRendering;
//    QSize size;
//    QMutex* mutex;
//    Frame frame;
//
//private:
//    Renderer& linked_;
//};
//
//Renderer::Renderer(const QString& id,
//                   Settings videoSettings,
//                   const QString& shmPath)
//    : pimpl_(std::make_unique<Impl>(*this, id, videoSettings, shmPath))
//{}
//
//Renderer::~Renderer()
//{}
//
//void
//Renderer::update(const QString& res, const QString& shmPath)
//{
//    if (!pimpl_->thread.isRunning())
//        pimpl_->thread.start();
//
//    setSize(stringToQSize(res));
//
////#ifdef ENABLE_LIBWRAP
////    Q_UNUSED(shmPath)
////    if (pimpl_->usingAVFrame_) {
////        VideoManager::instance().registerAVSinkTarget(pimpl_->id_, pimpl_->renderer->avTarget());
////    } else {
////        VideoManager::instance().registerSinkTarget(pimpl_->id_, pimpl_->renderer->target());
////    }
////#else // ENABLE_LIBWRAP
////    pimpl_->renderer->setShmPath(shmPath);
////    VideoManager::instance().startShmSink(pimpl_->id_, true);
////#endif
//}
//
//bool
//Renderer::isRendering() const
//{
//    return pimpl_->isRendering;
//}
//
//QString
//Renderer::getId() const
//{
//    return pimpl_->id;
//}
//
////Frame
////Renderer::currentFrame() const
////{
////    // TODO(sblin) remove Video::Frame when deleting old models.
////    auto frame = pimpl_->renderer->currentFrame();
////    Frame result;
////    result.ptr = frame.ptr;
////    result.size = frame.size;
////    result.storage = frame.storage;
////    result.height = frame.height;
////    result.width = frame.width;
////    return result;
////}
//
//QSize
//Renderer::size() const
//{
//    return pimpl_->size;
//}
//
//} // namespace video
//} // namespace api
//
//RendererPimpl::RendererPimpl(Renderer& linked,
//                             const QString& id,
//                             Settings videoSettings,
//                             const QString& shmPath,
//                             bool useAVFrame)
//    : linked_(linked)
//    , id_(id)
//    , videoSettings_(videoSettings)
//    , usingAVFrame_(useAVFrame)
//{
//    QSize size = stringToQSize(videoSettings.size);
//#ifdef ENABLE_LIBWRAP
//    Q_UNUSED(shmPath)
//    renderer = std::make_unique<Video::DirectRenderer>(id, size, usingAVFrame_);
//#else // ENABLE_LIBWRAP
//    renderer = std::make_unique<Video::ShmRenderer>(id, shmPath, size);
//#endif
//    renderer->moveToThread(&thread_);
//
//    connect(&thread_, &QThread::finished, [this] { renderer.reset(); });
//
//    connect(&linked,
//            &Renderer::startRendering,
//            renderer.get(),
//            &Video::Renderer::startRendering,
//            Qt::QueuedConnection);
//
//    connect(renderer.get(), &Video::Renderer::frameUpdated, this, [this] {
//        emit linked_.frameUpdated(id_);
//    });
//
//    connect(renderer.get(), &Video::Renderer::frameBufferRequested, this, [this](AVFrame* avFrame) {
//        emit linked_.frameBufferRequested(id_, avFrame);
//    }, Qt::DirectConnection);
//
//    connect(
//        renderer.get(),
//        &Video::Renderer::started,
//        this,
//        [this] { emit linked_.started(id_); },
//        Qt::DirectConnection);
//    connect(
//        renderer.get(),
//        &Video::Renderer::stopped,
//        this,
//        [this] { emit linked_.stopped(id_); },
//        Qt::DirectConnection);
//
//#ifdef ENABLE_LIBWRAP
//    if (usingAVFrame_) {
//        VideoManager::instance().registerAVSinkTarget(id_, renderer->avTarget());
//    } else {
//        VideoManager::instance().registerSinkTarget(id_, renderer->target());
//    }
//#else
//    VideoManager::instance().startShmSink(id_, true);
//#endif
//
//    thread_.start();
//}
//
//RendererPimpl::~RendererPimpl()
//{
//    thread_.quit();
//    thread_.wait();
//}
//
//} // end of namespace lrc
//
//#include "api/moc_newvideo.cpp"
