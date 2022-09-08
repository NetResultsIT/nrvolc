
NRVOLC_VERSION=0.0.1


# --- DO NOT CHANGE BELOW THIS LINE ----
message("NrVolChanger Version: $$NRVOLC_VERSION")
VERSION_PARTS = $$split(NRVOLC_VERSION, ".")
MAJOR_RELEASE = $$member(VERSION_PARTS, 0)
MINOR_RELEASE = $$member(VERSION_PARTS, 1)
BUILD_RELEASE = $$member(VERSION_PARTS, 2)

QT -= gui

macx {
    QT += macextras
}

TEMPLATE = lib
DEFINES += NRVOLC_LIB_LIBRARY


#set libversion on unix
unix: VERSION=$$NRVOLC_VERSION

CONFIG += c++11

TARGET = nrvolc

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    VolumeChanger.cpp

HEADERS += \
    lib_global.h \
    VolumeChanger.h

win32 {
    CONFIG(debug, debug|release): LIBSUFFIX=d
    PLATFORM = win32_vs2015
    WINEXT = dll lib exp pdb
    LIBS += ole32.lib
    HEADERS += $$PWD/NrVolumeChangerWin.h
    SOURCES += $$PWD/NrVolumeChangerWin.cpp
}

linux {
    CONFIG(debug, debug|release): LIBSUFFIX=_d

    HEADERS += $$PWD/NrVolumeChangerLinux.h
    SOURCES += $$PWD/NrVolumeChangerLinux.cpp

LIBS += -lasound
}

mac {
    HEADERS += $$PWD/NrVolumeChangerMac.h
    OBJECTIVE_HEADERS += $$PWD/cocoaHelper.h
    OBJECTIVE_SOURCES +=\
      $$PWD/cocoaHelper.mm \
      $$PWD/NrVolumeChangerMac.mm
    LIBS += -framework Foundation
    LIBS += -framework CoreFoundation
    LIBS += -framework CoreAudio
    LIBS += -framework AudioToolbox

    CONFIG(debug, debug|release): LIBSUFFIX=_debug

    CONFIG(release, debug|release) {
        # in order to have the major correctly added to the target linking phase
        VER_MAJ=$$MAJOR_RELEASE
        QMAKE_POST_LINK += "install_name_tool -id @rpath/libnrvolc.$${VER_MAJ}.dylib release/bin/libnrvolc.dylib $$escape_expand(\\n\\t)"
    } else {
        VER_MAJ=$$MAJOR_RELEASE
        QMAKE_POST_LINK += "install_name_tool -id @rpath/libnrvolc_debug.$${VER_MAJ}.dylib debug/bin/libnrvolc_debug.dylib $$escape_expand(\\n\\t)"
    }
}




#setup destination paths
DSTDIR = $$PWD/last_build/
INCLUDE_DIR = $$DSTDIR/include
DLLPATH = ""
INCLUDE_HEADERS = \
    $$PWD/VolumeChanger.h \
    $$PWD/lib_global.h \
    $$PWD/NrVolumeChangerMac.h \
    $$PWD/NrVolumeChangerWin.h \
    $$PWD/NrVolumeChangerLinux.h \


#### DEPLOY ####

# get build type
CONFIG(release, debug|release): BUILDTYPE = release
else: CONFIG(debug, debug|release): BUILDTYPE = debug

CONFIG(debug, debug|release) {
    DLLPATH = "/debug/bin/"
    DLLPATH = $$join(DLLPATH,,$$OUT_PWD,)
}
CONFIG(release, debug|release) {
    DLLPATH = "/release/bin/"
    DLLPATH = $$join(DLLPATH,,$${OUT_PWD},)
}

message("DLLPATH: $${DLLPATH} opwd: $$OUT_PWD  pwd: $$PWD --------------------------")

#add libsuffix to distinguish debug builds
TARGET=$$join(TARGET,,,$${LIBSUFFIX})

#we first get a full fledged dll filename (w/o dot extension)
win32 {
DLL = $$join(TARGET,,$${DLLPATH}/,)
} else {
DLL = $$join(TARGET,,$${DLLPATH}/lib,)
}
#then we add bin to the target to have it generated in the bin/ subfolder (compiler quirks...)
TARGET = $$join(TARGET,,bin/,)
#gcc compiler quirks
!win32 {
TARGET = $$join(TARGET,,$$BUILDTYPE/,)
}


#final copies
unix {
    QMAKE_POST_LINK += $$quote(cp -aP $$join(DLL,,,.*) $${DSTDIR} $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += "mkdir -p $$INCLUDE_DIR $$escape_expand(\\n\\t)"
    QMAKE_POST_LINK += "cp -aP $$INCLUDE_HEADERS $$INCLUDE_DIR $$escape_expand(\\n\\t)"
}
win32 {
    DLL = $$replace(DLL,"/","\\")
    DSTDIR = $$replace(DSTDIR,"/","\\")
    FINALDIR = $$replace(FINALDIR,"/","\\")
    INCLUDE_HEADERS = $$replace(INCLUDE_HEADERS,"/","\\")
    INCLUDE_DIR = "$$DSTDIR\\include"
    QMAKE_POST_LINK+="$$QMAKE_CHK_DIR_EXISTS \"$$INCLUDE_DIR\" $$QMAKE_MKDIR \"$$INCLUDE_DIR\" $$escape_expand(\\n\\t)"
    for(ext, WINEXT):QMAKE_POST_LINK+="$$QMAKE_COPY $$join(DLL,,,.$${ext}) \"$$DSTDIR\" $$escape_expand(\\n\\t)"
    for(vinc, INCLUDE_HEADERS):QMAKE_POST_LINK+="$$QMAKE_COPY $$replace(vinc,"/","\\") \"$$INCLUDE_DIR\" $$escape_expand(\\n\\t)"
}

message("NRVOLC INCLUDEPATH: $$INCLUDEPATH")
message("NRVOLC LIBS: $$LIBS")


