#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "FBController.h"
#include "management.h"
#include "xovi.h"

extern "C" void _xovi_construct()
{
    qtfb::management::start();
    qmlRegisterType<FBController>("net.asivery.Framebuffer", 1, 0, "FBController");
    qt_resource_rebuilder$qmldiff_add_external_diff(r$rootfb, "Framebuffer Root in Navigator");
}
