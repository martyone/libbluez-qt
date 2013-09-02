include(tests_common.pri)

pro_file_basename = $$basename(_PRO_FILE_)
pro_file_basename ~= s/\\.pro$//

TEMPLATE = app
TARGET = $${pro_file_basename}

QT += dbus testlib

HEADERS = testbase.h
SOURCES = $${pro_file_basename}.cpp

INCLUDEPATH += ../bluez-qt
LIBS += -L../bluez-qt -lbluez-qt5

target.path = $${INSTALL_TESTDIR}
INSTALLS += target

check.depends = all
check.commands = '\
    cd "$${OUT_PWD}" \
    && export LD_LIBRARY_PATH="$${OUT_PWD}/../bluez-qt:\$\${LD_LIBRARY_PATH}" \
    && eval \$\$(dbus-launch --sh-syntax) \
    && { ./$${TARGET}; retv=\$\$?; kill \$\${DBUS_SESSION_BUS_PID}; exit \$\${retv}; }'
check.CONFIG = phony
QMAKE_EXTRA_TARGETS += check
