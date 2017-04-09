QT       -= gui

TARGET = gcodelib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += gcode.cpp \
    gline.cpp \
    gmove.cpp \
    gnavigator.cpp \
    gnavigatoritem.cpp

HEADERS += gcode.h \
    gline.h \
    gmove.h \
    gcodelib.h \
    gnavigator.h \
    gnavigatoritem.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
