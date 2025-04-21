QT     += core gui qml quickcontrols2

TARGET = qtfb
TEMPLATE = lib
CONFIG += shared

xoviextension.target = xovi.cpp
xoviextension.commands = python3 $$(XOVI_REPO)/util/xovigen.py -o xovi.cpp -H xovi.h qtfb.xovi
xoviextension.depends = qtfb.xovi

QMAKE_EXTRA_TARGETS += xoviextension
PRE_TARGETDEPS += xovi.cpp

SOURCES += xovi.cpp main.cpp management.cpp FBController.cpp
HEADERS += FBController.h management.h


