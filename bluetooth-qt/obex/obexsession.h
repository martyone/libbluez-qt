/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -p obexsession.h:obexsession.cpp -i types.h session.xml
 *
 * qdbusxml2cpp is Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef OBEXSESSION_H_1306341758
#define OBEXSESSION_H_1306341758

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include "types.h"

/*
 * Proxy class for interface org.openobex.FileTransfer
 */
class OrgOpenobexFileTransferInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.openobex.FileTransfer"; }

public:
    OrgOpenobexFileTransferInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgOpenobexFileTransferInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> ChangeFolder(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("ChangeFolder"), argumentList);
    }

    inline QDBusPendingReply<> CopyFile(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0) << qVariantFromValue(in1);
        return asyncCallWithArgumentList(QLatin1String("CopyFile"), argumentList);
    }

    inline QDBusPendingReply<> CreateFolder(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("CreateFolder"), argumentList);
    }

    inline QDBusPendingReply<> Delete(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("Delete"), argumentList);
    }

    inline QDBusPendingReply<> GetFile(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0) << qVariantFromValue(in1);
        return asyncCallWithArgumentList(QLatin1String("GetFile"), argumentList);
    }

    inline QDBusPendingReply<QVariantMapList> ListFolder()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("ListFolder"), argumentList);
    }

    inline QDBusPendingReply<> MoveFile(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0) << qVariantFromValue(in1);
        return asyncCallWithArgumentList(QLatin1String("MoveFile"), argumentList);
    }

    inline QDBusPendingReply<> PutFile(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0) << qVariantFromValue(in1);
        return asyncCallWithArgumentList(QLatin1String("PutFile"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

/*
 * Proxy class for interface org.openobex.Session
 */
class OrgOpenobexSessionInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.openobex.Session"; }

public:
    OrgOpenobexSessionInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgOpenobexSessionInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> AssignAgent(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("AssignAgent"), argumentList);
    }

    inline QDBusPendingReply<> Close()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Close"), argumentList);
    }

    inline QDBusPendingReply<QVariantMap> GetProperties()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("GetProperties"), argumentList);
    }

    inline QDBusPendingReply<> ReleaseAgent(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("ReleaseAgent"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace org {
  namespace openobex {
    typedef ::OrgOpenobexFileTransferInterface FileTransfer;
    typedef ::OrgOpenobexSessionInterface Session;
  }
}
#endif