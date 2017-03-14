QT       -= gui

TARGET = gcodelib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += gcode.cpp \
    gline.cpp \
    gmove.cpp

HEADERS += gcode.h \
    gline.h \
    gmove.h \
    gcodelib.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
