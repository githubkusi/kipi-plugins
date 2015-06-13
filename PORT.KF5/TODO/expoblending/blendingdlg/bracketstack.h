/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2009-12-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef BRACKETSTACK_H
#define BRACKETSTACK_H

// Qt includes

#include <QTreeWidget>
#include <QString>
#include <QPixmap>

// KDE includes

#include <kurl.h>
#include <kfileitem.h>

namespace KIPI
{
class Interface;
}

using namespace KIPI;

namespace KIPIExpoBlendingPlugin
{


class BracketStackItem : public QTreeWidgetItem
{

public:

    explicit BracketStackItem(QTreeWidget* const parent);
    virtual ~BracketStackItem();

    void setUrl(const QUrl& url);
    QUrl url() const;

    void setOn(bool b);
    bool isOn() const;

    void setThumbnail(const QPixmap& pix);
    void setExposure(const QString& exp);

private:

    bool operator< (const QTreeWidgetItem& other) const;

private:

    QUrl m_url;
};

// ---------------------------------------------------------------------

class BracketStackList : public QTreeWidget
{
    Q_OBJECT

public:

    BracketStackList(Interface* iface, QWidget* parent);
    virtual ~BracketStackList();

    void addItems(const QUrl::List& list);

    QUrl::List urls();
    BracketStackItem* findItem(const QUrl& url);

Q_SIGNALS:

    void signalAddItems(const QUrl::List&);

private Q_SLOTS:

    void slotKDEPreview(const KFileItem&, const QPixmap&);
    void slotKDEPreviewFailed(const KFileItem&);
    void slotRawThumb(const QUrl&, const QImage&);
    void slotThumbnail(const QUrl& url, const QPixmap& pix);

private:

    class BracketStackListPriv;
    BracketStackListPriv* const d;
};

}  // namespace KIPIExpoBlendingPlugin

#endif /* BRACKETSTACK_H */