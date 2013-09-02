COVERAGE_EXCLUDES = \
    /usr/* \
    */moc_*.cpp \
    */*.moc \
    */qrc_*.cpp \
    /build/* \
    /tests/* \
    /bluez-qt/audio.cpp \
    /bluez-qt/audio.h \
    /bluez-qt/audiosink.cpp \
    /bluez-qt/audiosink.h \
    /bluez-qt/audiosource.cpp \
    /bluez-qt/audiosource.h \
    /bluez-qt/blueadapter.cpp \
    /bluez-qt/blueadapter.h \
    /bluez-qt/bluedevice.cpp \
    /bluez-qt/bluedevice.h \
    /bluez-qt/bluemanager.cpp \
    /bluez-qt/bluemanager.h \
    /bluez-qt/headset.cpp \
    /bluez-qt/headset.h \
    /bluez-qt/input.cpp \
    /bluez-qt/input.h \
    /bluez-qt/obex/obexagent.cpp \
    /bluez-qt/obex/obexagent.h \
    /bluez-qt/obex/obexclient.cpp \
    /bluez-qt/obex/obexclient.h \
    /bluez-qt/obex/obexsession.cpp \
    /bluez-qt/obex/obexsession.h \
    /bluez-qt/obex/obextransfer.cpp \
    /bluez-qt/obex/obextransfer.h \

coverage.CONFIG = phony
coverage.commands = '\
        echo "--- coverage: extracting info" \
        && lcov -c -o lcov.info -d . \
        && echo "--- coverage: excluding $$join(COVERAGE_EXCLUDES, ", ")" \
        && lcov -r lcov.info $$join(COVERAGE_EXCLUDES, "' '", "'", "'") -o lcov.info \
        && echo "--- coverage: generating html" \
        && genhtml -o coverage lcov.info --demangle-cpp \
        && echo -e "--- coverage: done\\n\\n\\tfile://$${OUT_PWD}/coverage/index.html\\n" \
        '

QMAKE_EXTRA_TARGETS += coverage
