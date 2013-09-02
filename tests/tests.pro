include(tests_common.pri)

TEMPLATE = subdirs
SUBDIRS = \
        ut_device.pro \
        ut_agent.pro \
        ut_bluetoothdevicesmodel.pro \
        ut_nearbydevicesmodel.pro \

tests_xml.path = $${INSTALL_TESTDIR}
tests_xml.files = tests.xml
INSTALLS += tests_xml
