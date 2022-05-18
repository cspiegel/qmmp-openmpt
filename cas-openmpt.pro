QT      += widgets
HEADERS += decoderfactory.h decoder.h metadatamodel.h settingsdialog.h settings.h mptwrap.h
SOURCES += decoder.cpp decoderfactory.cpp metadatamodel.cpp settingsdialog.cpp mptwrap.cpp
FORMS   += settingsdialog.ui

CONFIG += warn_on plugin link_pkgconfig c++14

TEMPLATE = lib

QMAKE_CLEAN += lib$${TARGET}.so

unix {
  CONFIG += link_pkgconfig
  PKGCONFIG += qmmp-1 libopenmpt

  QMMP_PREFIX = $$system(pkg-config qmmp-1 --variable=prefix)
  PLUGIN_DIR = $$system(pkg-config qmmp-1 --variable=plugindir)/Input
  LOCAL_INCLUDES = $${QMMP_PREFIX}/include
  LOCAL_INCLUDES -= $$QMAKE_DEFAULT_INCDIRS
  INCLUDEPATH += $$LOCAL_INCLUDES

  plugin.path = $${PLUGIN_DIR}
  plugin.files = lib$${TARGET}.so
  INSTALLS += plugin
}
