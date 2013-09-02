#include "testbase.h"
#include "moc_testbase.cpp"

#include <QtCore/QScopedPointer>

#include "bluetoothdevice.h"

namespace Tests {

class UtDevice : public BluezTestBase
{
    Q_OBJECT

public:
    UtDevice();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testProperties1_data();
    void testProperties1();
    void testProperties2_data();
    void testProperties2();
    void testWriteProperties_data();
    void testWriteProperties();
    void testReflectPropertiesChange_data();
    void testReflectPropertiesChange();
    void testAudio();
    void testInput();

private:
    static QString deviceDBusProperty2QtProperty(QString property);
    static QString deviceQtProperty2DBusProperty(QString property);

private:
    QPointer<BluetoothDevice> m_adapter0device2;
    QPointer<BluetoothDevice> m_adapter0device4;
};

} // namespace Tests

using namespace Tests;

/*
 * \class Tests::UtDevice
 */

UtDevice::UtDevice()
{
}

void UtDevice::initTestCase()
{
    QVERIFY(waitForService(SERVICE, bus()));

    m_adapter0device2 = new BluetoothDevice(QDBusObjectPath("/adapter0/device2"));
    m_adapter0device4 = new BluetoothDevice(QDBusObjectPath("/adapter0/device4"));
    // TODO: anything (on dbus) to wait for here?
}

void UtDevice::cleanupTestCase()
{
    delete m_adapter0device2;
    delete m_adapter0device4;
}

void UtDevice::testProperties1_data()
{
    QTest::addColumn<QVariant>("expected");

    createTestPropertyData(adapter0device2InitialProperties(), deviceDBusProperty2QtProperty);
    QTest::newRow("path") << QVariant("/adapter0/device2");
}

void UtDevice::testProperties1()
{
    QFETCH(QVariant, expected);

    testProperty(*m_adapter0device2, QTest::currentDataTag(), expected);
}

void UtDevice::testProperties2_data()
{
    QTest::addColumn<QVariant>("expected");

    createTestPropertyData(adapter0device4InitialProperties(), deviceDBusProperty2QtProperty);
    QTest::newRow("path") << QVariant("/adapter0/device4");
}

void UtDevice::testProperties2()
{
    QFETCH(QVariant, expected);

    testProperty(*m_adapter0device4, QTest::currentDataTag(), expected);
}

void UtDevice::testWriteProperties_data()
{
    QTest::addColumn<QVariant>("value1");
    QTest::addColumn<QVariant>("value2");

    QTest::newRow("trusted") << QVariant(false) << QVariant(true);
}

void UtDevice::testWriteProperties()
{
    QFETCH(QVariant, value1);
    QFETCH(QVariant, value2);

    QScopedPointer<BluetoothDevice> adapter0device2alt(
            new BluetoothDevice(QDBusObjectPath("/adapter0/device2")));

    testWriteProperty(m_adapter0device2, adapter0device2alt.data(), QTest::currentDataTag(), value1, value2);
}

void UtDevice::testReflectPropertiesChange_data()
{
    QTest::addColumn<QVariant>("injectedValue1");
    QTest::addColumn<QVariant>("injectedValue2");

    QTest::newRow("connected") << QVariant(false) << QVariant(true);
    QTest::newRow("profiles")
        << QVariant(QStringList())
        << QVariant(QStringList() << BluetoothProfiles::opush);
    QTest::newRow("trusted") << QVariant(false) << QVariant(true);
    QTest::newRow("paired") << QVariant(false) << QVariant(true);
}

void UtDevice::testReflectPropertiesChange()
{
    QFETCH(QVariant, injectedValue1);
    QFETCH(QVariant, injectedValue2);

    const char *property = QTest::currentDataTag();

    QDBusInterface injectionInterface(SERVICE, "/adapter0/device2", DEVICE_INTERFACE, bus());
    const QString injectionMethodNoNotify = "mock_setPropertyNoNotify";
    const QString injectionMethod = "SetProperty";
    const QString injectionKey = deviceQtProperty2DBusProperty(property);

    testReflectPropertyChange(m_adapter0device2, property,
            &injectionInterface, injectionMethodNoNotify, injectionMethod, injectionKey,
            injectedValue1, injectedValue2);
}

void UtDevice::testAudio()
{
    QDBusInterface device(SERVICE, "/adapter0/device2", DEVICE_INTERFACE, bus());

    // Enable A2SINK profile
    {
        QDBusReply<void> reply = device.call("SetProperty", "UUIDs",
                QVariant::fromValue(QDBusVariant(QStringList() << BluetoothProfiles::a2sink)));
        Q_ASSERT(reply.isValid());

        QVERIFY(m_adapter0device2->isProfileSupported(BluetoothProfiles::a2sink));
    }

    QDBusInterface audio(SERVICE, "/adapter0/device2", AUDIO_INTERFACE, bus());

    // Reset audio to disconnected state
    {
        m_adapter0device2->disconnectAudio();

        QDBusReply<QVariantMap> reply = audio.call("GetProperties");
        Q_ASSERT(reply.isValid());
        QCOMPARE(reply.value().value("State"), QVariant("disconnected")); // TODO: verify       

        QCOMPARE(m_adapter0device2->audioConnected(), false);
    }

    SignalSpy propertyChangedSpy(&audio, SIGNAL(PropertyChanged(QString,QDBusVariant)));
    SignalSpy connectedChangedSpy(m_adapter0device2, SIGNAL(audioConnectedChanged(bool)));

    m_adapter0device2->connectAudio();

    QVERIFY(waitForSignal(&propertyChangedSpy));
    QCOMPARE(propertyChangedSpy.count(), 1);
    QCOMPARE(propertyChangedSpy.at(0).at(0), QVariant("State"));
    QCOMPARE(propertyChangedSpy.at(0).at(1).value<QDBusVariant>().variant(), QVariant("connected"));

    QVERIFY(waitForSignal(&connectedChangedSpy));
    QCOMPARE(connectedChangedSpy.count(), 1);
    QCOMPARE(connectedChangedSpy.at(0).at(0).toBool(), true);

    QCOMPARE(m_adapter0device2->audioConnected(), true);
}

void UtDevice::testInput()
{
    QDBusInterface device(SERVICE, "/adapter0/device2", DEVICE_INTERFACE, bus());

    // Enable HID profile
    {
        QDBusReply<void> reply = device.call("SetProperty", "UUIDs",
                QVariant::fromValue(QDBusVariant(QStringList() << BluetoothProfiles::hid)));
        Q_ASSERT(reply.isValid());

        QVERIFY(m_adapter0device2->isProfileSupported(BluetoothProfiles::hid));
    }

    QDBusInterface input(SERVICE, "/adapter0/device2", INPUT_INTERFACE, bus());

    // Reset input to disconnected state
    {
        QDBusReply<void> reply = input.call("Disconnect");
        Q_ASSERT(reply.isValid());

        QCOMPARE(m_adapter0device2->inputConnected(), false);
    }

    SignalSpy propertyChangedSpy(&input, SIGNAL(PropertyChanged(QString,QDBusVariant)));
    SignalSpy connectedChangedSpy(m_adapter0device2, SIGNAL(inputConnectedChanged(bool)));

    m_adapter0device2->connectInput();

    QVERIFY(waitForSignal(&propertyChangedSpy));
    QCOMPARE(propertyChangedSpy.count(), 1);
    QCOMPARE(propertyChangedSpy.at(0).at(0), QVariant("Connected"));
    QCOMPARE(propertyChangedSpy.at(0).at(1).value<QDBusVariant>().variant(), QVariant(true));

    QVERIFY(waitForSignal(&connectedChangedSpy));
    QCOMPARE(connectedChangedSpy.count(), 1);
    QCOMPARE(connectedChangedSpy.at(0).at(0).toBool(), true);

    QCOMPARE(m_adapter0device2->inputConnected(), true);
}

QString UtDevice::deviceDBusProperty2QtProperty(QString property)
{
    if (property == "UUIDs") {
        return "profiles";
    } else if (property == "LegacyPairing") {
        return QString();
    } else {
        return firstLower(property);
    }
}

QString UtDevice::deviceQtProperty2DBusProperty(QString property)
{
    if (property == "profiles") {
        return "UUIDs";
    } else {
        return firstUpper(property);
    }
}

TEST_MAIN(UtDevice, BluezTestBase::ManagerMock)

#include "ut_device.moc"
