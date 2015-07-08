/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *   Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>
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
#include "shmrenderer.h"

#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QThread>

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

#include <QtCore/QTimer>
#include <chrono>

#include "private/videorenderermanager.h"
#include "video/resolution.h"
#include "private/videorenderer_p.h"

// Uncomment following line to output in console the FPS value
//#define DEBUG_FPS

/* Shared memory object
 * Implementation note: double-buffering
 * Shared memory is divided in two regions, each representing one frame.
 * First byte of each frame is warranted to by aligned on 16 bytes.
 * One region is marked readable: this region can be safely read.
 * The other region is writeable: only the producer can use it.
 */

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
   char data[];           /*!< the whole shared memory                                       */
#pragma GCC diagnostic pop
};

namespace Video {

class ShmRendererPrivate final : public QObject
{
   Q_OBJECT

public:
   ShmRendererPrivate(ShmRenderer* parent);

   //Types
   using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

   // Attributes
   QString    m_ShmPath       ;
   int        m_fd            ;
   SHMHeader* m_pShmArea      ;
   unsigned   m_ShmAreaLen    ;
   uint       m_FrameGen      ;
   int        m_fpsC          ;
   int        m_Fps           ;
   TimePoint  m_lastFrameDebug;
   QTimer*    m_pTimer        ;

   // Constants
   constexpr static const int FPS_RATE_SEC        = 1  ;
   constexpr static const int FRAME_CHECK_RATE_HZ = 120;

   // Helpers
   timespec createTimeout(           );
   bool     shmLock      (           );
   void     shmUnlock    (           );
   bool     getNewFrame  ( bool wait );
   bool     remapShm     (           );

private:
   Video::ShmRenderer* q_ptr;
};

ShmRendererPrivate::ShmRendererPrivate(ShmRenderer* parent)
   : QObject     ( parent                              )
   , q_ptr       ( parent                              )
   , m_fd        ( -1                                  )
   , m_fpsC      ( 0                                   )
   , m_Fps       ( 0                                   )
   , m_pShmArea  ( (SHMHeader*)MAP_FAILED              )
   , m_ShmAreaLen( 0                                   )
   , m_FrameGen  ( 0                                   )
   , m_pTimer    ( nullptr                             )
#ifdef DEBUG_FPS
   , m_frameCount( 0                                   )
   , m_lastFrameDebug(std::chrono::system_clock::now() )
#endif
{
}

/// Constructor
ShmRenderer::ShmRenderer(const QByteArray& id, const QString& shmPath, const QSize& res)
   : Renderer(id, res)
   , d_ptr(new ShmRendererPrivate(this))
{
   d_ptr->m_ShmPath = shmPath;
   setObjectName("Video::Renderer:"+id);
}

/// Destructor
ShmRenderer::~ShmRenderer()
{
   if (d_ptr->m_pTimer) {
      d_ptr->m_pTimer->stop();
      d_ptr->m_pTimer = nullptr;
   }
   stopShm();
}

/// Wait new frame data from shared memory and save pointer
bool ShmRendererPrivate::getNewFrame(bool wait)
{
   if (!shmLock())
      return false;

   if (m_FrameGen == m_pShmArea->frameGen) {
      shmUnlock();

      if (not wait)
         return false;

      // wait for a new frame, max 33ms
      static const struct timespec timeout = {0, 33000000};
      if (::sem_timedwait(&m_pShmArea->frameGenMutex, &timeout) < 0)
         return false;

      if (!shmLock())
         return false;

   }

   // valid frame to render (daemon may have stopped)?
   if (! m_pShmArea->frameSize) {
      shmUnlock();
      return false;
   }

   // map frame data
   if (!remapShm()) {
      qDebug() << "Could not resize shared memory";
      return false;
   }

   q_ptr->Video::Renderer::d_ptr->m_pFrame    = (char*)(m_pShmArea->data + m_pShmArea->readOffset);
   m_FrameGen                                 = m_pShmArea->frameGen;
   q_ptr->Video::Renderer::d_ptr->m_FrameSize = m_pShmArea->frameSize;

   shmUnlock();

   ++m_fpsC;

   // Compute the FPS shown to the client
   auto currentTime = std::chrono::system_clock::now();
   const std::chrono::duration<double> seconds = currentTime - m_lastFrameDebug;
   if (seconds.count() >= FPS_RATE_SEC) {
      m_Fps = m_fpsC / seconds.count();
      m_fpsC = 0;
      m_lastFrameDebug = currentTime;
#ifdef DEBUG_FPS
      qDebug() << this << ": FPS " << m_fps;
#endif
   }

   return true;
}

/// Remap the shared memory
/// Shared memory in unlocked state if returns false (resize failed).
bool ShmRendererPrivate::remapShm()
{
   // This loop handles case where daemon resize shared memory
   // during time we unlock it for remapping.
   while (m_ShmAreaLen != m_pShmArea->mapSize) {
      auto mapSize = m_pShmArea->mapSize;
      shmUnlock();

      if (::munmap(m_pShmArea, m_ShmAreaLen)) {
         qDebug() << "Could not unmap shared area: " << strerror(errno);
         return false;
      }

      m_pShmArea = (SHMHeader*) ::mmap(nullptr, mapSize, PROT_READ | PROT_WRITE,
                                       MAP_SHARED, m_fd, 0);

      if (m_pShmArea == MAP_FAILED) {
         qDebug() << "Could not remap shared area: " << strerror(errno);
         return false;
      }

      if (!shmLock())
         return false;

      m_ShmAreaLen = mapSize;
   }

   return true;
}

/// Connect to the shared memory
bool ShmRenderer::startShm()
{
   if (d_ptr->m_fd != -1) {
      qDebug() << "fd must be -1";
      return false;
   }

   d_ptr->m_fd = ::shm_open(d_ptr->m_ShmPath.toLatin1(), O_RDWR, 0);

   if (d_ptr->m_fd < 0) {
      qDebug() << "could not open shm area" << d_ptr->m_ShmPath
               << ", shm_open failed:"      << strerror(errno);
      return false;
   }

   // Map only header data
   const auto mapSize = sizeof(SHMHeader);
   d_ptr->m_pShmArea = (SHMHeader*) ::mmap(nullptr, mapSize,
      PROT_READ | PROT_WRITE,
      MAP_SHARED, d_ptr->m_fd, 0
   );

   if (d_ptr->m_pShmArea == MAP_FAILED) {
      qDebug() << "Could not remap shared area";
      return false;
   }

   d_ptr->m_ShmAreaLen = mapSize;
   return true;
}

/// Disconnect from the shared memory
void ShmRenderer::stopShm()
{
   if (d_ptr->m_fd < 0)
      return;

   if (d_ptr->m_pTimer) {
      d_ptr->m_pTimer->stop();
      d_ptr->m_pTimer = nullptr;
   }

   //Emit the signal before closing the file, this lower the risk of invalid
   //memory access
   emit stopped();

   ::close(d_ptr->m_fd);
   d_ptr->m_fd = -1;

   if (d_ptr->m_pShmArea == MAP_FAILED)
      return;

   ::munmap(d_ptr->m_pShmArea, d_ptr->m_ShmAreaLen);
   d_ptr->m_ShmAreaLen = 0;
   d_ptr->m_pShmArea = (SHMHeader*) MAP_FAILED;
}

/// Lock the memory while the copy is being made
bool ShmRendererPrivate::shmLock()
{
   return ::sem_wait(&m_pShmArea->mutex) >= 0;
}

/// Remove the lock, allow a new frame to be drawn
void ShmRendererPrivate::shmUnlock()
{
   ::sem_post(&m_pShmArea->mutex);
}

/*****************************************************************************
 *                                                                           *
 *                                   Slots                                   *
 *                                                                           *
 ****************************************************************************/

/// Start the rendering loop
void ShmRenderer::startRendering()
{
   QMutexLocker locker {mutex()};

   if (!startShm())
      return;

   Video::Renderer::d_ptr->m_isRendering = true;

   if (!d_ptr->m_pTimer) {
      d_ptr->m_pTimer = new QTimer(this);
      d_ptr->m_pTimer->setInterval(33);
      connect(d_ptr->m_pTimer,&QTimer::timeout,[this]() {
         emit this->frameUpdated();
      });
   }
   //FIXME This is a temporary hack as frameUpdated() is no longer emitted
   d_ptr->m_pTimer->start();

   emit started();
}

/// Stop the rendering loop
void ShmRenderer::stopRendering()
{
   QMutexLocker locker {mutex()};
   Video::Renderer::d_ptr->m_isRendering = false;

   if (d_ptr->m_pTimer) {
      d_ptr->m_pTimer->stop();
      d_ptr->m_pTimer = nullptr;
   }

   stopShm();
}

/*****************************************************************************
 *                                                                           *
 *                                 Getters                                   *
 *                                                                           *
 ****************************************************************************/

/// Get the current frame rate of this renderer
int ShmRenderer::fps() const
{
   return d_ptr->m_Fps;
}

/// Get frame data pointer from shared memory
const QVariant ShmRenderer::currentFrame() const
{
   if (!isRendering()) {
      if (Video::Renderer::d_ptr->m_pFrame && Video::Renderer::d_ptr->m_FrameSize)
         Video::Renderer::d_ptr->m_Content.setRawData(Video::Renderer::d_ptr->m_pFrame,Video::Renderer::d_ptr->m_FrameSize);
      return QVariant::fromValue(Video::Renderer::d_ptr->m_Content);
   }

   QMutexLocker lk {mutex()};
   d_ptr->getNewFrame(false);
   if (Video::Renderer::d_ptr->m_pFrame && Video::Renderer::d_ptr->m_FrameSize)
      Video::Renderer::d_ptr->m_Content.setRawData(Video::Renderer::d_ptr->m_pFrame,Video::Renderer::d_ptr->m_FrameSize);
   return QVariant::fromValue(Video::Renderer::d_ptr->m_Content);
}

Video::Renderer::ColorSpace ShmRenderer::colorSpace() const
{
   return Video::Renderer::ColorSpace::BGRA;
}

/*****************************************************************************
 *                                                                           *
 *                                 Setters                                   *
 *                                                                           *
 ****************************************************************************/

void ShmRenderer::setShmPath(const QString& path)
{
   d_ptr->m_ShmPath = path;
}

} // namespace Video

#include <shmrenderer.moc>
