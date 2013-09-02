TEMPLATE = subdirs
SUBDIRS += bluez-qt declarative
CONFIG += ordered

# Adds 'coverage' target
include(coverage.pri)
