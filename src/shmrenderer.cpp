/*
 *  Copyright (C) 2012-2022 Savoir-faire Linux Inc.
 *  Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>
 *  Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "shmrenderer.h"

#include "dbus/videomanager.h"
#include "videomanager_interface.h"

#include <QDebug>
#include <QMutex>
#include <QThread>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <errno.h>

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

#include <QTimer>

#include <chrono>

namespace lrc {

using namespace api::video;

namespace video {

// Uncomment following line to output in console the FPS value
//#define DEBUG_FPS

/* Shared memory object
 * Implementation note: double-buffering
 * Shared memory is divided in two regions, each representing one frame.
 * First byte of each frame is warranted to by aligned on 16 bytes.
 * One region is marked readable: this region can be safely read.
 * The other region is writeable: only the producer can use it.
 */

struct SHMHeader
{
    sem_t mutex;          /*!< Lock it before any operations on following fields.           */
    sem_t frameGenMutex;  /*!< unlocked by producer when frameGen modified                  */
    unsigned frameGen;    /*!< monotonically incremented when a producer changes readOffset */
    unsigned frameSize;   /*!< size in bytes of 1 frame                                     */
    unsigned mapSize;     /*!< size to map if you need to see all data                      */
    unsigned readOffset;  /*!< offset of readable frame in data                             */
    unsigned writeOffset; /*!< offset of writable frame in data                             */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    uint8_t data[]; /*!< the whole shared memory                                      */
#pragma GCC diagnostic pop
};

struct ShmRenderer::Impl final : public QObject
{
    Q_OBJECT
public:
    Impl(ShmRenderer* parent)
        : QObject(nullptr)
        , parent_(parent)
        , fd(-1)
        , shmArea((SHMHeader*) MAP_FAILED)
        , shmAreaLen(0)
        , frameGen(0)
        , fpsC(0)
        , fps(0)
        , timer(new QTimer(this))
#ifdef DEBUG_FPS
        , frameCount(0)
        , lastFrameDebug(std::chrono::system_clock::now())
#endif
    {
        timer->setInterval(33);
        connect(timer, &QTimer::timeout, [this]() { Q_EMIT parent_->frameUpdated(); });
        VideoManager::instance().startShmSink(parent_->id(), true);

        parent_->moveToThread(&thread);
        connect(&thread, &QThread::finished, [this] { parent_->stopRendering(); });
        thread.start();
    };
    ~Impl()
    {
        thread.quit();
        thread.wait();
    }

    // Constants
    constexpr static const int FPS_RATE_SEC = 1;
    constexpr static const int FRAME_CHECK_RATE_HZ = 120;

    // Lock the memory while the copy is being made
    bool shmLock() { return ::sem_wait(&shmArea->mutex) >= 0; };

    // Remove the lock, allow a new frame to be drawn
    void shmUnlock() { ::sem_post(&shmArea->mutex); };

    // Wait for new frame data from shared memory and save pointer.
    bool getNewFrame(bool wait)
    {
        if (!shmLock())
            return false;

        if (frameGen == shmArea->frameGen) {
            shmUnlock();

            if (not wait)
                return false;

            // wait for a new frame, max 33ms
            static const struct timespec timeout = {0, 33000000};
            if (::sem_timedwait(&shmArea->frameGenMutex, &timeout) < 0)
                return false;

            if (!shmLock())
                return false;
        }

        // valid frame to render (daemon may have stopped)?
        if (!shmArea->frameSize) {
            shmUnlock();
            return false;
        }

        // map frame data
        if (!remapShm()) {
            qDebug() << "Could not resize shared memory";
            return false;
        }

        if (not frame)
            frame.reset(new lrc::api::video::Frame);
        frame->ptr = shmArea->data + shmArea->readOffset;
        frame->size = shmArea->frameSize;
        frameGen = shmArea->frameGen;

        shmUnlock();

        ++fpsC;

        // Compute the FPS shown to the client
        auto currentTime = std::chrono::system_clock::now();
        const std::chrono::duration<double> seconds = currentTime - lastFrameDebug;
        if (seconds.count() >= FPS_RATE_SEC) {
            fps = (int) (fpsC / seconds.count());
            fpsC = 0;
            lastFrameDebug = currentTime;
#ifdef DEBUG_FPS
            qDebug() << this << ": FPS " << fps;
#endif
        }

        return true;
    };

    // Remap the shared memory.
    // Shared memory is in an unlocked state if returns false (resize failed).
    bool remapShm()
    {
        // This loop handles case where daemon resize shared memory
        // during time we unlock it for remapping.
        while (shmAreaLen != shmArea->mapSize) {
            auto mapSize = shmArea->mapSize;
            shmUnlock();

            if (::munmap(shmArea, shmAreaLen)) {
                qDebug() << "Could not unmap shared area: " << strerror(errno);
                return false;
            }

            shmArea
                = (SHMHeader*) ::mmap(nullptr, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            if (shmArea == MAP_FAILED) {
                qDebug() << "Could not remap shared area: " << strerror(errno);
                return false;
            }

            if (!shmLock())
                return false;

            shmAreaLen = mapSize;
        }

        return true;
    };

private:
    ShmRenderer* parent_;

public:
    QString path;
    int fd;
    SHMHeader* shmArea;
    unsigned shmAreaLen;
    uint frameGen;

    int fpsC;
    int fps;
    std::chrono::time_point<std::chrono::system_clock> lastFrameDebug;

    QTimer* timer;
    QMutex mutex;
    QThread thread;
    std::shared_ptr<lrc::api::video::Frame> frame;
};

ShmRenderer::ShmRenderer(const QString& id, const QSize& res, const QString& shmPath)
    : Renderer(id, res)
    , pimpl_(std::make_unique<ShmRenderer::Impl>(this))
{
    pimpl_->path = shmPath;
}

ShmRenderer::~ShmRenderer()
{
    VideoManager::instance().startShmSink(id(), false);
    stopShm();
}

void
ShmRenderer::update(const QSize& res, const QString& shmPath)
{
    Renderer::update(res);

    if (!pimpl_->thread.isRunning())
        pimpl_->thread.start();

    pimpl_->path = shmPath;
    VideoManager::instance().startShmSink(id(), true);
}

Frame
ShmRenderer::currentFrame() const
{
    QMutexLocker lk {&pimpl_->mutex};
    if (pimpl_->getNewFrame(false)) {
        if (auto frame_ptr = pimpl_->frame)
            return std::move(*frame_ptr);
    }
    return {};
}

bool
ShmRenderer::startShm()
{
    if (pimpl_->fd != -1) {
        qWarning() << "fd must be -1";
        return false;
    }

    pimpl_->fd = ::shm_open(pimpl_->path.toLatin1(), O_RDWR, 0);

    if (pimpl_->fd < 0) {
        qWarning() << "could not open shm area" << pimpl_->path
                   << ", shm_open failed:" << strerror(errno);
        return false;
    }

    // Map only header data
    const auto mapSize = sizeof(SHMHeader);
    pimpl_->shmArea
        = (SHMHeader*) ::mmap(nullptr, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, pimpl_->fd, 0);

    if (pimpl_->shmArea == MAP_FAILED) {
        qWarning() << "Could not remap shared area";
        return false;
    }

    pimpl_->shmAreaLen = mapSize;
    return true;
}

void
ShmRenderer::stopShm()
{
    if (pimpl_->fd < 0)
        return;

    pimpl_->timer->stop();

    // Emit the signal before closing the file, this lower the risk of invalid
    // memory access
    Q_EMIT stopped();

    {
        QMutexLocker lk(&pimpl_->mutex);
        // reset the frame so it doesn't point to an old value
        pimpl_->frame.reset();
    }

    ::close(pimpl_->fd);
    pimpl_->fd = -1;

    if (pimpl_->shmArea == MAP_FAILED)
        return;

    ::munmap(pimpl_->shmArea, pimpl_->shmAreaLen);
    pimpl_->shmAreaLen = 0;
    pimpl_->shmArea = (SHMHeader*) MAP_FAILED;
}

void
ShmRenderer::startRendering()
{
    QMutexLocker lk(&pimpl_->mutex);

    if (!startShm())
        return;

    pimpl_->timer->start();

    Q_EMIT started();
}

// Done on destroy instead
void
ShmRenderer::stopRendering()
{}

} // namespace video
} // namespace lrc

#include "moc_shmrenderer.cpp"
#include "shmrenderer.moc"
