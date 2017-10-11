/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
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
#include "newshmrenderer.h"

// Models and database
#include "api/lrc.h"

// system
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */ 
#include <semaphore.h>
#include <unistd.h>


struct SHMHeader {
   sem_t    mutex        ; /*!< Lock it before any operations on following fields.           */
   sem_t    frameGenMutex; /*!< unlocked by producer when frameGen modified                  */
   unsigned frameGen     ; /*!< monotonically incremented when a producer changes readOffset */
   unsigned frameSize    ; /*!< size in bytes of 1 frame                                     */
   unsigned mapSize      ; /*!< size to map if you need to see all data                      */
   unsigned readOffset   ; /*!< offset of readable frame in data                             */
   unsigned writeOffset  ; /*!< offset of writable frame in data                             */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
   uint8_t data[]     ; /*!< the whole shared memory                                      */
#pragma GCC diagnostic pop
};

namespace lrc
{

using namespace api;

class NewShmRendererPimpl : public QObject
{
    Q_OBJECT
public:
    NewShmRendererPimpl(const std::string& shmPath);
    ~NewShmRendererPimpl();
    bool startShm();
    void stopShm();
    bool remapShm();
    bool getNewFrame();
    int sharedMem_ = -1; // file descriptor
    const std::string shmPath_;
    std::shared_ptr<lrc::api::video::Frame> m_pFrame;
    SHMHeader* shmArea_;
    unsigned shmAreaLen_;
    uint m_FrameGen;
    bool shmLock();
    void shmUnlock();
    bool isRendering_ {false};
};

NewShmRendererPimpl::NewShmRendererPimpl(const std::string& shmPath)
: shmPath_(shmPath)
{
    
}

NewShmRendererPimpl::~NewShmRendererPimpl()
{
}


NewShmRenderer::NewShmRenderer(const std::string& shmPath, int width, int height)
: lrc::BaseRender::BaseRender(width, height)
, pimpl_(std::make_unique<NewShmRendererPimpl>(shmPath))
{

}

NewShmRenderer::~NewShmRenderer()
{
}

bool
NewShmRenderer::isRendering() const
{
    return false;
}

void
NewShmRenderer::stopRendering()
{
    //~ QMutexLocker locker {mutex()};

    pimpl_->stopShm();

    pimpl_->isRendering_ = false;
}

bool
NewShmRenderer::startRendering()
{
    //~ QMutexLocker locker {mutex()};

    if (not pimpl_->startShm() || pimpl_->isRendering_)
        return false;

    pimpl_->isRendering_ = true;
    return true;
}

bool
NewShmRendererPimpl::startShm()
{
    if(sharedMem_ != -1)
        return false;

    sharedMem_ = shm_open(shmPath_.c_str(), O_RDWR, 0);

    // Map only header data
    const auto mapSize = sizeof(SHMHeader);
    shmArea_ = (SHMHeader*) ::mmap(nullptr, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMem_, 0);

    if (shmArea_ == MAP_FAILED) {
        qDebug() << "Could not remap shared area";
        return false;
    }

   shmAreaLen_ = mapSize;

    return true;
}

void
NewShmRendererPimpl::stopShm()
{
    // [jn] todo
    if (sharedMem_ < 0)
        return;

    ::close(sharedMem_);
    sharedMem_ = -1;

    m_pFrame.reset();

    if (shmArea_ == MAP_FAILED)
        return;

    ::munmap(shmArea_, shmAreaLen_);
    shmAreaLen_ = 0;
    shmArea_ = (SHMHeader*) MAP_FAILED;

}

/// Lock the memory while the copy is being made
bool
NewShmRendererPimpl::shmLock()
{
   return ::sem_wait(&shmArea_->mutex) >= 0;
}

/// Remove the lock, allow a new frame to be drawn
void NewShmRendererPimpl::shmUnlock()
{
   ::sem_post(&shmArea_->mutex);
}

/// Remap the shared memory
/// Shared memory in unlocked state if returns false (resize failed).
bool NewShmRendererPimpl::remapShm()
{
    // This loop handles case where daemon resize shared memory
    // during time we unlock it for remapping.
    while (shmAreaLen_ != shmArea_->mapSize) { // [jn] ne serait-il pas judicieux de mettre un timer ?
        auto mapSize = shmArea_->mapSize;
        shmUnlock();

        if (::munmap(shmArea_, shmAreaLen_)) {
            qDebug() << "Could not unmap shared area: " << strerror(errno);
            return false;
        }

        shmArea_ = (SHMHeader*) ::mmap(nullptr, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMem_, 0);

        if (shmArea_ == MAP_FAILED) {
            qDebug() << "Could not remap shared area: " << strerror(errno);
            return false;
        }

        if (!shmLock())
            return false;

        shmAreaLen_ = mapSize;
    }

   return true;
}

bool
NewShmRendererPimpl::getNewFrame()
{
    if (!shmLock())
        return false;

    if (m_FrameGen == shmArea_->frameGen) {
        shmUnlock();

        // wait for a new frame, max 33ms
        static const struct timespec timeout = {0, 33000000};
        if (::sem_timedwait(&shmArea_->frameGenMutex, &timeout) < 0)
            return false;

        if (!shmLock())
            return false;
    }

    // valid frame to render (daemon may have stopped)?
    if (! shmArea_->frameSize) {
        shmUnlock();
        return false;
    }

    // map frame data
    if (!remapShm()) {
        qDebug() << "Could not resize shared memory";
        return false;
    }

    auto& frame_ptr = m_pFrame;
    if (not frame_ptr)
       frame_ptr.reset(new lrc::api::video::Frame);

    frame_ptr->storage.clear();
    frame_ptr->ptr = shmArea_->data + shmArea_->readOffset;
    frame_ptr->size = shmArea_->frameSize;
    m_FrameGen = shmArea_->frameGen;

    shmUnlock();

    return true;
}

lrc::api::video::Frame
NewShmRenderer::currentFrame()
{
    if (not pimpl_->isRendering_)
        return {};

    //~ QMutexLocker lk {mutex()}; // protect against thread
    if (pimpl_->getNewFrame()) {
        if (auto frame_ptr = pimpl_->m_pFrame)
            return std::move(*frame_ptr);
    }
    return {};
}

} // namespace lrc

#include "newshmrenderer.moc"
