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
    SmartInfoHub();
    static void smartInfo(int);
    static void setAnswer(int);
    static int getInformation();
    static void setCallID(const QString callID);

    signals:
    void updateInfo(int);

  private:
    static int answer;
    static QString callID;
};
#endif
