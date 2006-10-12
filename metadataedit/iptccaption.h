/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-10-12
 * Description : IPTC caption settings page.
 * 
 * Copyright 2006 by Gilles Caulier
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

#ifndef IPTC_CAPTION_H
#define IPTC_CAPTION_H

// Qt includes.

#include <qwidget.h>
#include <qcstring.h>

namespace KIPIMetadataEditPlugin
{

class IPTCCaptionPriv;

class IPTCCaption : public QWidget
{
    Q_OBJECT
    
public:

    IPTCCaption(QWidget* parent, QByteArray& iptcData);
    ~IPTCCaption();

    void applySettings(QByteArray& iptcData);

private:

    void readSettings(QByteArray& iptcData);

private:

    IPTCCaptionPriv* d;
};

}  // namespace KIPIMetadataEditPlugin

#endif // IPTC_CAPTION_H 
