/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a plugin to blend bracketed images.
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "intropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QGroupBox>

// KDE includes

#include <KLocalizedString>

// local includes

#include "kpbinarysearch.h"
#include "alignbinary.h"
#include "enfusebinary.h"

namespace KIPIExpoBlendingPlugin
{

class IntroPage::IntroPagePriv
{
public:

    IntroPagePriv(Manager* const m)
        : mngr(m), 
          binariesWidget(0)
    {
    }

    Manager*                     mngr;
    KIPIPlugins::KPBinarySearch* binariesWidget;
};

IntroPage::IntroPage(Manager* const mngr, KAssistantDialog* const dlg)
    : KPWizardPage(dlg, i18nc("@title:window", "Welcome to Exposure Blending Tool")),
      d(new IntroPagePriv(mngr))
{
    QVBoxLayout* const vbox = new QVBoxLayout();

    QLabel* const title     = new QLabel(this);
    title->setWordWrap(true);
    title->setOpenExternalLinks(true);
    title->setText(i18n("<qt>"
                        "<p><h1><b>Welcome to Exposure Blending tool</b></h1></p>"
                        "<p>This tool fuses bracketed images with different exposure to make pseudo "
                        "<a href='http://en.wikipedia.org/wiki/High_dynamic_range_imaging'>HDR image</a>.</p>"
                        "<p>It can also be used to merge focus bracketed stack to get a single image "
                        "with increased depth of field.</p>"
                        "<p>This assistant will help you to configure how to import images before "
                        "merging them to a single one.</p>"
                        "<p>Bracketed images must be taken with the same camera, "
                        "in the same conditions, and if possible using a tripod.</p>"
                        "<p>For more information, please take a look at "
                        "<a href='http://en.wikipedia.org/wiki/Bracketing'>this page</a></p>"
                        "</qt>"));
    vbox->addWidget(title);

    QGroupBox* binaryBox        = new QGroupBox(this);
    QGridLayout* binaryLayout   = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "Exposure Blending Binaries"));
    d->binariesWidget = new KIPIPlugins::KPBinarySearch(binaryBox);
    d->binariesWidget->addBinary(d->mngr->alignBinary());
    d->binariesWidget->addBinary(d->mngr->enfuseBinary());
#ifdef Q_OS_MAC
    d->binariesWidget->addDirectory("/Applications/Hugin/HuginTools");    // Hugin bundle PKG install
    d->binariesWidget->addDirectory("/opt/local/bin");                    // Std Macports install
    d->binariesWidget->addDirectory("/opt/digikam/bin");                  // digiKam Bundle PKG install
#endif
    vbox->addWidget(binaryBox);

    connect(d->binariesWidget, SIGNAL(signalBinariesFound(bool)),
            this, SIGNAL(signalIntroPageIsValid(bool)));

    emit signalIntroPageIsValid(d->binariesWidget->allBinariesFound());

    setLayout(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromUtf8("kipiplugin_expoblending/pics/assistant-stack.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));
}

IntroPage::~IntroPage()
{
}

bool IntroPage::binariesFound()
{
    return d->binariesWidget->allBinariesFound();
}

}   // namespace KIPIExpoBlendingPlugin
