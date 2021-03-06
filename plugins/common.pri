# PREFIX              - base prefix for installation (default: /usr/local)
# LIBDIR              - base directory for plugins (default: $$PREFIX/lib)
# NODEBUG             - no debug target

#Qt Version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

MERKAARTOR_SRC_DIR = $$PWD/..

#Static config
include ($$MERKAARTOR_SRC_DIR/src/Config.pri)

DEFINES += VERSION=\"\\\"$$VERSION\\\"\"
DEFINES += REVISION=\"\\\"$$REVISION\\\"\"

!contains(NODEBUG,1) {
    CONFIG += debug
    OBJECTS_DIR += tmp/$$(QMAKESPEC)/obj_debug
}
contains(NODEBUG,1) {
    CONFIG += release
    DEFINES += NDEBUG
    DEFINES += QT_NO_DEBUG_OUTPUT
    OBJECTS_DIR += tmp/$$(QMAKESPEC)/obj_release
}

win32-msvc* {
    DEFINES += _USE_MATH_DEFINES
}

TEMPLATE = lib
CONFIG += plugin

QT += core xml network

#DEFINES += QT_NO_DEBUG_OUTPUT

OUTPUT_DIR = $$MERKAARTOR_SRC_DIR/$$(QMAKESPEC)/binaries
UI_DIR += tmp/$$(QMAKESPEC)
MOC_DIR += tmp/$$(QMAKESPEC)
RCC_DIR += tmp/$$(QMAKESPEC)
DESTDIR = $$OUTPUT_DIR/$$(QMAKESPEC)/bin/plugins

DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/include
LIBS += -L$$OUTPUT_DIR

unix {
    # Prefix: base instalation directory
    isEmpty( PREFIX ) {
        PREFIX = /usr/local
    }
    isEmpty( LIBDIR ) {
        LIBDIR = $${PREFIX}/lib${LIB_SUFFIX}
    }

    target.path = $${LIBDIR}/merkaartor/plugins
    INSTALLS += target
}
