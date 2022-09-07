QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    MainWindow.h

FORMS += \
    MainWindow.ui

macx {
 CONFIG(debug, debug|release): LIBSUFFIX=_debug
}

win32 {
    CONFIG(debug, debug|release): LIBSUFFIX=d
}

linux {
    CONFIG(debug, debug|release): LIBSUFFIX=_d
}

LIBS += -L$$PWD/../lib/last_build # -lnrvolc$$LIBSUFFIX
INCLUDEPATH += $$PWD/../lib/last_build/include

win32 {
    LIBS += nrvolc$${LIBSUFFIX}.lib
} else {
    LIBS += -lnrvolc$$LIBSUFFIX
}

