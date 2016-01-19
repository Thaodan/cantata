/*
 * Cantata
 *
 * Copyright (c) 2011-2016 Craig Drummond <craig.p.drummond@gmail.com>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "monoicon.h"
#include "utils.h"
#include <QFile>
#include <QIconEngine>
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmapCache>
#include <QRect>
#include <QFontDatabase>

class MonoIconEngine : public QIconEngine
{
public:
    MonoIconEngine(const QString &file, FontAwesome::icon fa, const QColor &col, const QColor &sel)
        : fileName(file)
        , fontAwesomeIcon(fa)
        , color(col)
        , selectedColor(sel)
    {
    }

    virtual ~MonoIconEngine() {}

    MonoIconEngine * clone() const
    {
        return new MonoIconEngine(fileName, fontAwesomeIcon, color, selectedColor);
    }

    virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
    {
        Q_UNUSED(state)

        QColor col=QIcon::Selected==mode ? selectedColor : color;
        QString key=(fileName.isEmpty() ? QString::number(fontAwesomeIcon) : fileName)+
                    QLatin1Char('-')+QString::number(rect.width())+QLatin1Char('-')+QString::number(rect.height())+QLatin1Char('-')+col.name();
        QPixmap pix;

        if (!QPixmapCache::find(key, &pix)) {
            pix=QPixmap(rect.width(), rect.height());
            pix.fill(Qt::transparent);
            QPainter p(&pix);

            if (fileName.isEmpty()) {
                // Load fontawesome, if it is not already loaded
                if (fontAwesomeFontName.isEmpty()) {
                    Q_INIT_RESOURCE(fontawesome);
                    QFile res(":fontawesome-4.3.0.ttf");
                    res.open(QIODevice::ReadOnly);
                    QByteArray fontData( res.readAll() );
                    res.close();

                    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFontFromData(fontData));
                    if (!loadedFontFamilies.empty()) {
                        fontAwesomeFontName= loadedFontFamilies.at(0);
                    }
                }
                double scale=1.0;

                switch (fontAwesomeIcon) {
                case FontAwesome::lastfmsquare:
                case FontAwesome::lastfm:
                    scale=1.1;
                    break;
                case FontAwesome::list:
                    if (!Utils::isHighDpi()) {
                        scale=1.05;
                    }
                default:
                    scale=0.9;
                    break;
                }

                QFont font(fontAwesomeFontName);
                font.setPixelSize(qRound(rect.height()*scale));
                p.setFont(font);
                p.setPen(col);
                p.drawText(QRect(0, 0, rect.width(), rect.height()), QString(QChar(static_cast<int>(fontAwesomeIcon))), QTextOption(Qt::AlignCenter|Qt::AlignVCenter));
            } else {
                QSvgRenderer renderer;
                QFile f(fileName);
                QByteArray bytes;
                if (f.open(QIODevice::ReadOnly)) {
                    bytes=f.readAll();
                }
                if (!bytes.isEmpty()) {
                    bytes.replace("#000", col.name().toLatin1());
                }
                renderer.load(bytes);
                renderer.render(&p, QRect(0, 0, rect.width(), rect.height()));
            }
            QPixmapCache::insert(key, pix);
        }
        if (QIcon::Disabled==mode) {
            painter->save();
            painter->setOpacity(painter->opacity()*0.35);
        }
        painter->drawPixmap(rect.topLeft(), pix);
        if (QIcon::Disabled==mode) {
            painter->restore();
        }
    }

    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
    {
        QPixmap pix(size);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        paint(&painter, QRect(QPoint(0, 0), size), mode, state);
        return pix;
    }

private:
    QString fileName;
    FontAwesome::icon fontAwesomeIcon;
    QColor color;
    QColor selectedColor;
    static QString fontAwesomeFontName;
};

QString MonoIconEngine::fontAwesomeFontName;

QIcon MonoIcon::icon(const QString &fileName, const QColor &col, const QColor &sel)
{
    return QIcon(new MonoIconEngine(fileName, (FontAwesome::icon)0, col, sel));
}

QIcon MonoIcon::icon(const FontAwesome::icon icon, const QColor &col, const QColor &sel)
{
    return QIcon(new MonoIconEngine(QString(), icon, col, sel));
}
