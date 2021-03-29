#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QQmlContext>

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup() {}

public Q_SLOTS:

    void qmlEngineAvailable(QQmlEngine *engine)
    {
        engine->addImportPath("qrc:/tests/qml");
    }
};

QUICK_TEST_MAIN_WITH_SETUP(testqml, Setup)
#include "main.moc"
