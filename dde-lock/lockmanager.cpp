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
#include "passwdedit.h"
#include "util_updateui.h"
#include "dbus/dbusinputdevices.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QDebug>
#include <QMap>
#include <QProcess>
#include <QDBusConnection>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include "lockmanager.h"
#include "lockframe.h"
#include "dbus/dbuslockfront.h"

static const QSize ZoreSize = QSize(0, 0);

LockManager::LockManager(QWidget *parent)
    : QFrame(parent),

      m_activatedUser(UserWidget::loginUser())
{
    initUI();
    initConnect();
    initBackend();
    updateUI();

    leaveEvent(nullptr);
}

void LockManager::initConnect()
{

    connect(m_passwordEdit, &PassWdEdit::keybdLayoutButtonClicked, this, &LockManager::keybdLayoutWidgetPosit);
    connect(m_controlWidget, &ControlWidget::requestShutdown, this, &LockManager::shutdownMode);
    connect(m_controlWidget, &ControlWidget::requestSwitchUser, this, &LockManager::chooseUserMode);
    connect(m_controlWidget, &ControlWidget::requestSwitchUser, m_userWidget, &UserWidget::expandWidget, Qt::QueuedConnection);

    connect(m_requireShutdownWidget, &ShutdownWidget::shutDownWidgetAction, [this](const ShutdownWidget::Actions action) {
        switch (action) {
        case ShutdownWidget::RequireRestart:    m_action = Restart;         break;
        case ShutdownWidget::RequireShutdown:   m_action = Shutdown;        break;
        case ShutdownWidget::RequireSuspend:    m_action = Suspend;         break;
        }

        passwordMode();
    });
    connect(qApp, &QApplication::aboutToQuit, [this] {
        m_hotZoneInterface->EnableZoneDetected(true);
    });

    connect(m_passwordEdit, &PassWdEdit::leftKeyPressed, this, &LockManager::leftKeyPressed);
    connect(m_passwordEdit, &PassWdEdit::rightKeyPressed, this, &LockManager::rightKeyPressed);

    connect(m_unlockButton, &QPushButton::clicked, this, &LockManager::unlock);
}

void LockManager::keybdLayoutWidgetPosit()
{
    const QPoint p(m_keybdArrowWidget->rect().x() + 45, m_keybdArrowWidget->rect().y() + 21);

    m_keybdArrowWidget->setCornerPoint(p);

    const int x = m_passwordEdit->x() + m_keybdArrowWidget->width() / 2 - 22;

    m_keybdArrowWidget->show(x, m_passwordEdit->y() + m_passwordEdit->height() - 15);

    m_keybdLayoutWidget->show();
}

void LockManager::leftKeyPressed()
{
    if (!m_userWidget->isHidden() && m_passwordEdit->isHidden()) {
        m_userWidget->leftKeySwitchUser();
    }
//    if (!m_userWidget->isHidden() && !m_passwordEdit->isHidden() &&
//            m_passwordEdit->getText().isEmpty()) {
//        m_userWidget->leftKeySwitchUser();
//    }
    if (!m_requireShutdownWidget->isHidden()) {
        m_requireShutdownWidget->leftKeySwitch();
    }
}

void LockManager::rightKeyPressed()
{
    if (!m_userWidget->isHidden() && m_passwordEdit->isHidden()) {
        m_userWidget->rightKeySwitchUser();
    }
//    if (!m_userWidget->isHidden() && !m_passwordEdit->isHidden() &&
//            m_passwordEdit->getText().isEmpty()) {
//        m_userWidget->rightKeySwitchUser();
//    }
    if (!m_requireShutdownWidget->isHidden()) {
        m_requireShutdownWidget->rightKeySwitch();
    }
}

//void LockManager::onUserUnlock(const QString &username)
//{
//    qDebug()<< "recive message---------------------------";
//    if (username == m_userWidget->currentUser()) {
//        qApp->exit();
//    }
//}

void LockManager::initUI()
{
    setFocusPolicy(Qt::NoFocus);

    m_timeWidget = new TimeWidget(this);
    m_timeWidget->setFixedSize(400, 300);

    m_userWidget = new UserWidget(this);
    m_userWidget->setFixedWidth(width());
    m_userWidget->move(0, (height() - m_userWidget->height()) / 2 - 95);

    m_passwordEdit = new PassWdEdit(this);
    m_passwordEdit->setEnterBtnStyle(":/img/action_icons/unlock_normal.svg",
                                     ":/img/action_icons/unlock_hover.svg",
                                     ":/img/action_icons/unlock_press.svg");
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);
    m_passwordEdit->setFocus();
    m_passwdEditSize = m_passwordEdit->size();

    m_unlockButton = new QPushButton(this);
    m_unlockButton->setText(tr("Login"));
    m_unlockButton->setFixedSize(160, 36);
    m_unlockButton->hide();

    m_requireShutdownWidget = new ShutdownWidget(this);
    m_requireShutdownWidget->hide();
    m_requireShutdownWidget->setFixedWidth(width());
    m_requireShutdownWidget->setFixedHeight(300);

    m_controlWidget = new ControlWidget(this);
    m_controlWidget->setUserSwitchEnable(m_userWidget->count() > 1);
    m_controlWidget->setMPRISEnable(true);

    QHBoxLayout *passwdLayout = new QHBoxLayout;
    passwdLayout->setMargin(0);
    passwdLayout->setSpacing(0);
    passwdLayout->addStretch();
    passwdLayout->addWidget(m_passwordEdit);
    passwdLayout->addWidget(m_unlockButton);
    passwdLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addStretch();
    mainLayout->addLayout(passwdLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
    showFullScreen();
    activateWindow();
    updateWidgetsPosition();
    updateStyle(":/skin/lock.qss", this);

    m_lockInter = new DBusLockService(LOCKSERVICE_NAME, LOCKSERVICE_PATH,
                                      QDBusConnection::systemBus(), this);

    connect(m_lockInter, &DBusLockService::Event, this, &LockManager::lockServiceEvent);

    connect(m_passwordEdit, &PassWdEdit::submit, this, &LockManager::unlock);
    connect(m_userWidget, &UserWidget::userChanged, this, [=] (const QString & username) {
        m_passwordEdit->show();

        qDebug() << "current User:" << username << "11 m_activatedUser:" << m_activatedUser;

        if (username != m_activatedUser) {
            // goto greeter
            QProcess *process = new QProcess;
            connect(process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), process, &QProcess::deleteLater);
            process->start("dde-switchtogreeter " + username);

            m_userWidget->setCurrentUser(m_userWidget->loginUser());

            return;
        }

        checkUserIsNoPWGrp();

        updateBackground(m_activatedUser);
    });

    updateBackground(m_activatedUser);
}

void LockManager::updateWidgetsPosition()
{
    qDebug() << "lockManager: position" << this->width() << this->height();
    const int width = this->width();
    const int height = this->height();
    m_timeWidget->move(48, height - m_timeWidget->height() - 36); // left 48px and bottom 36px
    m_userWidget->setFixedWidth(width);
    if (!m_userWidget->isChooseUserMode)
        m_userWidget->move(0, (height - m_userWidget->height()) / 2 - 95);
    m_requireShutdownWidget->setFixedWidth(width);
    m_requireShutdownWidget->move(0, (height - m_requireShutdownWidget->height()) / 2 - 50);
    m_controlWidget->move(width - m_controlWidget->width(),
                          height - m_controlWidget->height() - 36); // margin right 0 margin bottom 36

}

void LockManager::chooseUserMode()
{
    m_passwordEdit->hide();
    m_unlockButton->hide();
    m_userWidget->show();
    m_requireShutdownWidget->hide();
    m_keybdLayoutWidget->hide();
    m_keybdArrowWidget->hide();
}

void LockManager::onUnlockFinished(const bool unlocked)
{
    if (!unlocked) {
        qDebug() << "Authorization failed!";

        m_lockInter->AuthenticateUser(m_activatedUser);
        return;
    }

    // Auth success
    switch (m_action) {
    case Restart:       m_sessionManagerIter->RequestReboot();    break;
    case Shutdown:      m_sessionManagerIter->RequestShutdown();    break;
    case Suspend:       m_sessionManagerIter->RequestSuspend();     break;
    default: break;
    }

    m_userWidget->saveLastUser();

#ifdef LOCK_NO_QUIT
    m_passwordEdit->setMessage("");
    emit checkedHide();
#else
    qApp->exit();
#endif
}

void LockManager::updateBackground(QString username)
{
    LockFrame *frame = qobject_cast<LockFrame *>(parent());
    frame->setBackground(m_userWidget->getUserGreeterBackground(username));
}

void LockManager::showEvent(QShowEvent *event)
{
    disableZone();

    m_keybdLayoutWidget->hide();
    m_keybdArrowWidget->hide();

    // check user is nopassword group
    checkUserIsNoPWGrp();

    m_controlWidget->setUserSwitchEnable(m_userWidget->count() > 1);
    updateBackground(m_activatedUser);

    m_passwordEdit->setMessage("");

    m_passwordEdit->updateKeybdLayoutUI(m_userWidget->getUserKBHistory(m_userWidget->currentUser()));
    m_keybdLayoutWidget->updateButtonList(m_userWidget->getUserKBHistory(m_userWidget->currentUser()));
    m_keybdLayoutWidget->setDefault(m_userWidget->getUserKBLayout(m_userWidget->currentUser()));

    QFrame::showEvent(event);
}

void LockManager::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    updateWidgetsPosition();
}

void LockManager::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Escape:
        if (!m_requireShutdownWidget->isHidden()) {
            m_requireShutdownWidget->hide();
            m_userWidget->show();
            if (!m_userWidget->isChooseUserMode) {
                if (m_passwordEdit->size() != ZoreSize) {
                    m_passwordEdit->show();
                    m_passwordEdit->setFocus();
                } else {
                    m_unlockButton->show();
                }
            }
        }
#ifdef QT_DEBUG
//        qApp->quit();   break;
#endif
    default:;
    }
}

void LockManager::mouseReleaseEvent(QMouseEvent *e)
{
    qDebug() << "ReleaseEvent";
    m_action = Unlock;
    if (!m_userWidget->isChooseUserMode) {
        passwordMode();
    }
    if (e->button() == Qt::LeftButton) {
        if (!m_keybdArrowWidget->isHidden()) {
            m_keybdArrowWidget->hide();
        }
    }
}

void LockManager::unlock()
{
    m_keybdLayoutWidget->hide();
    m_keybdArrowWidget->hide();

    if (!m_requireShutdownWidget->isHidden()) {
        m_requireShutdownWidget->shutdownAction();
        return;
    }

    if (m_userWidget->isChooseUserMode && !m_userWidget->isHidden()) {
        m_userWidget->chooseButtonChecked();
        m_passwordEdit->getFocusTimer->start();
        qDebug() << "lineEditGrabKeyboard";
        return;
    }

    if (!m_passwordEdit->isVisible())
        return;

//    qDebug() << "unlock" << m_userWidget->currentUser() << m_passwordEdit->getText();
    const QString &username = m_activatedUser;
    const QString &password = m_passwordEdit->getText();

    if (password.isEmpty())
        return;

    if (m_authenticating)
        return;

    m_authenticating = true;

    m_lockInter->UnlockCheck(username, password);
}

void LockManager::lockServiceEvent(quint32 eventType, quint32 pid, const QString &username, const QString &message)
{
    if (username != m_activatedUser)
        return;

    qDebug() << eventType << pid << username << message;

    // Don't show password prompt from standard pam modules since
    // we'll provide our own prompt or just not.
    const QString msg = message == "Password: " ? "" : message;

    m_authenticating = false;

    switch (eventType) {
    case DBusLockService::PromptQuestion:
        qDebug() << "prompt quesiton from pam: " << message;
        m_passwordEdit->setMessage(message);
        break;
    case DBusLockService::PromptSecret:
        qDebug() << "prompt secret from pam: " << message;
        if (msg.isEmpty())
            m_passwordEdit->setAlert(true, tr("Wrong Password"));
        else
            m_passwordEdit->setMessage(msg);
        break;
    case DBusLockService::ErrorMsg:
        qWarning() << "error message from pam: " << message;
        break;
    case DBusLockService::TextInfo:
        m_passwordEdit->setMessage(message);
        break;
    case DBusLockService::Failure:
        onUnlockFinished(false);
        break;
    case DBusLockService::Successed:
        onUnlockFinished(true);
        break;
    default:
        break;
    }
}

void LockManager::checkUserIsNoPWGrp()
{
    bool isNOPWGrp = false;
    do {
        long ngroups_max = sysconf(_SC_NGROUPS_MAX) + 1;
        int ngroups;
        struct group *gr;
        const char *currentUser = m_userWidget->currentUser().toStdString().c_str();
        const QString &checkGrp = "nopasswdlogin";

        gid_t *groups = new gid_t[ngroups_max * sizeof(gid_t)];

        ngroups = getgroups(ngroups_max, groups);

        struct passwd *pw = getpwnam(currentUser);
        if (pw == nullptr) {
            delete []groups;
            break;
        }

        if (getgrouplist(currentUser, pw->pw_gid, groups, &ngroups) == -1) {
            delete []groups;
            break;
        }

        for (int i = 0; i < ngroups; i++) {
            gr = getgrgid(groups[i]);
            if (gr == nullptr)
                continue;
            if (QString(gr->gr_name) == checkGrp) {
                isNOPWGrp = true;
                break;
            }
        }

        delete []groups;

    } while(false);

    if (isNOPWGrp) {
        m_unlockButton->show();
        m_passwordEdit->setFixedSize(ZoreSize);
    } else {
        m_lockInter->AuthenticateUser(m_activatedUser);
        m_unlockButton->hide();
        m_passwordEdit->setFixedSize(m_passwdEditSize);
    }
}

void LockManager::initBackend()
{
    m_hotZoneInterface = new DBusHotzone("com.deepin.daemon.Zone", "/com/deepin/daemon/Zone",
                                         QDBusConnection::sessionBus(), this);
#ifndef LOCK_NO_QUIT
    m_hotZoneInterface->EnableZoneDetected(false);
#endif


    DBusInputDevices *dbusInputDevices = new DBusInputDevices(this);
    foreach(InputDevice device, dbusInputDevices->infos()) {
        if (device.deviceType == "keyboard") {
            m_keyboardLayoutInterface = new DBusKeyboard(this);
            break;
        }
    }

    m_sessionManagerIter = new DBusSessionManagerInterface("com.deepin.SessionManager", "/com/deepin/SessionManager",
            QDBusConnection::sessionBus(), this);
}

void LockManager::updateUI()
{
    m_keybdLayoutWidget = new KbLayoutWidget;

    m_keybdArrowWidget = new DArrowRectangle(DArrowRectangle::ArrowTop, this);

    m_keybdArrowWidget->setBackgroundColor(QColor::fromRgbF(1, 1, 1, 0.15));
    m_keybdArrowWidget->setBorderColor(Qt::transparent);
    m_keybdArrowWidget->setArrowX(17);
    m_keybdArrowWidget->setArrowWidth(12);
    m_keybdArrowWidget->setArrowHeight(6);
    m_keybdArrowWidget->setMargin(1);

    m_keybdArrowWidget->setContent(m_keybdLayoutWidget);
    m_keybdLayoutWidget->setParent(m_keybdArrowWidget);
    m_keybdLayoutWidget->show();
    m_keybdArrowWidget->move(m_passwordEdit->x() + 123, m_passwordEdit->y() + m_passwordEdit->height() - 15);
    m_keybdArrowWidget->hide();

    connect(m_keybdLayoutWidget, &KbLayoutWidget::setButtonClicked, this, &LockManager::setCurrentKeyboardLayout);
    connect(m_keybdLayoutWidget, &KbLayoutWidget::setButtonClicked, m_keybdArrowWidget, &DArrowRectangle::hide);
}

void LockManager::enableZone()
{
    m_hotZoneInterface->EnableZoneDetected(true);
}

void LockManager::disableZone()
{
    m_hotZoneInterface->EnableZoneDetected(false);
}

ControlWidget *LockManager::control()
{
    return m_controlWidget;
}

void LockManager::setCurrentKeyboardLayout(QString keyboard_value)
{
    m_userWidget->setUserKBlayout(m_userWidget->currentUser(), keyboard_value);
}

void LockManager::passwordMode()
{
//    m_userWidget->setCurrentUser(m_activatedUser);
    m_userWidget->show();
    m_requireShutdownWidget->hide();

    if (m_passwordEdit->size() != ZoreSize) {
        m_passwordEdit->show();
        m_passwordEdit->setFocus();
    } else {
        m_unlockButton->show();
    }

    if (m_action == Suspend) {
        m_sessionManagerIter->RequestSuspend().waitForFinished();
        m_action = Unlock;
        return;
    }

    if (m_action == Restart) {
        m_passwordEdit->setAlert(true, tr("Enter your password to restart"));
        m_passwordEdit->setEnterBtnStyle(":/img/action_icons/reboot_normal.svg",
                                         ":/img/action_icons/reboot_hover.svg",
                                         ":/img/action_icons/reboot_press.svg");
    } else if (m_action == Shutdown) {
        m_passwordEdit->setAlert(true, tr("Enter your password to shutdown"));
        m_passwordEdit->setEnterBtnStyle(":/img/action_icons/shutdown_normal.svg",
                                         ":/img/action_icons/shutdown_hover.svg",
                                         ":/img/action_icons/shutdown_press.svg");
    } else if (m_action == Unlock) {
        m_passwordEdit->setEnterBtnStyle(":/img/action_icons/unlock_normal.svg",
                                         ":/img/action_icons/unlock_hover.svg",
                                         ":/img/action_icons/unlock_press.svg");
    }
}

void LockManager::shutdownMode()
{
    m_userWidget->hide();
    m_passwordEdit->hide();
	m_unlockButton->hide();
    m_requireShutdownWidget->show();
}
