TEMPLATE = subdirs
SUBDIRS += bluez-qt declarative
equals(QT_MAJOR_VERSION, 5): SUBDIRS += tests
CONFIG += ordered

# Adds 'coverage' target
include(coverage.pri)
