/*
 * Copyright (C) 2015 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -c DBusAccounts -l dbusaccounts -p dbusaccounts com.deepin.daemon.Accounts
 *
 * qdbusxml2cpp is Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef DBUSACCOUNTS_H_1447229744
#define DBUSACCOUNTS_H_1447229744

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.daemon.Accounts
 */
class DBusAccounts: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName !="com.deepin.daemon.Accounts")
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        QStringList keys = changedProps.keys();
        foreach(const QString &prop, keys) {
        const QMetaObject* self = metaObject();
            for (int i=self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
 	            Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
   }
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.daemon.Accounts"; }

public:
    DBusAccounts(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~DBusAccounts();

    Q_PROPERTY(bool AllowGuest READ allowGuest NOTIFY AllowGuestChanged)
    inline bool allowGuest() const
    { return qvariant_cast< bool >(property("AllowGuest")); }

    Q_PROPERTY(QString GuestIcon READ guestIcon NOTIFY GuestIconChanged)
    inline QString guestIcon() const
    { return qvariant_cast< QString >(property("GuestIcon")); }

    Q_PROPERTY(QStringList UserList READ userList NOTIFY UserListChanged)
    inline QStringList userList() const
    { return qvariant_cast< QStringList >(property("UserList")); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> AllowGuestAccount(bool in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("AllowGuestAccount"), argumentList);
    }

    inline QDBusPendingReply<QString> CreateGuestAccount()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("CreateGuestAccount"), argumentList);
    }

    inline QDBusPendingReply<> CreateUser(const QString &in0, const QString &in1, int in2)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2);
        return asyncCallWithArgumentList(QStringLiteral("CreateUser"), argumentList);
    }

    inline QDBusPendingReply<bool> DeleteUser(const QString &in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("DeleteUser"), argumentList);
    }

    inline QDBusPendingReply<QString> FindUserById(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("FindUserById"), argumentList);
    }

    inline QDBusPendingReply<QString> FindUserByName(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("FindUserByName"), argumentList);
    }

    inline QDBusPendingReply<bool> IsPasswordValid(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsPasswordValid"), argumentList);
    }

    inline QDBusPendingReply<bool, QString, int> IsUsernameValid(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsUsernameValid"), argumentList);
    }
    inline QDBusReply<bool> IsUsernameValid(const QString &in0, QString &out1, int &out2)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("IsUsernameValid"), argumentList);
        if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().count() == 3) {
            out1 = qdbus_cast<QString>(reply.arguments().at(1));
            out2 = qdbus_cast<int>(reply.arguments().at(2));
        }
        return reply;
    }

    inline QDBusPendingReply<QString, bool> RandUserIcon()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("RandUserIcon"), argumentList);
    }
    inline QDBusReply<QString> RandUserIcon(bool &out1)
    {
        QList<QVariant> argumentList;
        QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("RandUserIcon"), argumentList);
        if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().count() == 2) {
            out1 = qdbus_cast<bool>(reply.arguments().at(1));
        }
        return reply;
    }

Q_SIGNALS: // SIGNALS
    void Error(uint in0, const QString &in1, const QString &in2);
    void UserAdded(const QString &in0);
    void UserDeleted(const QString &in0);
// begin property changed signals
void AllowGuestChanged();
void GuestIconChanged();
void UserListChanged();
};

namespace com {
  namespace deepin {
    namespace daemon {
      typedef ::DBusAccounts Accounts;
    }
  }
}
#endif
