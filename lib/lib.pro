
NRVOLC_VERSION=0.0.2


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

#set libversion on unix
unix: VERSION=$$NRVOLC_VERSION

CONFIG += c++11

TARGET = nrvolc

# Uncomment to compile nrvolc as staticlib
# NOTE: it is not very convenient to compile as static lib
#       since on windows we depend on ole32 that would have to be linked
#       by app or dll linking statically to nrvolc
# NOTE: You would also to define NRVOLC_STATIC in the linking project to avoid
#       expansion of NRVOLC_LIB_EXPORTS to __declspec(dllimport)
#CONFIG += staticlib


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    VolumeChanger.cpp

HEADERS += \
    VolumeChanger.h

win32 {
    # compiler options to generate an external symbol file
    CONFIG(release, debug|release) {
        QMAKE_CXXFLAGS_RELEASE += /Zi
        QMAKE_LFLAGS_RELEASE += /DEBUG
        QMAKE_LFLAGS_RELEASE += /OPT:REF
        QMAKE_LFLAGS_RELEASE += /OPT:ICF
        #ignore (starting from vs2013) the missing pdb files for dependecy libs
        QMAKE_LFLAGS_RELEASE += /ignore:4099
    }

    CONFIG(debug, debug|release): LIBSUFFIX=d
    PLATFORM = win32_vs2015
    WINEXT = lib pdb
    !contains(CONFIG, staticlib) {
        message("Building nrvolc as dynamic library")
        DEFINES += NRVOLC_DLL
        WINEXT += dll exp
    } else {
        message("Building nrvolc as static library")
        DEFINES += NRVOLC_STATIC
        #On windows staticlib release builds the pdb file is in the form vcXXX.pdb
        QMAKE_CXXFLAGS_RELEASE += /Fd$${OUT_PWD}/release/bin/$${TARGET}.pdb
    }
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

