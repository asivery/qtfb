#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "FBController.h"
#include "management.h"
#include <iostream>

extern "C" void _ext_load()
{
    qtfb::management::start();
    qmlRegisterType<FBController>("net.asivery.Framebuffer", 1, 0, "FBController");
    CERR << "Loaded framebuffer extension" << std::endl;
}
