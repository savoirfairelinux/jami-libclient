#ifndef SMARTINFOHUB_H_
#define SMARTINFOHUB_H_

#include <iostream>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QMimeData>
#include <QtCore/QItemSelectionModel>
#include <QObject>




class SmartInfoHub
{
    public:

        static void smartInfo(int);
        static SmartInfoHub& instance();
        void setMapInfo(QMap<QString, QString> info);

        /***********************************************************************
         *                                                                     *
         *                            Getter                                   *
         *                                                                     *
         ***********************************************************************/
         float   getLocalFps();
         float   getRemoteFps();
         int     getRemoteWidth();
         int     getRemoteHeight();
         QString getLocalVideoCodec();
         QString getRemoteVideoCodec();
         QString getRemoteAudioCodec();

    signals:

        void updateInfo(int);

    private:
        SmartInfoHub();
        float remoteFps_,localFps_;
        int remoteWidth_, remoteHeight_;
        QString localVideoCodec_, remoteVideoCodec_, remoteAudioCodec_;
};
#endif
