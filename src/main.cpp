#include "AppSettings.h"
#include "PerformanceStatus.h"
#include "MemoryController.h"
#include "TabManager.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>

int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/resources/aven-app-icon.png")));
    QCoreApplication::setOrganizationName(QStringLiteral("Aven"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("aven.browser"));
    QCoreApplication::setApplicationName(QStringLiteral("Aven Browser"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.2.1"));

    AppSettings settings;
    TabManager tabs(&settings);
    PerformanceStatus performanceStatus;
    MemoryController memoryController(&tabs, &settings, &performanceStatus);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appSettings"), &settings);
    engine.rootContext()->setContextProperty(QStringLiteral("tabManager"), &tabs);
    engine.rootContext()->setContextProperty(QStringLiteral("performanceStatus"), &performanceStatus);
    engine.rootContext()->setContextProperty(QStringLiteral("memoryController"), &memoryController);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("Aven"), QStringLiteral("Main"));

    return app.exec();
}
