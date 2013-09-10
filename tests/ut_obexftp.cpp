#include "testbase.h"
#include "moc_testbase.cpp"

#include "obex/obexftp.h"

namespace Tests {

class ObexFtpTestBase : public TestBase
{
    Q_OBJECT

public:
    class ClientMock;
    class SessionMock;
    class FileTransferMock;

    static const QString SERVICE;

    static const QString CLIENT_PATH;

    static const QString CLIENT_INTERFACE;
    static const QString SESSION_INTERFACE;
    static const QString FILE_TRANSFER_INTERFACE;
    static const QString AGENT_INTERFACE;

public:
    ObexFtpTestBase() {}

public:
    static QDBusConnection bus() { return QDBusConnection::sessionBus(); }
};

class UtObexFtp : public ObexFtpTestBase
{
    Q_OBJECT

public:
    UtObexFtp();

private slots:
    void initTestCase();
    void test();

private:
};

class ObexFtpTestBase::ClientMock : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openobex.Client")

public:
    typedef ObexFtpTestBase TestBase; // required by template<> main()

public:
    ClientMock();

    Q_SCRIPTABLE QDBusObjectPath CreateSession(const QVariantMap &device);

signals:
    // mock API
    Q_SCRIPTABLE void mock_CreateSessionCalled(const QVariantMap &device,
            const QDBusObjectPath &sessionPath);

private:
    QMap<QString, SessionMock *> m_sessions;
};

class ObexFtpTestBase::SessionMock : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openobex.Session")

    class FileTransferAdaptor;
    class TransferAdaptor;

public:
    SessionMock(const QString &path, QObject *parent);

    Q_SCRIPTABLE QVariantMap GetProperties() const { return m_properties; }
    Q_SCRIPTABLE void AssignAgent(const QDBusObjectPath &agentPath, const QDBusMessage &message);
    Q_SCRIPTABLE void Close() { emit mock_CloseCalled(); }

    // mock API
    Q_SCRIPTABLE bool mock_AssignAgentCalled() const;

signals:
    // mock API
    Q_SCRIPTABLE void mock_CloseCalled();

private:
    const QString m_path;
    QString m_agentService;
    QVariantMap m_properties;
};

class ObexFtpTestBase::SessionMock::FileTransferAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openobex.FileTransfer")

public:
    FileTransferAdaptor(SessionMock *session) : QDBusAbstractAdaptor(session) { }

public slots:
    Q_SCRIPTABLE void PutFile(const QString &localFile, const QString &remoteFile)
    {
        emit mock_PutFileCalled(localFile, remoteFile);

        QDBusInterface agent(session()->m_agentService,
                session()->m_properties.value("AgentPath").toString(), AGENT_INTERFACE, bus());

        QDBusPendingReply<void> progressReply = agent.asyncCall("Progress",
                QVariant::fromValue(QDBusObjectPath(session()->m_path)), (qulonglong)21);
        if (progressReply.isError()) {
            qWarning("%s: error calling agent.Progress(): %s", Q_FUNC_INFO,
                    qPrintable(progressReply.error().message()));
            return;
        }

        QDBusPendingReply<void> completeReply = agent.asyncCall("Complete",
                QVariant::fromValue(QDBusObjectPath(session()->m_path)));
        if (completeReply.isError()) {
            qWarning("%s: error calling agent.Complete(): %s", Q_FUNC_INFO,
                    qPrintable(completeReply.error().message()));
            return;
        }
    }

signals:
    // mock API
    void mock_PutFileCalled(const QString &localFile, const QString &remoteFile);

private:
    SessionMock *session() { return qobject_cast<SessionMock *>(parent()); }
};

class ObexFtpTestBase::SessionMock::TransferAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openobex.Transfer")

public:
    TransferAdaptor(SessionMock *session) : QDBusAbstractAdaptor(session) { }

public slots:
    Q_SCRIPTABLE QVariantMap GetProperties() const
    {
        QVariantMap properties;
        properties["Size"] = (qulonglong)42;
        return properties;
    }
};

} // namespace Tests

using namespace Tests;

/*
 * \class Tests::ObexFtpTestBase
 */

const QString ObexFtpTestBase::SERVICE = "org.openobex.client";

const QString ObexFtpTestBase::CLIENT_PATH = "/";

const QString ObexFtpTestBase::CLIENT_INTERFACE = "org.openobex.Client";
const QString ObexFtpTestBase::SESSION_INTERFACE = "org.openobex.Session";
const QString ObexFtpTestBase::FILE_TRANSFER_INTERFACE = "org.openobex.FileTransfer";
const QString ObexFtpTestBase::AGENT_INTERFACE = "org.openobex.Agent";

/*
 * \class Tests::UtObexFtp
 */

UtObexFtp::UtObexFtp()
{
}

void UtObexFtp::initTestCase()
{
    QVERIFY(waitForService(SERVICE, bus()));
}

void UtObexFtp::test()
{
    const QString device = "de:ad:be:ef:de:ad";

    QDBusInterface client(SERVICE, CLIENT_PATH, CLIENT_INTERFACE, bus());

    ObexFtp obexFtp(device);

    QVERIFY(!obexFtp.isConnected());

    SignalSpy createSessionCalledSpy(&client,
            SIGNAL(mock_CreateSessionCalled(QVariantMap,QDBusObjectPath)));
    SignalSpy connectedSpy(&obexFtp, SIGNAL(connected()));

    obexFtp.connect();

    QVERIFY(waitForSignal(&createSessionCalledSpy));
    QCOMPARE(createSessionCalledSpy.count(), 1);
    QCOMPARE(createSessionCalledSpy.at(0).at(0).toMap().value("Destination"), QVariant(device));
    QCOMPARE(createSessionCalledSpy.at(0).at(0).toMap().value("Target"), QVariant("FTP"));

    QVERIFY(waitForSignal(&connectedSpy));
    QCOMPARE(connectedSpy.count(), 1);

    QVERIFY(obexFtp.isConnected());

    const QString sessionPath = createSessionCalledSpy.at(0).at(1).value<QDBusObjectPath>().path();

    QDBusInterface session(SERVICE, sessionPath, SESSION_INTERFACE, bus());

    QDBusReply<bool> assignAgentCalled = session.call("mock_AssignAgentCalled");
    QVERIFY(assignAgentCalled.isValid());
    QVERIFY(assignAgentCalled.value());

    QDBusInterface fileTransfer(SERVICE, sessionPath, FILE_TRANSFER_INTERFACE, bus());

    SignalSpy putFileCalledSpy(&fileTransfer, SIGNAL(mock_PutFileCalled(QString,QString)));
    SignalSpy transferProgressSpy(&obexFtp, SIGNAL(transferProgress(qulonglong,qulonglong)));
    SignalSpy transferCompleteSpy(&obexFtp, SIGNAL(transferComplete()));

    obexFtp.transferFile("C:/WINDOWS", "/dev/null");

    QVERIFY(waitForSignal(&putFileCalledSpy));
    QCOMPARE(putFileCalledSpy.count(), 1);
    QCOMPARE(putFileCalledSpy.at(0).at(0), QVariant("C:/WINDOWS"));
    QCOMPARE(putFileCalledSpy.at(0).at(1), QVariant("/dev/null"));

    QVERIFY(waitForSignal(&transferProgressSpy));
    QCOMPARE(transferProgressSpy.count(), 1);
    QCOMPARE(transferProgressSpy.at(0).at(0), QVariant((qulonglong)21));
    QCOMPARE(transferProgressSpy.at(0).at(1), QVariant((qulonglong)42));

    QVERIFY(waitForSignal(&transferCompleteSpy));
    QCOMPARE(transferCompleteSpy.count(), 1);

    SignalSpy closeCalledSpy(&session, SIGNAL(mock_CloseCalled()));

    obexFtp.disconnect();

    QVERIFY(waitForSignal(&closeCalledSpy));
    QCOMPARE(closeCalledSpy.count(), 1);
}

/*
 * \class Tests::ObexFtpTestBase::ClientMock
 */

ObexFtpTestBase::ClientMock::ClientMock()
{
    if (!bus().registerObject(CLIENT_PATH, this, QDBusConnection::ExportScriptableContents)) {
        qFatal("Failed to register ClientMock to D-Bus at path '%s': '%s'",
                qPrintable(CLIENT_PATH), qPrintable(bus().lastError().message()));
    }
}

QDBusObjectPath ObexFtpTestBase::ClientMock::CreateSession(const QVariantMap &device)
{
    static int sessionNum = -1;
    ++sessionNum;

    const QString sessionPath = QString("/session%1").arg(sessionNum);

    m_sessions[sessionPath] = new SessionMock(sessionPath, this);

    emit mock_CreateSessionCalled(device, QDBusObjectPath(sessionPath));

    return QDBusObjectPath(sessionPath);
}

/*
 * \class Tests::ObexFtpTestBase::SessionMock
 */

ObexFtpTestBase::SessionMock::SessionMock(const QString &path, QObject *parent)
    : QObject(parent),
      m_path(path)
{
    new FileTransferAdaptor(this);
    new TransferAdaptor(this);

    if (!bus().registerObject(m_path, this,
                QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors)) {
        qFatal("Failed to register SessionMock to D-Bus at path '%s': '%s'",
                qPrintable(m_path), qPrintable(bus().lastError().message()));
    }
}

void ObexFtpTestBase::SessionMock::AssignAgent(const QDBusObjectPath &agentPath,
        const QDBusMessage &message)
{
    m_properties["AgentPath"] = agentPath.path();
    m_agentService = message.service();
}

bool ObexFtpTestBase::SessionMock::mock_AssignAgentCalled() const
{
    return !m_properties.value("AgentPath").toString().isEmpty();
}

TEST_MAIN(UtObexFtp, ObexFtpTestBase::ClientMock)

#include "ut_obexftp.moc"
