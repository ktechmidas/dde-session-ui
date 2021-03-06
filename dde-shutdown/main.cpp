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

#include <QApplication>
#include <QtCore/QTranslator>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDBusAbstractInterface>

#include <DLog>

#include "app/shutdownframe.h"
#include "dbus/dbuscontrolcenter.h"

const QString DBUS_PATH = "/com/deepin/dde/shutdownFront";
const QString DBUS_NAME = "com.deepin.dde.shutdownFront";

DCORE_USE_NAMESPACE

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName("dde-shutdown");
    qApp->setApplicationVersion("2015.1.0");
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // hide dde-control-center
    DBusControlCenter *DCCInter = new DBusControlCenter;
    if (DCCInter->isValid())
        DCCInter->HideImmediately();
    DCCInter->deleteLater();

    QTranslator translator;
    translator.load("/usr/share/dde-session-ui/translations/dde-session-ui_" + QLocale::system().name());
    app.installTranslator(&translator);

//    QCommandLineOption hide(QStringList() << "H" << "Hide", "hide buttons", "btns");
//    QCommandLineOption disable(QStringList() << "D" << "Disable", "disable buttons", "btns");

    // NOTE: it's better to be -h/--show, but some apps may have used the binary to show
    // the shutdown frame directly, we need to keep the default behavier to that.
    QCommandLineOption daemon(QStringList() << "d" << "daemon",
                              "start the program, but do not show the window.");
    QCommandLineParser parser;
    parser.addHelpOption();
//    parser.addOption(hide);
//    parser.addOption(disable);
    parser.addOption(daemon);
    parser.process(app);

    QDBusConnection session = QDBusConnection::sessionBus();
    if(session.registerService(DBUS_NAME)){
        qDebug() << "dbus registration success.";

        ShutdownFrame w;
        //    w.hideBtns(parser.values(hide));
        //    w.disableBtns(parser.values(disable));
        if (!parser.isSet(daemon)) {
            w.showFullScreen();
        }

        ShutdownFrontDBus adaptor(&w); Q_UNUSED(adaptor);
        QDBusConnection::sessionBus().registerObject(DBUS_PATH, &w);

        return app.exec();
    } else {
        qWarning() << "dde-shutdown is running...";

        if (!parser.isSet(daemon)) {
            const char* interface = "com.deepin.dde.shutdownFront";
            QDBusInterface ifc(DBUS_NAME, DBUS_PATH, interface, session, NULL);
            ifc.asyncCall("Show");
        }

        return 0;
    }

    return app.exec();
}
