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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace DDESESSIONCC
{

const QString CONFIG_FILE("/var/lib/lightdm/lightdm-deepin-greeter/state");
const QString LOCK_SHUTDOWN_FLAG("/tmp/expandstate");
const QString DEFAULT_CURSOR_THEME("/usr/share/icons/default/index.theme");

const int PASSWDLINEEIDT_WIDTH = 250;
const int LAYOUTBUTTON_HEIGHT =  36;

const int CapslockWarningWidth = 23;
const int CapslockWarningRightMargin = 8;
}


#endif // CONSTANTS_H

