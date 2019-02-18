QT       -= gui

TARGET = gcodelib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += gcode.cpp \
    gmove.cpp \
    gnavigator.cpp \
    gnavigatoritem.cpp \
    gcodeline.cpp

HEADERS += gcode.h \
    gmove.h \
    gcodelib.h \
    gnavigator.h \
    gnavigatoritem.h \
    gcodeline.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
