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
#include <QtCore/QTime>

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
#include "private/videorenderermanager.h"
#include "video/resolution.h"
#include "private/videorenderer_p.h"

//#define DEBUG_FPS
#ifdef DEBUG_FPS
#include <chrono>
#endif

///Shared memory object
// Implementation note: double-buffering
// Shared memory is divided in two regions, each representing one frame.
// First byte of each frame is warranted to by aligned on 16 bytes.
// One region is marked readable: this region can be safely read.
// The other region is writeable: only the producer can use it.

struct SHMHeader {
    sem_t mutex; // Lock it before any operations on following fields.
    sem_t frameGenMutex; // unlocked by producer when frameGen modified
    unsigned frameGen; // monotonically incremented when a producer changes readOffset
    unsigned frameSize; // size in bytes of 1 frame
    unsigned readOffset; // offset of readable frame in data
    unsigned writeOffset; // offset of writable frame in data

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
    char data[]; // the whole shared memory
#pragma GCC diagnostic pop
};

namespace Video {

class ShmRendererPrivate : public QObject
{
   Q_OBJECT
public:
   ShmRendererPrivate(Video::ShmRenderer* parent);

   //Attributes
   QString           m_ShmPath    ;
   int               m_fd         ;
   SHMHeader*        m_pShmArea   ;
   unsigned          m_ShmAreaLen ;
   uint              m_BufferGen  ;
   QTimer*           m_pTimer     ;
   int               m_fpsC       ;
   int               m_Fps        ;
   QTime             m_CurrentTime;

#ifdef DEBUG_FPS
   unsigned          m_frameCount;
   std::chrono::time_point<std::chrono::system_clock> m_lastFrameDebug;
#endif

   //Constants
   static const int TIMEOUT_SEC = 1; // 1 second

   //Helpers
   timespec createTimeout();
   bool     shmLock      ();
   void     shmUnlock    ();
   bool     renderToBitmap();
   bool     resizeShm();

private:
   Video::ShmRenderer* q_ptr;

private Q_SLOTS:
   void timedEvents();
};

}

Video::ShmRendererPrivate::ShmRendererPrivate(Video::ShmRenderer* parent) : QObject(parent), q_ptr(parent),
   m_fd(-1),m_fpsC(0),m_Fps(0),
   m_pShmArea((SHMHeader*)MAP_FAILED), m_ShmAreaLen(0), m_BufferGen(0),
   m_pTimer(nullptr)
#ifdef DEBUG_FPS
   , m_frameCount(0)
   , m_lastFrameDebug(std::chrono::system_clock::now())
#endif
{
}

///Constructor
Video::ShmRenderer::ShmRenderer(const QByteArray& id, const QString& shmPath, const QSize& res): Renderer(id, res), d_ptr(new ShmRendererPrivate(this))
{
   d_ptr->m_ShmPath = shmPath;
   setObjectName("Video::Renderer:"+id);
}

///Destructor
Video::ShmRenderer::~ShmRenderer()
{
   stopShm();
}

///Get the data from shared memory and transform it into a QByteArray
bool
Video::ShmRendererPrivate::renderToBitmap()
{
   QMutexLocker locker {q_ptr->mutex()};

#ifdef Q_OS_LINUX
   auto& renderer = static_cast<Video::Renderer*>(q_ptr)->d_ptr;
   auto& frame = renderer->otherFrame();
   if (frame.isEmpty())
      return false;

   if (!shmLock())
      return false;

   if (m_BufferGen == m_pShmArea->frameGen) {
       shmUnlock();

       // wait for a new frame, max 33ms
       static const struct timespec timeout = {0, 33000000};
       if (::sem_timedwait(&m_pShmArea->frameGenMutex, &timeout) < 0)
           return false;

	   if (!shmLock())
		   return false;
   }

   // valid frame to render?
   if (not m_pShmArea->frameSize)
       return false;

   if (!resizeShm()) {
      qDebug() << "Could not resize shared memory";
      return false;
   }

   if ((unsigned)frame.size() != m_pShmArea->frameSize)
      frame.resize(m_pShmArea->frameSize);
   std::copy_n(m_pShmArea->data + m_pShmArea->readOffset, m_pShmArea->frameSize, frame.data());
   m_BufferGen = m_pShmArea->frameGen;
   renderer->updateFrameIndex();
   shmUnlock();

#ifdef DEBUG_FPS
    auto currentTime = std::chrono::system_clock::now();
    const std::chrono::duration<double> seconds = currentTime - m_lastFrameDebug;
    ++m_frameCount;
    if (seconds.count() > 1) {
        qDebug() << this << ": FPS " << (m_frameCount / seconds.count());
        m_frameCount = 0;
        m_lastFrameDebug = currentTime;
    }
#endif

   return true;
#else
   return false;
#endif
}

///Resize the shared memory
bool
Video::ShmRendererPrivate::resizeShm()
{
    const auto areaSize = sizeof(SHMHeader) + 2 * m_pShmArea->frameSize + 15;
    if (m_ShmAreaLen == areaSize)
        return true;

    shmUnlock();
    if (::munmap(m_pShmArea, m_ShmAreaLen)) {
        qDebug() << "Could not unmap shared area:" << strerror(errno);
        return false;
    }

    m_pShmArea = (SHMHeader*) ::mmap(nullptr, areaSize,
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED, m_fd, 0);
    if (m_pShmArea == MAP_FAILED) {
        qDebug() << "Could not remap shared area";
        return false;
    }

    m_ShmAreaLen = areaSize;
    return shmLock();
}

///Connect to the shared memory
bool Video::ShmRenderer::startShm()
{
   if (d_ptr->m_fd != -1) {
      qDebug() << "fd must be -1";
      return false;
   }

   d_ptr->m_fd = ::shm_open(d_ptr->m_ShmPath.toLatin1(), O_RDWR, 0);
   if (d_ptr->m_fd < 0) {
      qDebug() << "could not open shm area " << d_ptr->m_ShmPath
               << ", shm_open failed:" << strerror(errno);
      return false;
   }

   const auto areaSize = sizeof(SHMHeader);
   d_ptr->m_pShmArea = (SHMHeader*) ::mmap(nullptr, areaSize,
                                           PROT_READ | PROT_WRITE,
                                           MAP_SHARED, d_ptr->m_fd, 0);
   if (d_ptr->m_pShmArea == MAP_FAILED) {
       qDebug() << "Could not remap shared area";
       return false;
   }

   d_ptr->m_ShmAreaLen = areaSize;

   emit started();
   return true;
}

///Disconnect from the shared memory
void Video::ShmRenderer::stopShm()
{
   if (d_ptr->m_fd < 0)
       return;

   ::close(d_ptr->m_fd);
   d_ptr->m_fd = -1;

   if (d_ptr->m_pShmArea == MAP_FAILED)
       return;

   ::munmap(d_ptr->m_pShmArea, d_ptr->m_ShmAreaLen);
   d_ptr->m_ShmAreaLen = 0;
   d_ptr->m_pShmArea = (SHMHeader*) MAP_FAILED;
}

///Lock the memory while the copy is being made
bool Video::ShmRendererPrivate::shmLock()
{
#ifdef Q_OS_LINUX
   return sem_wait(&m_pShmArea->mutex) >= 0;
#else
   return false;
#endif
}

///Remove the lock, allow a new frame to be drawn
void Video::ShmRendererPrivate::shmUnlock()
{
#ifdef Q_OS_LINUX
   sem_post(&m_pShmArea->mutex);
#endif
}


/*****************************************************************************
 *                                                                           *
 *                                   Slots                                   *
 *                                                                           *
 ****************************************************************************/

///Update the buffer
void Video::ShmRendererPrivate::timedEvents()
{
   if (renderToBitmap()) {
      //Compute the FPS shown to the client
      if (m_CurrentTime.second() != QTime::currentTime().second()) {
         m_Fps = m_fpsC;
         m_fpsC=0;
         m_CurrentTime = QTime::currentTime();
      }
      m_fpsC++;

      //emit q_ptr->frameUpdated();
   }
   /*else {
      qDebug() << "Frame dropped";
      usleep(rand()%1000); //Be sure it can come back in sync
   }*/
}

///Start the rendering loop
void Video::ShmRenderer::startRendering()
{
   QMutexLocker locker {mutex()};

   if (!startShm()) {
      qDebug() << "Cannot start rendering on " << d_ptr->m_ShmPath;
      return;
   }

   if (!d_ptr->m_pTimer) {
      d_ptr->m_pTimer = new QTimer(nullptr);

//       m_pTimer->moveToThread(thread());
      connect(d_ptr->m_pTimer,SIGNAL(timeout()),d_ptr.data(),SLOT(timedEvents()));
      d_ptr->m_pTimer->setInterval(30);
   }

   if (!d_ptr->m_pTimer->isActive()) {
      qDebug() << "Is running" << thread()->isRunning();
      d_ptr->m_pTimer->start();
   }
   else
      qDebug() << "Timer already started!";

   setRendering(true);
}

///Stop the rendering loop
void Video::ShmRenderer::stopRendering()
{
   setRendering(false);

   QMutexLocker locker {mutex()};
   qDebug() << "Stopping rendering on" << this;
   if (d_ptr->m_pTimer)
      d_ptr->m_pTimer->stop();
   emit stopped();
   stopShm();
}


/*****************************************************************************
 *                                                                           *
 *                                 Getters                                   *
 *                                                                           *
 ****************************************************************************/

///Get the current frame rate of this renderer
int Video::ShmRenderer::fps() const
{
   return d_ptr->m_Fps;
}

/*****************************************************************************
 *                                                                           *
 *                                 Setters                                   *
 *                                                                           *
 ****************************************************************************/

void Video::ShmRenderer::setShmPath(const QString& path)
{
   d_ptr->m_ShmPath = path;
}

#include <shmrenderer.moc>
