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

#include "userwidget.h"
#include "constants.h"
#include "dbus/dbuslockservice.h"

#include <QApplication>
#include <QtWidgets>
#include <QtGui>
#include <QtCore>
#include <QSettings>
#include <QJsonObject>
#include <QJsonValue>
#include <unistd.h>
#include <pwd.h>

#define LOCKSERVICE_PATH "/com/deepin/dde/LockService"
#define LOCKSERVICE_NAME "com.deepin.dde.LockService"

DWIDGET_USE_NAMESPACE

UserWidget::UserWidget(QWidget* parent)
    : QFrame(parent),
    m_lockInter(LOCKSERVICE_NAME, LOCKSERVICE_PATH, QDBusConnection::systemBus(), this),
    m_dbusLogined(new Logined("com.deepin.daemon.Accounts", "/com/deepin/daemon/Logined", QDBusConnection::systemBus(), this))
{
    m_currentUser = m_lockInter.CurrentUser();
    qDebug() << Q_FUNC_INFO << m_currentUser;

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedWidth(qApp->desktop()->width());

    m_dbusAccounts = new DBusAccounts(ACCOUNT_DBUS_SERVICE,  ACCOUNT_DBUS_PATH, QDBusConnection::systemBus(), this);

    onUserListChanged();

    m_dbusLogined->setSync(false);
    m_dbusLogined->userList();

    initUI();
    initConnections();
}

UserWidget::~UserWidget()
{
    qDeleteAll(m_userBtns);
}

void UserWidget::initUI()
{
    for (DBusUser *inter : m_userDbus.values()) {
        if (!inter->locked() && !m_whiteList.contains(inter->userName()))
            m_whiteList.append(inter->userName());
    }

    setCurrentUser(currentUser());

    const int count = m_userBtns.count();
    const int maxLineCap = width() / USER_ICON_WIDTH - 1; // 1 for left-offset and right-offset.

    // Adjust size according to user count.
    if (maxLineCap < count) {
        setFixedSize(width(), USER_ICON_HEIGHT * qCeil(count * 1.0 / maxLineCap));
    } else {
        setFixedHeight(USER_ICON_HEIGHT);
    }
}

void UserWidget::initConnections()
{
    connect(m_dbusAccounts, &DBusAccounts::UserListChanged, this, &UserWidget::onUserListChanged);
    connect(m_dbusAccounts, &DBusAccounts::UserAdded, this, &UserWidget::onUserAdded);
    connect(m_dbusAccounts, &DBusAccounts::UserDeleted, this, &UserWidget::onUserRemoved);

    connect(m_dbusLogined, &Logined::UserListChanged, this, &UserWidget::onLoginUserListChanged);
}

void UserWidget::onUserListChanged()
{
    for (const QString &name : m_dbusAccounts->userList())
        onUserAdded(name);
}

void UserWidget::onUserAdded(const QString &path)
{
    if (m_userDbus.contains(path))
        return;

    DBusUser *user = new DBusUser(ACCOUNT_DBUS_SERVICE, path, QDBusConnection::systemBus(), this);

    m_userDbus.insert(path, user);

    UserButton *userBtn = new UserButton(user);
    userBtn->hide();
    userBtn->setParent(this);
    userBtn->move(rect().center() - userBtn->rect().center());

    connect(userBtn, &UserButton::imageClicked, this, &UserWidget::setCurrentUser);

    m_userBtns.append(userBtn);
}

void UserWidget::onUserRemoved(const QString &name)
{
    DBusUser *user;
    user = m_userDbus.find(name).value();

    if (user) {
        m_userDbus.remove(name);
        user->deleteLater();

        UserButton *button;
        for (int index(0); index != m_userBtns.count(); ++index) {
            button = m_userBtns.at(index);

            if (button) {
                m_userBtns.removeOne(button);
                button->deleteLater();
                return;
            }
        }
    }
}

void UserWidget::onLoginUserListChanged(const QString &value)
{
    QJsonDocument doc = QJsonDocument::fromJson(value.toUtf8());

    QJsonArray jsonArray = doc.array();

    if (jsonArray.isEmpty())
        return;

    m_loggedInUsers.clear();

    for (int i(0); i != jsonArray.count(); ++i) {
        const QJsonObject &obj = jsonArray.at(i).toObject();
        if (obj["Display"].toString().isEmpty())
            continue;
        m_loggedInUsers << obj["Name"].toString();
    }
}

UserButton *UserWidget::getUserByName(const QString &username)
{
    for (UserButton *button : m_userBtns)
        if (button->name() == username)
            return button;

    Q_UNREACHABLE();

    return nullptr;
}

void UserWidget::setCurrentUser(const QString &username)
{
    qDebug() << username << sender();

    m_currentUser = username;

    isChooseUserMode = false;

    for (UserButton *user : m_userBtns) {
        if (user->name() == username) {
            user->showButton();
            user->setImageSize(user->AvatarLargerSize);
            user->setButtonChecked(false);
            user->setSelected(false);
            user->show();
        } else
            user->hide(180);

        user->move(rect().center() - user->rect().center(), 200);
    }

    emit userChanged(m_currentUser);
    emit chooseUserModeChanged(isChooseUserMode, m_currentUser);
}

void UserWidget::removeUser(QString name)
{
    qDebug() << "remove user: " << name;

    for (int i(0); i < m_userBtns.count(); ++i) {
        if (m_userBtns[i]->name() == name) {
            UserButton *btn = m_userBtns[i];
            m_userBtns.removeAt(i);
            btn->deleteLater();
            break;
        }
    }
}

void UserWidget::expandWidget()
{
    isChooseUserMode = true;

    const int count = m_userBtns.count();
    const int maxLineCap = width() / USER_ICON_WIDTH - 1; // 1 for left-offset and right-offset.
    const int offset = (width() - USER_ICON_WIDTH * qMin(count, maxLineCap)) / 2;

    // Adjust size according to user count.
    if (maxLineCap < count) {
        setFixedSize(width(), USER_ICON_HEIGHT * qCeil(count * 1.0 / maxLineCap));
    }

    for (int i = 0; i != count; ++i)
    {
        UserButton *user = m_userBtns.at(i);
        const QString username = user->name();

        user->setSelected(username == m_currentUser);

        if (m_loggedInUsers.contains(username)) {
            user->setButtonChecked(true);
        } else {
            user->setButtonChecked(false);
        }

        user->stopAnimation();
        user->show();
        user->showButton();
        user->setImageSize(UserButton::AvatarSmallSize);
        if (i + 1 <= maxLineCap) {
            // the first line.
            user->move(QPoint(offset + i * USER_ICON_WIDTH, 0), 200);
        } else {
            // the second line.
            user->move(QPoint(offset + (i - maxLineCap) * USER_ICON_WIDTH, USER_ICON_HEIGHT), 200);
        }
    }

    emit chooseUserModeChanged(isChooseUserMode, m_currentUser);
}

void UserWidget::saveLastUser()
{
    m_lockInter.SwitchToUser(currentUser()).waitForFinished();
}

void UserWidget::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    if (isChooseUserMode) {
        // rearrange the user icons.
        expandWidget();
    } else {
        for (UserButton *user : m_userBtns)
            user->move(rect().center() - user->rect().center(), 1);
    }
}

void UserWidget::chooseButtonChecked() {
    qDebug() << "Grab Key Release Event";

    this->grabKeyboard();
    qDebug() << "Get the Key Return or Enter";
    bool checkedBtsExist = false;
    for (UserButton* user: m_userBtns) {
        if (user->selected()) {
            setCurrentUser(user->name());
            checkedBtsExist = true;
        }
    }
    if (!checkedBtsExist) {
        setCurrentUser(m_currentUser);
    }
}

void UserWidget::leftKeySwitchUser() {
    if (isChooseUserMode) {
        if (!m_currentUserIndex)
            m_currentUserIndex = m_userBtns.length() - 1;
        else
            m_currentUserIndex -= 1;

        m_currentUser = m_userBtns.at(m_currentUserIndex)->name();

        for (int i(0); i < m_userBtns.count(); i++)
            m_userBtns.at(i)->setSelected(i == m_currentUserIndex);
    }
}

void UserWidget::rightKeySwitchUser() {
    if (isChooseUserMode) {
        if (m_currentUserIndex ==  m_userBtns.length() - 1)
            m_currentUserIndex = 0;
        else
            m_currentUserIndex = m_currentUserIndex + 1;

        m_currentUser = m_userBtns.at(m_currentUserIndex)->name();

        for (int i(0); i < m_userBtns.count(); i++)
            m_userBtns.at(i)->setSelected(i == m_currentUserIndex);
    }
}

const QString UserWidget::loginUser()
{
    struct passwd *pws;
    pws = getpwuid(getuid());
    return QString(pws->pw_name);
}

const QString UserWidget::currentUser()
{
    qDebug() << Q_FUNC_INFO << m_currentUser;

    if (!m_currentUser.isEmpty() && m_whiteList.contains(m_currentUser)) {
        return m_currentUser;
    }

    struct passwd *pws;
    pws = getpwuid(getuid());
    const QString currentLogin(pws->pw_name);

    //except the current-user named lightdm
    if (!currentLogin.isEmpty() && currentLogin!="lightdm")
        return currentLogin;

    if (!m_whiteList.isEmpty())
        return m_whiteList.first();

    // return first user
    if (m_userDbus.count() > 0) {
        const QString tmpUsername = m_userDbus.first()->userName();
        return tmpUsername;
    }

//    whiteList = QStringList(whiteList.toSet().toList());
//    if (whiteList.length()!=0) {
//        updateAvatar(whiteList[0]);
//        return whiteList[0];
//    }

    qWarning() << "no users !!!";
    return QString();
}

const QString UserWidget::getUserAvatar(const QString &username)
{
    UserButton *user = getUserByName(username);
    if (user)
        return user->avatar();
    return QString();
}

const QStringList UserWidget::getLoggedInUsers() const
{
    return m_loggedInUsers;
}

bool UserWidget::getUserIsAutoLogin(const QString &username)
{
    UserButton *user = getUserByName(username);
    return user ? user->automaticLogin() : false;
}

const QString UserWidget::getUserGreeterBackground(const QString &username)
{
    UserButton *user = getUserByName(username);
    return user ? user->greeter() : QString();
}

const QStringList UserWidget::getUserKBHistory(const QString &username)
{
    UserButton *user = getUserByName(username);
    return user ? user->kbHistory() : QStringList();
}

const QString UserWidget::getUserKBLayout(const QString &username)
{
    UserButton *user = getUserByName(username);
    return user ? user->kblayout() : QString();
}

void UserWidget::setUserKBlayout(const QString &username, const QString &layout)
{
    UserButton *user = getUserByName(username);
    if (user)
        user->setKbLayout(layout);
}
