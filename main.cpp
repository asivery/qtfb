#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "FBController.h"
#include "management.h"

int main(int argc, char *argv[])
{
    qtfb::management::start();

    QGuiApplication a(argc, argv);
    QQmlApplicationEngine engine;
    qmlRegisterType<FBController>("net.asivery.Framebuffer", 1, 0, "FBController");
    engine.load(QUrl(QStringLiteral("./_start.qml")));

    return a.exec();
}
