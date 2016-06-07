/*
 * SmartInfo is an API who will retrieve a lot of information from the daemon.
 *    -Local and remote video codec
 *    -Local and remote frame rate
 *    -Remote audio codec
 *    -Remote resolution
 * The goal is to display all that to the clients.
 * That will help the user to debug his system and/or just show him what
 * happened in the system.
 */

#ifndef SMARTINFOHUB_H_
#define SMARTINFOHUB_H_

#include <iostream>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QMimeData>
#include <QtCore/QItemSelectionModel>
#include <QObject>

class SmartInfoHub final : public QObject
{
    Q_OBJECT
    public:
        /* This method control the update of the variable of this class.
         * The input refreshTimeMS set the update time of the information.
         * If refreshTimeMS=0, al the tool smartInfo stop.
         */
        static void smartInfo(int refreshTimeMS);

        // Signleton
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
        /* Signal emit every tine something change. He contain a list of update
         * You can connect it to a lambda function
         */
        void newInfo(QList<QString>);

    private:
        SmartInfoHub();
        float remoteFps_,localFps_;
        int remoteWidth_, remoteHeight_;
        QString localVideoCodec_, remoteVideoCodec_, remoteAudioCodec_;
};
#endif
