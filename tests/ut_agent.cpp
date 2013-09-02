#include "testbase.h"
#include "moc_testbase.cpp"

#include "asyncagent.h"

namespace Tests {

class UtAgent : public BluezTestBase
{
    Q_OBJECT

private:
    static const QString AGENT_PATH;

public:
    class AgentRelay;

public:
    UtAgent();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testRequestConfirmation_data();
    void testRequestConfirmation();
    void testRequestPasskey();
    void testRequestPinCode();

private:
    QPointer<AgentRelay> m_agentRelay;
    QPointer<AsyncAgent> m_agent;
};

class UtAgent::AgentRelay : public QObject
{
    Q_OBJECT

public:
    AgentRelay(QObject *parent) : QObject(parent) {}

signals:
    void requestConfirmation(const QString &deviceAlias, uint key);
    void requestPasskey(const QString &deviceAlias);
    void requestPidCode(const QString &deviceAlias);
};

} // namespace Tests

using namespace Tests;

/*
 * \class Tests::UtAgent
 */

const QString UtAgent::AGENT_PATH = "/agent";

UtAgent::UtAgent()
{
}

void UtAgent::initTestCase()
{
    QVERIFY(waitForService(SERVICE, bus()));

    m_agentRelay = new AgentRelay(0);
    m_agent = new AsyncAgent(AGENT_PATH, m_agentRelay);
    // TODO: anything (on dbus) to wait for here?

    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    SignalSpy agentRegisteredSpy(&adapter, SIGNAL(mock_agentRegistered(QDBusObjectPath)));

    m_agent->registerAgent();

    QVERIFY(waitForSignal(&agentRegisteredSpy));
    QCOMPARE(agentRegisteredSpy.count(), 1);
    QCOMPARE(agentRegisteredSpy.at(0).at(0).value<QDBusObjectPath>().path(), AGENT_PATH);
}

void UtAgent::cleanupTestCase()
{
    delete m_agent;
    delete m_agentRelay;
}

void UtAgent::testRequestConfirmation_data()
{
    QTest::addColumn<bool>("accept");

    QTest::newRow("accept") << true;
    QTest::newRow("reject") << false;
}

void UtAgent::testRequestConfirmation()
{
    QFETCH(bool, accept);

    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    SignalSpy requestConfirmationSpy(m_agentRelay, SIGNAL(requestConfirmation(QString,uint)));

    QDBusPendingReply<bool> reply = adapter.asyncCall("mock_requestConfirmation",
            QVariant::fromValue(QDBusObjectPath("/adapter0/device2")), 42u);
    Q_ASSERT(!reply.isError());

    QVERIFY(waitForSignal(&requestConfirmationSpy));
    QCOMPARE(requestConfirmationSpy.count(), 1);
    QCOMPARE(requestConfirmationSpy.at(0).at(0), adapter0device2InitialProperties().value("Alias"));
    QCOMPARE(requestConfirmationSpy.at(0).at(1), QVariant(42u));

    m_agent->replyRequestConfirmation(accept);

    reply.waitForFinished();
    QVERIFY(reply.isValid());
    QCOMPARE(reply.value(), accept);
}

void UtAgent::testRequestPasskey()
{
    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    SignalSpy requestPasskeySpy(m_agentRelay, SIGNAL(requestPasskey(QString)));

    QDBusPendingReply<uint> reply = adapter.asyncCall("mock_requestPasskey",
            QVariant::fromValue(QDBusObjectPath("/adapter0/device2")));
    Q_ASSERT(!reply.isError());

    QVERIFY(waitForSignal(&requestPasskeySpy));
    QCOMPARE(requestPasskeySpy.count(), 1);
    QCOMPARE(requestPasskeySpy.at(0).at(0), adapter0device2InitialProperties().value("Alias"));

    m_agent->replyPasskey(42);

    reply.waitForFinished();
    QVERIFY(reply.isValid());
    QCOMPARE(reply.value(), 42u);
}

void UtAgent::testRequestPinCode()
{
    QDBusInterface adapter(SERVICE, "/adapter0", ADAPTER_INTERFACE, bus());

    SignalSpy requestPinCodeSpy(m_agentRelay, SIGNAL(requestPidCode(QString)));

    QDBusPendingReply<QString> reply = adapter.asyncCall("mock_requestPinCode",
            QVariant::fromValue(QDBusObjectPath("/adapter0/device2")));
    Q_ASSERT(!reply.isError());

    QVERIFY(waitForSignal(&requestPinCodeSpy));
    QCOMPARE(requestPinCodeSpy.count(), 1);
    QCOMPARE(requestPinCodeSpy.at(0).at(0), adapter0device2InitialProperties().value("Alias"));

    m_agent->replyRequestPidCode("secret");

    reply.waitForFinished();
    QVERIFY(reply.isValid());
    QCOMPARE(reply.value(), QString("secret"));
}

TEST_MAIN(UtAgent, BluezTestBase::ManagerMock)

#include "ut_agent.moc"
