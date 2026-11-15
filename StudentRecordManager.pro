# qmake project file (open this in Qt Creator if you prefer qmake over CMake)
QT       += core gui widgets
CONFIG   += c++17
TARGET    = StudentRecordManager
TEMPLATE  = app
INCLUDEPATH += src

SOURCES += src/main.cpp src/MainWindow.cpp src/StudentStore.cpp
HEADERS += src/MainWindow.h src/StudentStore.h
