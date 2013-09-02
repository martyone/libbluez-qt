#include "testbase.h"
#include "moc_testbase.cpp"

#include "nearbydevicesmodel.h"

namespace Tests {

class UtNearbyDevicesModel : public BluezTestBase
{
    Q_OBJECT

public:
    // see NearbyDevicesModel::m_roleNames
    enum DisplayRole {
        RoleName = 1,
        RoleAlias,
        RoleAddress,
        RoleIcon,
        RoleLegacyPairing,
    };

public:
    UtNearbyDevicesModel();

private slots:
    void initTestCase();
    void init();
    void testSetAdapterProperty();
    void testCreateDevice_data();
    void testCreateDevice();
    void testRemoveDevice_data();
    void testRemoveDevice();
    void testRemoveDefaultAdapter();
    void testReflectDevicePropertyChange_data();
    void testReflectDevicePropertyChange();
    void testPairing();
    void testDiscover_data();
    void testDiscover();

private:
    QPointer<NearbyDevicesModel> m_model;
};

} // namespace Tests

using namespace Tests;

/*
 * \class Tests::UtNearbyDevicesModel
 */

UtNearbyDevicesModel::UtNearbyDevicesModel()
{
}

void UtNearbyDevicesModel::initTestCase()
{
    QVERIFY(waitForService(SERVICE, bus()));

    NearbyDevicesModel model;
    QCOMPARE(model.rowCount(), 2);
}

void UtNearbyDevicesModel::init()
{
    if (m_model != 0) {
        delete m_model;
        QDBusInterface manager(SERVICE, MANAGER_PATH, MANAGER_INTERFACE, bus());
        QDBusReply<void> reply = manager.call("mock_reset");
        Q_ASSERT(reply.isValid());
    }

    m_model = new NearbyDevicesModel();
}

void UtNearbyDevicesModel::testSetAdapterProperty()
{
    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    SignalSpy propertyChangedSpy(&adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)));

    m_model->setAdapterProperty("Powered", false);

    QVERIFY(waitForSignal(&propertyChangedSpy));
    QCOMPARE(propertyChangedSpy.count(), 1);
    QCOMPARE(propertyChangedSpy.at(0).at(0), QVariant("Powered"));
    QCOMPARE(propertyChangedSpy.at(0).at(1).value<QDBusVariant>().variant(), QVariant(false));
}

void UtNearbyDevicesModel::testCreateDevice_data()
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
        properties["LegacyPairing"] = false;

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
        properties["LegacyPairing"] = true;

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
        properties["LegacyPairing"] = false;

        const bool audioConnected = false;
        const bool inputConnected = true;

        QTest::newRow("case3") << "/adapter0" << "/adapter0/device5" << 2
            << properties << audioConnected << inputConnected;
    }
}

void UtNearbyDevicesModel::testCreateDevice()
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
    SignalSpy nearbyDeviceFoundSpy(m_model, SIGNAL(nearbyDeviceFound(int)));

    QDBusReply<void> beginReply = adapter.call("mock_beginAddDevice",
            QVariant::fromValue(QDBusObjectPath(devicePath)));
    Q_ASSERT(beginReply.isValid());

    QDBusInterface device(SERVICE, devicePath, DEVICE_INTERFACE, bus());

    foreach (const QString &name, properties.keys()) {
        QDBusReply<void> reply = device.call("SetProperty", name,
                QVariant::fromValue(QDBusVariant(properties.value(name))));
        Q_ASSERT(reply.isValid());
    }

    // TODO: unused - remove
    if (audioConnected) {
        QDBusInterface audio(SERVICE, devicePath, AUDIO_INTERFACE, bus());
        QDBusReply<void> reply = audio.call("mock_setProperty", "State",
                QVariant::fromValue(QDBusVariant("connected")));
        Q_ASSERT(reply.isValid());
    }

    // TODO: unused - remove
    if (inputConnected) {
        QDBusInterface input(SERVICE, devicePath, INPUT_INTERFACE, bus());
        QDBusReply<void> reply = input.call("mock_setProperty", "Connected",
                QVariant::fromValue(QDBusVariant(true)));
        Q_ASSERT(reply.isValid());
    }

    QDBusReply<void> endReply = adapter.call("mock_endAddDevice");
    Q_ASSERT(endReply.isValid());

    QVERIFY(waitForSignal(&nearbyDeviceFoundSpy));
    QCOMPARE(nearbyDeviceFoundSpy.count(), 1);
    QCOMPARE(nearbyDeviceFoundSpy.at(0).at(0), QVariant(insertedAtRow));

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

    QCOMPARE(m_model->data(index, RoleName          ), properties["Name"]);
    QCOMPARE(m_model->data(index, RoleAlias         ), properties["Alias"]);
    QCOMPARE(m_model->data(index, RoleAddress       ), properties["Address"]);
    QCOMPARE(m_model->data(index, RoleIcon          ), properties["Icon"]);
    QCOMPARE(m_model->data(index, RoleLegacyPairing ), properties["LegacyPairing"]);
}

void UtNearbyDevicesModel::testRemoveDevice_data()
{
    QTest::addColumn<QString>("adapterPath");
    QTest::addColumn<QString>("devicePath");
    QTest::addColumn<int>("removedAtRow");

    QTest::newRow("case1") << "/adapter0" << "/adapter0/device2" << 0;
    QTest::newRow("case2") << "/adapter0" << "/adapter0/device4" << 1;
}

void UtNearbyDevicesModel::testRemoveDevice()
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

void UtNearbyDevicesModel::testRemoveDefaultAdapter()
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

void UtNearbyDevicesModel::testReflectDevicePropertyChange_data()
{
    QTest::addColumn<int>("role");
    QTest::addColumn<QVariant>("newValue");

    QTest::newRow("Name")          << (int)RoleName          << QVariant("deadbeefdead-new-name");
    QTest::newRow("Alias")         << (int)RoleAlias         << QVariant("deadbeefdead-new-alias");
    QTest::newRow("Address")       << (int)RoleAddress       << QVariant("de:ad:be:ee:ee:ef");
    QTest::newRow("Icon")          << (int)RoleIcon          << QVariant("x-dev/deadbeefdead-new-icon");
    QTest::newRow("LegacyPairing") << (int)RoleLegacyPairing << QVariant(true);
}

void UtNearbyDevicesModel::testReflectDevicePropertyChange()
{
    QSKIP("NearbyDevicesModel implementation incomplete - devices are not watched for properties "
            "change");

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

void UtNearbyDevicesModel::testPairing()
{
    const QString internalAgentPath = "/temp/agent";

    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    SignalSpy createPairedDeviceCalledSpy(&adapter,
            SIGNAL(mock_createPairedDeviceCalled(QString,QDBusObjectPath,QString)));

    m_model->pair("be:ef:de:ad:be:ef");

    QVERIFY(waitForSignal(&createPairedDeviceCalledSpy));
    QCOMPARE(createPairedDeviceCalledSpy.count(), 1);
    QCOMPARE(createPairedDeviceCalledSpy.at(0).at(0), QVariant("be:ef:de:ad:be:ef"));
    QCOMPARE(createPairedDeviceCalledSpy.at(0).at(1).value<QDBusObjectPath>().path(),
            internalAgentPath);
    QCOMPARE(createPairedDeviceCalledSpy.at(0).at(2), QVariant(QString()));
}

void UtNearbyDevicesModel::testDiscover_data()
{
    QTest::addColumn<bool>("start");

    QTest::newRow("start") << true;
    QTest::newRow("stop") << false;
}

void UtNearbyDevicesModel::testDiscover()
{
    QFETCH(bool, start);

    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    QDBusReply<void> reply = adapter.call("mock_setPropertyNoNotify", "Discovering",
            QVariant::fromValue(QDBusVariant(!start)));
    Q_ASSERT(reply.isValid());

    SignalSpy propertyChangedSpy(&adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)));

    m_model->discover(start);

    QVERIFY(waitForSignal(&propertyChangedSpy));
    QCOMPARE(propertyChangedSpy.count(), 1);
    QCOMPARE(propertyChangedSpy.at(0).at(0), QVariant("Discovering"));
    QCOMPARE(propertyChangedSpy.at(0).at(1).value<QDBusVariant>().variant(), QVariant(start));
}

TEST_MAIN(UtNearbyDevicesModel, BluezTestBase::ManagerMock)

#include "ut_nearbydevicesmodel.moc"
