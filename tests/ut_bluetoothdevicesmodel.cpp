#include "testbase.h"
#include "moc_testbase.cpp"

#include "bluetoothdevicemodel.h"

namespace Tests {

class UtBluetoothDevicesModel : public BluezTestBase
{
    Q_OBJECT

public:
    // see BluetoothDevicesModel::m_roleNames
    enum DisplayRole {
        RoleConnected = 1,
        RoleAudioConnected,
        RoleInputConnected,
        RoleTrusted,
        RolePaired,
        RoleProfiles,
        RoleAlias,
        RoleName,
        RoleAddress,
        RoleIcon,
        RolePath,
    };

public:
    UtBluetoothDevicesModel();

private slots:
    void initTestCase();
    void init();
    void testGetDevicePathForName();
    void testGetDeviceForPath();
    void testGetDeviceForHwAddress();
    void testMakePowered();
    void testGetPowered();
    void testMakeDiscoverable();
    void testGetDiscoverable();
    void testSetDiscoverableTimeout();
    void testGetDiscoverableTimeout();
    void testCreateDevice_data();
    void testCreateDevice();
    void testRemoveDevice_data();
    void testRemoveDevice();
    void testRemoveDefaultAdapter();
    void testReflectDevicePropertyChange_data();
    void testReflectDevicePropertyChange();

private:
    QPointer<BluetoothDevicesModel> m_model;
};

} // namespace Tests

using namespace Tests;

/*
 * \class Tests::UtBluetoothDevicesModel
 */

UtBluetoothDevicesModel::UtBluetoothDevicesModel()
{
}

void UtBluetoothDevicesModel::initTestCase()
{
    QVERIFY(waitForService(SERVICE, bus()));

    BluetoothDevicesModel model;
    QVERIFY(model.adapterPresent());
    QCOMPARE(model.rowCount(), 2);
}

void UtBluetoothDevicesModel::init()
{
    if (m_model != 0) {
        delete m_model;
        QDBusInterface manager(SERVICE, MANAGER_PATH, MANAGER_INTERFACE, bus());
        QDBusReply<void> reply = manager.call("mock_reset");
        Q_ASSERT(reply.isValid());
    }

    m_model = new BluetoothDevicesModel();
}

void UtBluetoothDevicesModel::testGetDevicePathForName()
{
    const QString adapter0device2Name = adapter0device2InitialProperties().value("Name").toString();
    const QString adapter0device4Name = adapter0device4InitialProperties().value("Name").toString();
    const QString adapter1device5Name = adapter1device5InitialProperties().value("Name").toString();

    QCOMPARE(m_model->devicePath(adapter0device2Name), QString("/adapter0/device2"));
    QCOMPARE(m_model->devicePath(adapter0device4Name), QString("/adapter0/device4"));
    QCOMPARE(m_model->devicePath(adapter1device5Name), QString());
}

void UtBluetoothDevicesModel::testGetDeviceForPath()
{
    QVERIFY(m_model->device("/adapter0/device2") != 0);
    QCOMPARE(m_model->device("/adapter0/device2")->path(), QString("/adapter0/device2"));
    QVERIFY(m_model->device("/adapter0/device4") != 0);
    QCOMPARE(m_model->device("/adapter0/device4")->path(), QString("/adapter0/device4"));
    QVERIFY(m_model->device("/adapter0/device3") == 0);
    QVERIFY(m_model->device("/adapter1/device5") == 0);
}

void UtBluetoothDevicesModel::testGetDeviceForHwAddress()
{
    const QString adapter0device2Address =
        adapter0device2InitialProperties().value("Address").toString();
    const QString adapter0device4Address =
        adapter0device4InitialProperties().value("Address").toString();
    const QString adapter1device5Address =
        adapter1device5InitialProperties().value("Address").toString();

    QVERIFY(m_model->deviceByHwAddress(adapter0device2Address) != 0);
    QCOMPARE(m_model->deviceByHwAddress(adapter0device2Address)->address(), adapter0device2Address);
    QVERIFY(m_model->deviceByHwAddress(adapter0device4Address) != 0);
    QCOMPARE(m_model->deviceByHwAddress(adapter0device4Address)->address(), adapter0device4Address);
    QVERIFY(m_model->deviceByHwAddress(adapter1device5Address) == 0);
}

void UtBluetoothDevicesModel::testMakePowered()
{
    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    SignalSpy propertyChangedSpy(&adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)));

    m_model->makePowered(false);

    QVERIFY(waitForSignal(&propertyChangedSpy));
    QCOMPARE(propertyChangedSpy.count(), 1);
    QCOMPARE(propertyChangedSpy.at(0).at(0), QVariant("Powered"));
    QCOMPARE(propertyChangedSpy.at(0).at(1).value<QDBusVariant>().variant(), QVariant(false));
}

void UtBluetoothDevicesModel::testGetPowered()
{
    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    QCOMPARE(m_model->powered(), true);

    SignalSpy poweredChangedSpy(m_model, SIGNAL(poweredChanged(bool)));

    QDBusReply<void> reply = adapter.call("SetProperty", "Powered",
            QVariant::fromValue(QDBusVariant(false)));
    Q_ASSERT(reply.isValid());

    QVERIFY(waitForSignal(&poweredChangedSpy));
    QCOMPARE(poweredChangedSpy.count(), 1);
    QCOMPARE(poweredChangedSpy.at(0).at(0), QVariant(false));

    QCOMPARE(m_model->powered(), false);
}

void UtBluetoothDevicesModel::testMakeDiscoverable()
{
    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    SignalSpy propertyChangedSpy(&adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)));

    m_model->makeDiscoverable(false);

    QVERIFY(waitForSignal(&propertyChangedSpy));
    QCOMPARE(propertyChangedSpy.count(), 1);
    QCOMPARE(propertyChangedSpy.at(0).at(0), QVariant("Discoverable"));
    QCOMPARE(propertyChangedSpy.at(0).at(1).value<QDBusVariant>().variant(), QVariant(false));
}

void UtBluetoothDevicesModel::testGetDiscoverable()
{
    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    QCOMPARE(m_model->discoverable(), true);

    SignalSpy discoverableChangedSpy(m_model, SIGNAL(discoverableChanged(bool)));

    QDBusReply<void> reply = adapter.call("SetProperty", "Discoverable",
            QVariant::fromValue(QDBusVariant(false)));
    Q_ASSERT(reply.isValid());

    QVERIFY(waitForSignal(&discoverableChangedSpy));
    QCOMPARE(discoverableChangedSpy.count(), 1);
    QCOMPARE(discoverableChangedSpy.at(0).at(0), QVariant(false));

    QCOMPARE(m_model->discoverable(), false);
}

void UtBluetoothDevicesModel::testSetDiscoverableTimeout()
{
    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    SignalSpy propertyChangedSpy(&adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)));

    m_model->setDiscoverableTimeout(24);

    QVERIFY(waitForSignal(&propertyChangedSpy));
    QCOMPARE(propertyChangedSpy.count(), 1);
    QCOMPARE(propertyChangedSpy.at(0).at(0), QVariant("DiscoverableTimeout"));
    QCOMPARE(propertyChangedSpy.at(0).at(1).value<QDBusVariant>().variant(), QVariant(24));
}

void UtBluetoothDevicesModel::testGetDiscoverableTimeout()
{
    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    QCOMPARE(m_model->discoverableTimeout(), 42);

    SignalSpy discoverableTimeoutChangedSpy(m_model, SIGNAL(discoverableTimeoutChanged(int)));

    QDBusReply<void> reply = adapter.call("SetProperty", "DiscoverableTimeout",
            QVariant::fromValue(QDBusVariant(24)));
    Q_ASSERT(reply.isValid());

    QVERIFY(waitForSignal(&discoverableTimeoutChangedSpy));
    QCOMPARE(discoverableTimeoutChangedSpy.count(), 1);
    QCOMPARE(discoverableTimeoutChangedSpy.at(0).at(0), QVariant(24));

    QCOMPARE(m_model->discoverableTimeout(), 24);
}

void UtBluetoothDevicesModel::testCreateDevice_data()
{
    QTest::addColumn<QString>("adapterPath");
    QTest::addColumn<QString>("devicePath");
    QTest::addColumn<int>("insertedAtRow");
    QTest::addColumn<QVariantMap>("properties");
    QTest::addColumn<bool>("audioConnected");
    QTest::addColumn<bool>("inputConnected");

    {
        QVariantMap properties;
        properties["Address"] = "de:ad:de:ad:de:ad";
        properties["Name"] = "deaddeaddead";
        properties["Icon"] = "x-dev/deaddeaddead";
        properties["UUIDs"] = QStringList() << BluetoothProfiles::a2src;
        properties["Paired"] = true;
        properties["Connected"] = true;
        properties["Trusted"] = false;
        properties["Alias"] = "deaddeaddead-alias";

        const bool audioConnected = false;
        const bool inputConnected = false;

        QTest::newRow("case1") << "/adapter0" << "/adapter0/device1" << 2
            << properties << audioConnected << inputConnected;
    }

    {
        QVariantMap properties;
        properties["Address"] = "de:ad:de:ad:de:ad";
        properties["Name"] = "deaddeaddead";
        properties["Icon"] = "x-dev/deaddeaddead";
        properties["UUIDs"] = QStringList() << BluetoothProfiles::a2sink;
        properties["Paired"] = false;
        properties["Connected"] = false;
        properties["Trusted"] = false;
        properties["Alias"] = "deaddeaddead-alias";

        const bool audioConnected = true;
        const bool inputConnected = false;

        QTest::newRow("case2") << "/adapter0" << "/adapter0/device3" << 2
            << properties << audioConnected << inputConnected;
    }

    {
        QVariantMap properties;
        properties["Address"] = "de:ad:de:ad:de:ad";
        properties["Name"] = "deaddeaddead";
        properties["Icon"] = "x-dev/deaddeaddead";
        properties["UUIDs"] = QStringList() << BluetoothProfiles::hid;
        properties["Paired"] = true;
        properties["Connected"] = false;
        properties["Trusted"] = true;
        properties["Alias"] = "deaddeaddead-alias";

        const bool audioConnected = false;
        const bool inputConnected = true;

        QTest::newRow("case3") << "/adapter0" << "/adapter0/device5" << 2
            << properties << audioConnected << inputConnected;
    }
}

void UtBluetoothDevicesModel::testCreateDevice()
{
    QFETCH(QString, adapterPath);
    QFETCH(QString, devicePath);
    QFETCH(int, insertedAtRow);
    QFETCH(QVariantMap, properties);
    QFETCH(bool, audioConnected);
    QFETCH(bool, inputConnected);

    QDBusInterface adapter(SERVICE, adapterPath, ADAPTER_INTERFACE, bus());

    SignalSpy rowsAboutToBeInsertedSpy(m_model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    SignalSpy rowsInsertedSpy(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QDBusReply<void> beginReply = adapter.call("mock_beginAddDevice",
            QVariant::fromValue(QDBusObjectPath(devicePath)));
    Q_ASSERT(beginReply.isValid());

    QDBusInterface device(SERVICE, devicePath, DEVICE_INTERFACE, bus());

    foreach (const QString &name, properties.keys()) {
        QDBusReply<void> reply = device.call("SetProperty", name,
                QVariant::fromValue(QDBusVariant(properties.value(name))));
        Q_ASSERT(reply.isValid());
    }

    if (audioConnected) {
        QDBusInterface audio(SERVICE, devicePath, AUDIO_INTERFACE, bus());
        QDBusReply<void> reply = audio.call("mock_setProperty", "State",
                QVariant::fromValue(QDBusVariant("connected")));
        Q_ASSERT(reply.isValid());
    }

    if (inputConnected) {
        QDBusInterface input(SERVICE, devicePath, INPUT_INTERFACE, bus());
        QDBusReply<void> reply = input.call("mock_setProperty", "Connected",
                QVariant::fromValue(QDBusVariant(true)));
        Q_ASSERT(reply.isValid());
    }

    QDBusReply<void> endReply = adapter.call("mock_endAddDevice");
    Q_ASSERT(endReply.isValid());

    QVERIFY(waitForSignal(&rowsAboutToBeInsertedSpy));
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 1);
    QCOMPARE(rowsAboutToBeInsertedSpy.at(0).at(0), QVariant(QModelIndex()));
    QCOMPARE(rowsAboutToBeInsertedSpy.at(0).at(1), QVariant(insertedAtRow));
    QCOMPARE(rowsAboutToBeInsertedSpy.at(0).at(2), QVariant(insertedAtRow));

    QVERIFY(waitForSignal(&rowsInsertedSpy));
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.at(0).at(0), QVariant(QModelIndex()));
    QCOMPARE(rowsInsertedSpy.at(0).at(1), QVariant(insertedAtRow));
    QCOMPARE(rowsInsertedSpy.at(0).at(2), QVariant(insertedAtRow));

    QCOMPARE(m_model->rowCount(), 3);

    const QModelIndex index = m_model->index(insertedAtRow, 0);

    QCOMPARE(m_model->data(index, RoleConnected     ), properties["Connected"]);
    QCOMPARE(m_model->data(index, RoleAudioConnected), QVariant(audioConnected));
    QCOMPARE(m_model->data(index, RoleInputConnected), QVariant(inputConnected));
    QCOMPARE(m_model->data(index, RoleTrusted       ), properties["Trusted"]);
    QCOMPARE(m_model->data(index, RolePaired        ), properties["Paired"]);
    QCOMPARE(m_model->data(index, RoleProfiles      ), properties["UUIDs"]);
    QCOMPARE(m_model->data(index, RoleAlias         ), properties["Alias"]);
    QCOMPARE(m_model->data(index, RoleName          ), properties["Name"]);
    QCOMPARE(m_model->data(index, RoleAddress       ), properties["Address"]);
    QCOMPARE(m_model->data(index, RoleIcon          ), properties["Icon"]);
    QCOMPARE(m_model->data(index, RolePath          ), QVariant(devicePath));
}

void UtBluetoothDevicesModel::testRemoveDevice_data()
{
    QTest::addColumn<QString>("adapterPath");
    QTest::addColumn<QString>("devicePath");
    QTest::addColumn<int>("removedAtRow");

    QTest::newRow("case1") << "/adapter0" << "/adapter0/device2" << 0;
    QTest::newRow("case2") << "/adapter0" << "/adapter0/device4" << 1;
}

void UtBluetoothDevicesModel::testRemoveDevice()
{
    QFETCH(QString, adapterPath);
    QFETCH(QString, devicePath);
    QFETCH(int, removedAtRow);

    QDBusInterface adapter(SERVICE, adapterPath, ADAPTER_INTERFACE, bus());

    SignalSpy rowsAboutToBeRemovedSpy(m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    SignalSpy rowsRemovedSpy(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QDBusReply<void> reply = adapter.call("mock_removeDevice",
            QVariant::fromValue(QDBusObjectPath(devicePath)));
    Q_ASSERT(reply.isValid());

    QVERIFY(waitForSignal(&rowsAboutToBeRemovedSpy));
    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 1);
    QCOMPARE(rowsAboutToBeRemovedSpy.at(0).at(0), QVariant(QModelIndex()));
    QCOMPARE(rowsAboutToBeRemovedSpy.at(0).at(1), QVariant(removedAtRow));
    QCOMPARE(rowsAboutToBeRemovedSpy.at(0).at(2), QVariant(removedAtRow));

    QVERIFY(waitForSignal(&rowsRemovedSpy));
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.at(0).at(0), QVariant(QModelIndex()));
    QCOMPARE(rowsRemovedSpy.at(0).at(1), QVariant(removedAtRow));
    QCOMPARE(rowsRemovedSpy.at(0).at(2), QVariant(removedAtRow));

    QCOMPARE(m_model->rowCount(), 1);
}

void UtBluetoothDevicesModel::testRemoveDefaultAdapter()
{
    QDBusInterface manager(SERVICE, MANAGER_PATH, MANAGER_INTERFACE, bus());

    SignalSpy rowsAboutToBeRemovedSpy(m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    SignalSpy rowsRemovedSpy(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    SignalSpy rowsAboutToBeInsertedSpy(m_model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    SignalSpy rowsInsertedSpy(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QDBusReply<void> reply = manager.call("mock_removeAdapter",
            QVariant::fromValue(QDBusObjectPath("/adapter0")));
    Q_ASSERT(reply.isValid());

    QVERIFY(waitForSignal(&rowsAboutToBeRemovedSpy));
    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 1);
    QCOMPARE(rowsAboutToBeRemovedSpy.at(0).at(0), QVariant(QModelIndex()));
    QCOMPARE(rowsAboutToBeRemovedSpy.at(0).at(1), QVariant(0));
    QCOMPARE(rowsAboutToBeRemovedSpy.at(0).at(2), QVariant(1));

    QVERIFY(waitForSignal(&rowsRemovedSpy));
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.at(0).at(0), QVariant(QModelIndex()));
    QCOMPARE(rowsRemovedSpy.at(0).at(1), QVariant(0));
    QCOMPARE(rowsRemovedSpy.at(0).at(2), QVariant(1));

    QVERIFY(waitForSignal(&rowsAboutToBeInsertedSpy));
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 1);
    QCOMPARE(rowsAboutToBeInsertedSpy.at(0).at(0), QVariant(QModelIndex()));
    QCOMPARE(rowsAboutToBeInsertedSpy.at(0).at(1), QVariant(0));
    QCOMPARE(rowsAboutToBeInsertedSpy.at(0).at(2), QVariant(0));

    QVERIFY(waitForSignal(&rowsInsertedSpy));
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.at(0).at(0), QVariant(QModelIndex()));
    QCOMPARE(rowsInsertedSpy.at(0).at(1), QVariant(0));
    QCOMPARE(rowsInsertedSpy.at(0).at(2), QVariant(0));

    QCOMPARE(m_model->rowCount(), 1);
}

void UtBluetoothDevicesModel::testReflectDevicePropertyChange_data()
{
    QTest::addColumn<int>("role");
    QTest::addColumn<QVariant>("newValue");

    QTest::newRow("Connected") << (int)RoleConnected << QVariant(true);
    QTest::newRow("Trusted")   << (int)RoleTrusted   << QVariant(true);
    QTest::newRow("Paired")    << (int)RolePaired    << QVariant(true);
    QTest::newRow("UUIDs")     << (int)RoleProfiles
        << QVariant(QStringList() << BluetoothProfiles::a2sink);
    QTest::newRow("Alias")     << (int)RoleAlias     << QVariant("deadbeefdead-new-alias");
    QTest::newRow("Name")      << (int)RoleName      << QVariant("deadbeefdead-new-name");
    QTest::newRow("Address")   << (int)RoleAddress   << QVariant("de:ad:be:ee:ee:ef");
    QTest::newRow("Icon")      << (int)RoleIcon      << QVariant("x-dev/deadbeefdead-new-icon");
}

void UtBluetoothDevicesModel::testReflectDevicePropertyChange()
{
    QFETCH(int, role);
    QFETCH(QVariant, newValue);

    const QString propertyName = QTest::currentDataTag();

    QDBusInterface device(SERVICE, "/adapter0/device2", DEVICE_INTERFACE, bus());

    SignalSpy dataChangedSpy(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

    QDBusReply<void> reply = device.call("SetProperty", propertyName,
            QVariant::fromValue(QDBusVariant(newValue)));
    Q_ASSERT(reply.isValid());

    QVERIFY(waitForSignal(&dataChangedSpy));
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.at(0).at(0).value<QModelIndex>().row(), 0);
    QCOMPARE(dataChangedSpy.at(0).at(0).value<QModelIndex>().column(), 0);
    QCOMPARE(dataChangedSpy.at(0).at(0).value<QModelIndex>().data(role), QVariant(newValue));
}

TEST_MAIN(UtBluetoothDevicesModel, BluezTestBase::ManagerMock)

#include "ut_bluetoothdevicesmodel.moc"
