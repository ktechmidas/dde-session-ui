/*
 * Copyright (C) 2011 ~ 2017 Deepin Technology Co., Ltd.
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

#include "common.h"
#include <QSvgRenderer>

void DrawHelper::DrawImage(QPainter *painter, const QStyleOptionViewItem &option, const QString &pix, bool withText, bool withProgress)
{
    const QRect rect(option.rect);
    const int yOffset = withProgress ? 30 : withText ? 20 : 40;
    QPixmap pixmap(pix);

    QSvgRenderer renderer(pix);
    renderer.render(painter, QRect(QPoint(rect.x() + (rect.width() - pixmap.width()) / 2, rect.y() + yOffset), pixmap.size()));
}

void DrawHelper::DrawText(QPainter *painter, const QStyleOptionViewItem &option, const QString &text, QColor color, bool withImage)
{
    QRect rect( option.rect );
    if (withImage) {
        rect.setY(rect.y() + 70);
    }

    QTextOption opt;
    opt.setAlignment(Qt::AlignCenter);

    painter->setPen(color);
    painter->drawText(rect, text, opt);
}

void DrawHelper::DrawProgressBar(QPainter *painter, const QStyleOptionViewItem &option, double progress)
{
    const QRect rect( option.rect );
    const int progressBarWidth = 80;
    const int progressBarHeight = 4;
    const float radius = progressBarHeight / 2.f;

    painter->setPen(Qt::NoPen);

    QRect dest(rect.x() + (rect.width() - progressBarWidth) / 2, rect.y() + 110, progressBarWidth, progressBarHeight);
    painter->setBrush(QColor(0, 0, 0, 25));
    painter->drawRoundedRect(dest, radius, radius);

    dest.setWidth(dest.width() * progress);
    painter->setBrush(Qt::black);
    painter->drawRoundedRect(dest, radius, radius);
}
