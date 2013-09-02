#ifndef TESTBASE_H
#define TESTBASE_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusServiceWatcher>
#include <QtTest/QSignalSpy>
#include <QTest>

#include "btprofiles.h"

Q_DECLARE_METATYPE(QDBusPendingCallWatcher*) // needed by waitForSignal

namespace Tests {

#define TEST_MAIN(TestClass, MockClass)                       \
    int main(int argc, char *argv[])                          \
    {                                                         \
        return Tests::main<TestClass, MockClass>(argc, argv); \
    }                                                         \

class TestBase : public QObject
{
    Q_OBJECT

public:
    class SignalSpy;
    typedef QList<SignalSpy *> SignalSpyList;

    enum {
        SIGNAL_WAIT_TIMEOUT = 5000, // [ms]
    };

public:
    TestBase();

public:
    static void mockMessageHandler(QtMsgType type, const QMessageLogContext &context,
            const QString &message);

    static bool waitForSignal(QObject *object, const char *signal);
    static bool waitForSignal(SignalSpy *signalSpy);
    static bool waitForSignals(const SignalSpyList &signalSpies);
    static bool waitForService(const QString &serviceName, const QDBusConnection &bus);

    static void createTestPropertyData(const QVariantMap &expected,
            QString (*dbusProperty2QtProperty)(QString));
    static void testProperty(const QObject &object, const QString &property,
            const QVariant &expected);
    static void testWriteProperty(QObject *object, QObject *otherObject, const char *property,
            const QVariant &value1, const QVariant &value2);
    static void testReflectPropertyChange(QObject *object, const char *property,
            QDBusInterface *injectionInterface,
            const QString &injectionMethodNoNotify, const QString &injectionMethod,
            const QString &injectionKey, const QVariant &injectedValue1,
            const QVariant &injectedValue2);

    static QString firstLower(QString s) { return s.replace(0, 1, s.at(0).toLower()); }
    static QString firstUpper(QString s) { return s.replace(0, 1, s.at(0).toUpper()); }
    static QByteArray notifySignal(const QObject &object, const char *property);
};

class BluezTestBase : public TestBase
{
    Q_OBJECT

public:
    class ManagerMock;
    class AdapterMock;
    class DeviceMock;

    static const QString SERVICE;

    static const QString MANAGER_PATH;

    static const QString ADAPTER_INTERFACE;
    static const QString AGENT_INTERFACE;
    static const QString AUDIO_INTERFACE;
    static const QString DEVICE_INTERFACE;
    static const QString INPUT_INTERFACE;
    static const QString MANAGER_INTERFACE;

public:
    BluezTestBase();

public:
    static QDBusConnection bus() { return QDBusConnection::systemBus(); }

    static QVariantMap adapter0device2InitialProperties();
    static QVariantMap adapter0device4InitialProperties();
    static QVariantMap adapter1device5InitialProperties();
};

/*
 * QSignalSpy is inherited to fix two issues when used together with QDBusInterface:
 *
 * 1. QDBusInterface relies on QObject::connectNotify() to connect the D-Bus signal immediately when
 *    QObject::connect() is called. QSignalSpy uses internal version of QObject::connect() which
 *    does not call connectNotify(). This issue is fixed as a side effect of the fix to the second
 *    issue -- otherwise it would be necessary to invoke connectNotify() explicitely upon SignalSpy
 *    construction.
 * 2. TestBase::waitForSignals() uses combination of SignalSpy and QEventLoop to wait for signals.
 *    It connects the signals to QEventLoop::quit(). When used together with QDBusInterface, it is
 *    usually too late to connect -- that is why the signalEmitted() signal exists.
 */
class TestBase::SignalSpy : public QSignalSpy
{
    Q_OBJECT

public:
    SignalSpy(QObject *object, const char *signal)
        : QSignalSpy(object, signal)
    {
        connect(object, signal, this, SIGNAL(signalEmitted()));
    }

signals:
    void signalEmitted();
};

class BluezTestBase::ManagerMock : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Manager")

public:
    typedef BluezTestBase TestBase; // required by template<> main()

public:
    ManagerMock();

public:
    Q_SCRIPTABLE QDBusObjectPath DefaultAdapter() const { return m_defaultAdapter; }
    Q_SCRIPTABLE QVariantMap GetProperties() const;

    // mock API
    Q_SCRIPTABLE void mock_reset();
    Q_SCRIPTABLE void mock_beginAddAdapter(const QDBusObjectPath &path);
    Q_SCRIPTABLE void mock_endAddAdapter();
    Q_SCRIPTABLE void mock_removeAdapter(const QDBusObjectPath &path);
    Q_SCRIPTABLE void mock_setDefaultAdapter(const QDBusObjectPath &path);

signals:
    Q_SCRIPTABLE void PropertyChanged(const QString &key, const QDBusVariant &value);
    Q_SCRIPTABLE void AdapterAdded(const QDBusObjectPath &path);
    Q_SCRIPTABLE void AdapterRemoved(const QDBusObjectPath &path);
    Q_SCRIPTABLE void DefaultAdapterChanged(const QDBusObjectPath &defaultAdapterPath);

private:
    QVariantMap m_properties;
    QPointer<AdapterMock> m_newAdapter;
    QMap<QDBusObjectPath, AdapterMock *> m_adapters;
    QDBusObjectPath m_defaultAdapter;
};

class BluezTestBase::AdapterMock : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Adapter")

public:
    AdapterMock(const QString &path, QObject *parent);
    ~AdapterMock();
    static AdapterMock *instance(const QString &path) { return s_instances.value(path); }

public:
    QString path() const { return m_path; }

    Q_SCRIPTABLE QVariantMap GetProperties() const;
    Q_SCRIPTABLE void SetProperty(const QString &key, const QDBusVariant &value);

    Q_SCRIPTABLE void StartDiscovery();
    Q_SCRIPTABLE void StopDiscovery();
    Q_SCRIPTABLE QList<QDBusObjectPath> ListDevices() const;

    Q_SCRIPTABLE void RegisterAgent(const QDBusObjectPath &agentPath, const QString &capability,
            const QDBusMessage &message);

    Q_SCRIPTABLE void CreatePairedDevice(const QString &deviceAddress,
            const QDBusObjectPath &agentPath, const QString &capability);

    // mock API
    Q_SCRIPTABLE void mock_setPropertyNoNotify(const QString &key, const QDBusVariant &value);
    Q_SCRIPTABLE void mock_beginAddDevice(const QDBusObjectPath &path);
    Q_SCRIPTABLE void mock_endAddDevice();
    Q_SCRIPTABLE void mock_removeDevice(const QDBusObjectPath &path);

    Q_SCRIPTABLE bool mock_requestConfirmation(const QDBusObjectPath &devicePath, uint key,
            const QDBusMessage &message);
    Q_SCRIPTABLE uint mock_requestPasskey(const QDBusObjectPath &devicePath,
            const QDBusMessage &message);
    Q_SCRIPTABLE QString mock_requestPinCode(const QDBusObjectPath &devicePath,
            const QDBusMessage &message);

signals:
    Q_SCRIPTABLE void PropertyChanged(const QString &key, const QDBusVariant &value);
    Q_SCRIPTABLE void DeviceFound(const QString &address, const QVariantMap &properties);
    Q_SCRIPTABLE void DeviceCreated(const QDBusObjectPath &path);
    Q_SCRIPTABLE void DeviceRemoved(const QDBusObjectPath &path);
    Q_SCRIPTABLE void DeviceDisappeared(const QString &address);

    // mock API
    Q_SCRIPTABLE void mock_agentRegistered(const QDBusObjectPath &devicePath);
    Q_SCRIPTABLE void mock_createPairedDeviceCalled(const QString &deviceAddres,
            const QDBusObjectPath &agentPath, const QString &capability);

private:
    static QMap<QString, AdapterMock *> s_instances;
    const QString m_path;
    QVariantMap m_properties;
    QPointer<DeviceMock> m_newDevice;
    QMap<QDBusObjectPath, DeviceMock *> m_devices;
    QPointer<QDBusInterface> m_agent;
};

class BluezTestBase::DeviceMock : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Device")

public:
    class AudioAdaptor;
    class InputAdaptor;

public:
    DeviceMock(const QString &path, QObject *parent);
    ~DeviceMock();
    static DeviceMock *instance(const QString &path) { return s_instances.value(path); }

public:
    QString path() const { return m_path; }
    AudioAdaptor *audio() { return m_audioAdaptor; }
    InputAdaptor *input() { return m_inputAdaptor; }

    Q_SCRIPTABLE QVariantMap GetProperties() const { return m_properties; }
    Q_SCRIPTABLE void SetProperty(const QString &key, const QDBusVariant &value);

    // mock API
    Q_SCRIPTABLE void mock_setPropertyNoNotify(const QString &key, const QDBusVariant &value);

signals:
    Q_SCRIPTABLE void PropertyChanged(const QString &key, const QDBusVariant &value);

    // mock API

private:
    static QMap<QString, DeviceMock *> s_instances;
    const QString m_path;
    const QPointer<AudioAdaptor> m_audioAdaptor;
    const QPointer<InputAdaptor> m_inputAdaptor;
    QVariantMap m_properties;
};

class BluezTestBase::DeviceMock::AudioAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Audio")

public:
    AudioAdaptor(DeviceMock *device) : QDBusAbstractAdaptor(device) { }

public slots:
    void Connect() { mock_setProperty("State", QDBusVariant("connected")); }
    void Disconnect() { mock_setProperty("State", QDBusVariant("disconnected")); }
    QVariantMap GetProperties() const { return m_properties; }

    // mock API
    void mock_setProperty(const QString &name, const QDBusVariant &value)
    {
        if (m_properties.value(name) != value.variant()) {
            m_properties[name] = value.variant();
            emit PropertyChanged(name, value);
        }
    }

signals:
    void PropertyChanged(const QString &name, const QDBusVariant &value);

private:
    QVariantMap m_properties;
};

class BluezTestBase::DeviceMock::InputAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Input")

public:
    InputAdaptor(DeviceMock *device) : QDBusAbstractAdaptor(device) { }

public slots:
    void Connect() { mock_setProperty("Connected", QDBusVariant(true)); }
    void Disconnect() { mock_setProperty("Connected", QDBusVariant(false)); }
    QVariantMap GetProperties() const { return m_properties; }

    // mock API
    void mock_setProperty(const QString &name, const QDBusVariant &value)
    {
        if (m_properties.value(name) != value.variant()) {
            m_properties[name] = value.variant();
            emit PropertyChanged(name, value);
        }
    }

signals:
    void PropertyChanged(const QString &name, const QDBusVariant &value);

private:
    QVariantMap m_properties;
};

/*
 * \class Tests::TestBase
 */

TestBase::TestBase()
{
}

void TestBase::mockMessageHandler(QtMsgType type, const QMessageLogContext &context,
    const QString &message)
{
    qInstallMessageHandler(0);
    qt_message_output(type, context, QString("MOCK   : ").append(message));
    qInstallMessageHandler(mockMessageHandler);
}

bool TestBase::waitForSignal(QObject *object, const char *signal)
{
    SignalSpy spy(object, signal);
    return waitForSignals(SignalSpyList() << &spy);
}

bool TestBase::waitForSignal(SignalSpy *signalSpy)
{
    return waitForSignals(SignalSpyList() << signalSpy);
}

bool TestBase::waitForSignals(const SignalSpyList &signalSpies)
{
    struct H
    {
        static bool allReceived(const SignalSpyList &signalSpies)
        {
            foreach (SignalSpy *signalSpy, signalSpies) {
                if (signalSpy->count() == 0) {
                    return false;
                }
            }
            return true;
        }
    };

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    connect(&timeoutTimer, SIGNAL(timeout()), &loop, SLOT(quit()));
    foreach (SignalSpy *signalSpy, signalSpies) {
        connect(signalSpy, SIGNAL(signalEmitted()), &loop, SLOT(quit()));
    }

    timeoutTimer.start(SIGNAL_WAIT_TIMEOUT);

    while (!H::allReceived(signalSpies)) {
        loop.exec();
        if (!timeoutTimer.isActive()) {
            return false;
        }
    }

    return true;
}

bool TestBase::waitForService(const QString &serviceName, const QDBusConnection &bus)
{
    if (bus.interface()->registeredServiceNames().value().contains(serviceName)) {
        return true;
    }

    QDBusServiceWatcher watcher(serviceName, bus, QDBusServiceWatcher::WatchForRegistration);
    SignalSpy spy(&watcher, SIGNAL(serviceRegistered(QString)));

    return waitForSignal(&spy);
}

void TestBase::createTestPropertyData(const QVariantMap &expected,
        QString (*dbusProperty2QtProperty)(QString))
{
    QMapIterator<QString, QVariant> it(expected);

    while (it.hasNext()) {
        it.next();

        const QString qtProperty = (dbusProperty2QtProperty)(it.key());
        if (qtProperty.isEmpty()) {
            continue;
        }

        if (it.value().canConvert(QVariant::Map)) {
            const QVariantMap mapTypeProperty = it.value().toMap();
            QMapIterator<QString, QVariant> it2(mapTypeProperty);
            while (it2.hasNext()) {
                it2.next();

                QTest::newRow(qPrintable(QString("%1/%2").arg(qtProperty).arg(it2.key())))
                    << it2.value();
            }
        } else {
            QTest::newRow(qPrintable(qtProperty)) << it.value();
        }
    }
}

void TestBase::testProperty(const QObject &object, const QString &property,
        const QVariant &expected)
{
    Q_ASSERT(property.count('/') <= 1);

    const QString propertyName = QString(property).section('/', 0, 0);
    const QString variantMapKey = QString(property).section('/', 1, 1);

    QVERIFY(object.property(qPrintable(propertyName)).isValid());

    if (variantMapKey.isEmpty()) {
        QVERIFY(object.property(qPrintable(propertyName)).type() != QVariant::Map);
        QCOMPARE(object.property(qPrintable(propertyName)), expected);
    } else {
        QCOMPARE(object.property(qPrintable(propertyName)).type(), QVariant::Map);
        QVERIFY(object.property(qPrintable(propertyName)).toMap().contains(variantMapKey));
        QCOMPARE(object.property(qPrintable(propertyName)).toMap()[variantMapKey], expected);
    }
}

void TestBase::testWriteProperty(QObject *object, QObject *otherObject, const char *property,
        const QVariant &value1, const QVariant &value2)
{
    Q_ASSERT(!QString(property).contains('/'));

    QVERIFY(object->setProperty(property, value1));

    const QByteArray notifySignal = TestBase::notifySignal(*object, property);

    SignalSpy spy(object, notifySignal);
    SignalSpy otherSpy(otherObject, notifySignal);

    QVERIFY(object->setProperty(property, value2));

    QVERIFY(waitForSignals(SignalSpyList() << &spy << &otherSpy));

    // TODO: test signal argument if available
    QVERIFY(object->property(property).isValid());
    QCOMPARE(object->property(property), value2);
    QVERIFY(otherObject->property(property).isValid());
    QCOMPARE(otherObject->property(property), value2);
}

void TestBase::testReflectPropertyChange(QObject *object, const char *property,
        QDBusInterface *injectionInterface,
        const QString &injectionMethodNoNotify, const QString &injectionMethod,
        const QString &injectionKey, const QVariant &injectedValue1, const QVariant &injectedValue2)
{
    Q_ASSERT(!QString(property).contains('/'));

    QDBusReply<void> reply1 = injectionInterface->call(injectionMethodNoNotify, injectionKey,
            QVariant::fromValue(QDBusVariant(injectedValue1)));
    Q_ASSERT(reply1.isValid());

    const QByteArray notifySignal = TestBase::notifySignal(*object, property);

    SignalSpy spy(object, notifySignal);

    QDBusPendingReply<> reply2 = injectionInterface->asyncCall(injectionMethod, injectionKey,
            QVariant::fromValue(QDBusVariant(injectedValue2)));
    Q_ASSERT(!reply2.isError());

    QVERIFY(waitForSignal(&spy));
    QCOMPARE(spy.count(), 1);

    QVERIFY(object->property(property).isValid());
    QCOMPARE(object->property(property), injectedValue2);

    reply2.waitForFinished();
    Q_ASSERT(reply2.isValid());
}

QByteArray TestBase::notifySignal(const QObject &object, const char *property)
{
    Q_ASSERT(object.metaObject()->indexOfProperty(property) != -1);
    Q_ASSERT(object.metaObject()->property(object.metaObject()->indexOfProperty(property))
            .hasNotifySignal());

    return QByteArray().append('0' + QSIGNAL_CODE).append(object.metaObject()->property(
                object.metaObject()->indexOfProperty(property)
                ).notifySignal().methodSignature());
}

/*
 * \class Tests::BluezTestBase
 */

const QString BluezTestBase::SERVICE = "org.bluez";

const QString BluezTestBase::MANAGER_PATH = "/";

const QString BluezTestBase::ADAPTER_INTERFACE = "org.bluez.Adapter";
const QString BluezTestBase::AGENT_INTERFACE = "org.bluez.Agent";
const QString BluezTestBase::AUDIO_INTERFACE = "org.bluez.Audio";
const QString BluezTestBase::DEVICE_INTERFACE = "org.bluez.Device";
const QString BluezTestBase::INPUT_INTERFACE = "org.bluez.Input";
const QString BluezTestBase::MANAGER_INTERFACE = "org.bluez.Manager";

BluezTestBase::BluezTestBase()
{
}

QVariantMap BluezTestBase::adapter0device2InitialProperties()
{
    static QVariantMap properties;
    static bool initialized = false;

    if (initialized) {
        return properties;
    }

    properties["Address"] = "de:ad:be:ef:de:ad";
    properties["Name"] = "deadbeefdead";
    properties["Icon"] = "x-dev/deadbeefdead";
    properties["UUIDs"] = QStringList() << BluetoothProfiles::hid;
    properties["Paired"] = false;
    properties["Connected"] = false;
    properties["Trusted"] = false;
    properties["Alias"] = "deadbeefdead-alias";
    properties["LegacyPairing"] = false;

    initialized = true;
    return properties;
}

QVariantMap BluezTestBase::adapter0device4InitialProperties()
{
    static QVariantMap properties;
    static bool initialized = false;

    if (initialized) {
        return properties;
    }

    properties["Address"] = "de:ad:be:ef:be:ef";
    properties["Name"] = "deadbeefbeef";
    properties["Icon"] = "x-dev/deadbeefbeef";
    properties["UUIDs"] = QStringList() << BluetoothProfiles::a2sink;
    properties["Paired"] = false;
    properties["Connected"] = false;
    properties["Trusted"] = false;
    properties["Alias"] = "deadbeefbeef-alias";
    properties["LegacyPairing"] = true;

    initialized = true;
    return properties;
}

QVariantMap BluezTestBase::adapter1device5InitialProperties()
{
    static QVariantMap properties;
    static bool initialized = false;

    if (initialized) {
        return properties;
    }

    properties["Address"] = "be:ef:be:ef:be:ef";
    properties["Name"] = "beefbeefbeef";
    properties["Icon"] = "beefbeefbeef";
    properties["UUIDs"] = QStringList() << BluetoothProfiles::a2src;
    properties["Paired"] = true;
    properties["Connected"] = true;
    properties["Trusted"] = true;
    properties["Alias"] = "beefbeefbeef-alias";
    properties["LegacyPairing"] = false;

    initialized = true;
    return properties;
}

/*
 * \class Tests::BluezTestBase::ManagerMock
 */

BluezTestBase::ManagerMock::ManagerMock()
    : QObject(0)
{
    if (!bus().registerObject(MANAGER_PATH, this, QDBusConnection::ExportScriptableContents)) {
        qFatal("Failed to register ManagerMock to D-Bus at path '%s': '%s'",
                qPrintable(MANAGER_PATH), qPrintable(bus().lastError().message()));
    }

    mock_reset();
}

QVariantMap BluezTestBase::ManagerMock::GetProperties() const
{
    QVariantMap retv = m_properties;

    retv.insert("Adapters", QVariant::fromValue(m_adapters.keys()));

    return retv;
}

void BluezTestBase::ManagerMock::mock_reset()
{
    while (!m_adapters.isEmpty()) {
        mock_removeAdapter(m_adapters.keys().first());
    }

    mock_beginAddAdapter(QDBusObjectPath("/adapter0"));
    AdapterMock *const adapter1 = AdapterMock::instance("/adapter0");

    adapter1->SetProperty("Powered", QDBusVariant(true));
    adapter1->SetProperty("Discoverable", QDBusVariant(true));
    adapter1->SetProperty("DiscoverableTimeout", QDBusVariant(42));

    adapter1->mock_beginAddDevice(QDBusObjectPath("/adapter0/device2"));
    {
        DeviceMock *const device = DeviceMock::instance("/adapter0/device2");

        foreach (const QString &key, adapter0device2InitialProperties().keys()) {
            device->SetProperty(key, QDBusVariant(adapter0device2InitialProperties().value(key)));
        }

        device->input()->mock_setProperty("Connected", QDBusVariant(true));

    }
    adapter1->mock_endAddDevice();

    adapter1->mock_beginAddDevice(QDBusObjectPath("/adapter0/device4"));
    {
        DeviceMock *const device = DeviceMock::instance("/adapter0/device4");

        foreach (const QString &key, adapter0device4InitialProperties().keys()) {
            device->SetProperty(key, QDBusVariant(adapter0device4InitialProperties().value(key)));
        }

        device->audio()->mock_setProperty("State", QDBusVariant("connected"));

    }
    adapter1->mock_endAddDevice();

    mock_endAddAdapter();
    mock_setDefaultAdapter(QDBusObjectPath("/adapter0"));

    mock_beginAddAdapter(QDBusObjectPath("/adapter1"));
    AdapterMock *const adapter2 = AdapterMock::instance("/adapter1");

    adapter2->SetProperty("Powered", QDBusVariant(true));
    adapter2->SetProperty("Discoverable", QDBusVariant(false));
    adapter2->SetProperty("DiscoverableTimeout", QDBusVariant(0));

    adapter2->mock_beginAddDevice(QDBusObjectPath("/adapter1/device5"));
    {
        DeviceMock *const device = DeviceMock::instance("/adapter1/device5");

        foreach (const QString &key, adapter1device5InitialProperties().keys()) {
            device->SetProperty(key, QDBusVariant(adapter1device5InitialProperties().value(key)));
        }

    }
    adapter2->mock_endAddDevice();

    mock_endAddAdapter();
}

void BluezTestBase::ManagerMock::mock_beginAddAdapter(const QDBusObjectPath &path)
{
    Q_ASSERT(m_newAdapter == 0);

    m_newAdapter = new AdapterMock(path.path(), this);
}

void BluezTestBase::ManagerMock::mock_endAddAdapter()
{
    Q_ASSERT(m_newAdapter != 0);

    m_adapters.insert(QDBusObjectPath(m_newAdapter->path()), m_newAdapter);
    emit AdapterAdded(QDBusObjectPath(m_newAdapter->path()));
    emit PropertyChanged("Adapters", QDBusVariant(QVariant::fromValue(m_adapters.keys())));

    m_newAdapter = 0;
}

void BluezTestBase::ManagerMock::mock_removeAdapter(const QDBusObjectPath &path)
{
    Q_ASSERT(m_adapters.contains(path));

    AdapterMock *const adapter = m_adapters.take(path);
    if (m_defaultAdapter == path) {
        m_defaultAdapter = m_adapters.isEmpty() ? QDBusObjectPath() : m_adapters.keys().first();
        // specification says DefaultAdapterChanged will not be emitted if all adapters are removed
        if (!m_defaultAdapter.path().isEmpty()) {
            emit DefaultAdapterChanged(m_defaultAdapter);
        }
    }

    emit AdapterRemoved(path);
    emit PropertyChanged("Adapters", QDBusVariant(QVariant::fromValue(m_adapters.keys())));

    delete adapter;
}

void BluezTestBase::ManagerMock::mock_setDefaultAdapter(const QDBusObjectPath &path)
{
    Q_ASSERT(m_adapters.contains(path));

    m_defaultAdapter = path;

    emit DefaultAdapterChanged(m_defaultAdapter);
}

/*
 * \class Tests::BluezTestBase::AdapterMock
 */

QMap<QString, BluezTestBase::AdapterMock *> BluezTestBase::AdapterMock::s_instances;

BluezTestBase::AdapterMock::AdapterMock(const QString &path, QObject *parent)
    : QObject(parent),
      m_path(path)
{
    Q_ASSERT(!s_instances.contains(path));

    s_instances.insert(path, this);

    if (!bus().registerObject(m_path, this, QDBusConnection::ExportScriptableContents)) {
        qFatal("Failed to register AdapterMock to D-Bus at path '%s': '%s'",
                qPrintable(m_path), qPrintable(bus().lastError().message()));
    }
}

BluezTestBase::AdapterMock::~AdapterMock()
{
    s_instances.remove(m_path);
}

QVariantMap BluezTestBase::AdapterMock::GetProperties() const
{
    QVariantMap retv = m_properties;

    retv.insert("Devices", QVariant::fromValue(m_devices.keys()));

    return retv;
}

void BluezTestBase::AdapterMock::SetProperty(const QString &key, const QDBusVariant &value)
{
    if (m_properties.value(key) == value.variant()) {
        return;
    }

    m_properties[key] = value.variant();

    emit PropertyChanged(key, value);
}

void BluezTestBase::AdapterMock::StartDiscovery()
{
    if (m_properties.value("Discovering").toBool() == true) {
        return;
    }

    m_properties["Discovering"] = true;

    emit PropertyChanged("Discovering", QDBusVariant(true));
}

void BluezTestBase::AdapterMock::StopDiscovery()
{
    if (m_properties.value("Discovering").toBool() == false) {
        return;
    }

    m_properties["Discovering"] = false;

    emit PropertyChanged("Discovering", QDBusVariant(false));
}

QList<QDBusObjectPath> BluezTestBase::AdapterMock::ListDevices() const
{
    return m_devices.keys();
}

void BluezTestBase::AdapterMock::RegisterAgent(const QDBusObjectPath &agentPath,
        const QString &capability, const QDBusMessage &message)
{
    Q_UNUSED(capability);

    if (m_agent != 0) {
        qWarning("%s: Error: only one agent can be registered", Q_FUNC_INFO);
        return;
    }

    m_agent = new QDBusInterface(message.service(), agentPath.path(), AGENT_INTERFACE, bus());

    if (!m_agent->isValid()) {
        qWarning("%s: Error: %s", Q_FUNC_INFO, qPrintable(m_agent->lastError().message()));
        return;
    }

    emit mock_agentRegistered(agentPath);
}

void BluezTestBase::AdapterMock::CreatePairedDevice(const QString &deviceAddress,
        const QDBusObjectPath &agentPath, const QString &capability)
{
    emit mock_createPairedDeviceCalled(deviceAddress, agentPath, capability);
}

void BluezTestBase::AdapterMock::mock_setPropertyNoNotify(const QString &key,
        const QDBusVariant &value)
{
    m_properties[key] = value.variant();
}

void BluezTestBase::AdapterMock::mock_beginAddDevice(const QDBusObjectPath &path)
{
    Q_ASSERT(m_newDevice == 0);

    m_newDevice = new DeviceMock(path.path(), this);
}

void BluezTestBase::AdapterMock::mock_endAddDevice()
{
    Q_ASSERT(m_newDevice != 0);

    m_devices.insert(QDBusObjectPath(m_newDevice->path()), m_newDevice);
    emit DeviceFound(m_newDevice->GetProperties().value("Address").toString(),
            m_newDevice->GetProperties());
    emit DeviceCreated(QDBusObjectPath(m_newDevice->path()));
    emit PropertyChanged("Devices", QDBusVariant(QVariant::fromValue(m_devices.keys())));

    m_newDevice = 0;
}

void BluezTestBase::AdapterMock::mock_removeDevice(const QDBusObjectPath &path)
{
    Q_ASSERT(m_devices.contains(path));

    DeviceMock *const device = m_devices.take(path);

    emit DeviceRemoved(path);
    emit DeviceDisappeared(device->GetProperties().value("Address").toString());
    emit PropertyChanged("Devices", QDBusVariant(QVariant::fromValue(m_devices.keys())));

    delete device;
}

bool BluezTestBase::AdapterMock::mock_requestConfirmation(const QDBusObjectPath &devicePath,
        uint key, const QDBusMessage &message)
{
    QDBusPendingReply<> reply = m_agent->asyncCall("RequestConfirmation",
            QVariant::fromValue(devicePath), key);

    QDBusPendingCallWatcher watcher(reply);
    if (!waitForSignal(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)))) {
        bus().send(message.createErrorReply(QDBusError::Other,
                    "Timeout calling RequestConfirmation"));
        return false;
    }

    if (reply.isError() && reply.error().name() == "org.bluez.Error.Rejected") {
        return false;
    } else if (reply.isValid()) {
        return true;
    } else {
        bus().send(message.createErrorReply(QDBusError::Other,
                    "Error calling RequestConfirmation: " + reply.error().message()));
        return false;
    }
}

uint BluezTestBase::AdapterMock::mock_requestPasskey(const QDBusObjectPath &devicePath,
        const QDBusMessage &message)
{
    QDBusPendingReply<uint> reply = m_agent->asyncCall("RequestPasskey",
            QVariant::fromValue(devicePath));

    QDBusPendingCallWatcher watcher(reply);
    if (!waitForSignal(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)))) {
        bus().send(message.createErrorReply(QDBusError::Other,
                    "Timeout calling RequestPasskey"));
        return 0;
    }

    if (!reply.isValid()) {
        bus().send(message.createErrorReply(QDBusError::Other,
                    "Error calling RequestPasskey: " + reply.error().message()));
        return 0;
    }

    return reply.value();
}

QString BluezTestBase::AdapterMock::mock_requestPinCode(const QDBusObjectPath &devicePath,
        const QDBusMessage &message)
{
    QDBusPendingReply<QString> reply = m_agent->asyncCall("RequestPinCode",
            QVariant::fromValue(devicePath));

    QDBusPendingCallWatcher watcher(reply);
    if (!waitForSignal(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)))) {
        bus().send(message.createErrorReply(QDBusError::Other,
                    "Timeout calling RequestPinCode"));
        return QString();
    }

    if (!reply.isValid()) {
        bus().send(message.createErrorReply(QDBusError::Other,
                    "Error calling RequestPinCode: " + reply.error().message()));
        return QString();
    }

    return reply.value();
}

/*
 * \class Tests::BluezTestBase::DeviceMock
 */

QMap<QString, BluezTestBase::DeviceMock *> BluezTestBase::DeviceMock::s_instances;

BluezTestBase::DeviceMock::DeviceMock(const QString &path, QObject *parent)
    : QObject(parent),
      m_path(path),
      m_audioAdaptor(new AudioAdaptor(this)),
      m_inputAdaptor(new InputAdaptor(this))
{
    Q_ASSERT(!s_instances.contains(path));

    s_instances.insert(path, this);

    if (!bus().registerObject(m_path, this,
                QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors)) {
        qFatal("Failed to register DeviceMock to D-Bus at path '%s': '%s'",
                qPrintable(m_path), qPrintable(bus().lastError().message()));
    }
}

BluezTestBase::DeviceMock::~DeviceMock()
{
    s_instances.remove(m_path);
}

void BluezTestBase::DeviceMock::SetProperty(const QString &key, const QDBusVariant &value)
{
    if (m_properties.value(key) == value.variant()) {
        return;
    }

    m_properties[key] = value.variant();

    emit PropertyChanged(key, value);
}

void BluezTestBase::DeviceMock::mock_setPropertyNoNotify(const QString &key,
        const QDBusVariant &value)
{
    m_properties[key] = value.variant();
}

template<class TestClass, class MockClass>
int main(int argc, char *argv[])
{
    qputenv("DBUS_SYSTEM_BUS_ADDRESS",
            qgetenv("DBUS_SESSION_BUS_ADDRESS"));

    if (argc == 2 && argv[1] == QLatin1String("--mock")) {
        qInstallMessageHandler(Tests::TestBase::mockMessageHandler);
        QGuiApplication app(argc, argv);

        qRegisterMetaType<QDBusPendingCallWatcher *>();

        qDebug("%s: starting...", Q_FUNC_INFO);

        MockClass mock;

        if (!MockClass::TestBase::bus().registerService(MockClass::TestBase::SERVICE)) {
            qFatal("Failed to register mock D-Bus service '%s': '%s'",
                    qPrintable(MockClass::TestBase::SERVICE),
                    qPrintable(MockClass::TestBase::bus().lastError().message()));
        }

        return app.exec();
    } else {
        QGuiApplication app(argc, argv);

        qRegisterMetaType<QDBusPendingCallWatcher *>();

        TestClass test;

        QProcess mock;
        mock.setProcessChannelMode(QProcess::ForwardedChannels);
        mock.start(app.applicationFilePath(), QStringList("--mock"));
        if (mock.state() == QProcess::NotRunning) {
            qFatal("Failed to start mock");
        }

        const int retv = QTest::qExec(&test, argc, argv);

        mock.terminate();
        mock.waitForFinished();

        return retv;
    }
}

} // namespace Tests

#endif // TESTBASE_H
