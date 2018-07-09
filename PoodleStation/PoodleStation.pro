QT += core gui multimedia
greaterThan(QT_MAJOR_VERSION, 4) : QT += widgets

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle

SOURCES += main.cpp \
    emuwindow.cpp \
    emuthread.cpp \
    emulator.cpp \
    cpu.cpp \
    cop0.cpp \
    interpreter.cpp \
    disasm.cpp \
    timers.cpp \
    dma.cpp \
    gpu.cpp \
    cdrom.cpp \
    gte.cpp

HEADERS += \
    emuwindow.hpp \
    emuthread.hpp \
    emulator.hpp \
    cpu.hpp \
    cop0.hpp \
    interpreter.hpp \
    disasm.hpp \
    timers.hpp \
    dma.hpp \
    gpu.hpp \
    cdrom.hpp \
    gte.hpp
