/* ============================================================
 * File  : plugin_jpeglossless.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-09-26
 * Description : JPEG loss less operations plugin
 *
 * Copyright 2003 by Renchi Raju & Gilles Caulier

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <iostream>

#include <klocale.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kurl.h>

#include <qdir.h>

#include <digikam/albummanager.h>
#include <digikam/albuminfo.h>

#include "actions.h"
#include "actionthread.h"
#include "progressdlg.h"
#include "messagebox.h"
#include "plugin_jpeglossless.h"

K_EXPORT_COMPONENT_FACTORY( kipiplugin_jpeglossless,
                            KGenericFactory<Plugin_JPEGLossless>("kipiplugin_jpeglossless"));


/////////////////////////////////////////////////////////////////////////////////////////////////////

Plugin_JPEGLossless::Plugin_JPEGLossless(QObject *parent,
                                         const char*,
                                         const QStringList &)
    : KIPI::Plugin(parent, "JPEGLossless")
{
    KGlobal::locale()->insertCatalogue("kipiplugin_jpeglossless");

    kdDebug() << "Plugin_JPEGLossless plugin loaded" << endl;

    // Main submenu for JPEGLossLess plugin transform actions.

    m_action_Transform = new KActionMenu(i18n("&Transform"),
                         actionCollection(),
                         "jpeglossless_transform");

    m_action_RotateImage = new KActionMenu(i18n("Rotate"),
                           "rotate_cw",
                           m_action_Transform,
                           "jpeglossless_rotate");

    m_action_RotateImage->insert( new KAction(i18n("90 degrees"),
                                0,
                                Key_1,
                                this,
                                SLOT(slotRotate()),
                                m_action_RotateImage,
                                "rotate_90") );

    m_action_RotateImage->insert( new KAction(i18n("180 degrees"),
                                0,
                                Key_2,
                                this,
                                SLOT(slotRotate()),
                                m_action_RotateImage,
                                "rotate_180") );

    m_action_RotateImage->insert( new KAction(i18n("270 degrees"),
                                0,
                                Key_3,
                                this,
                                SLOT(slotRotate()),
                                m_action_RotateImage,
                                "rotate_270") );

    m_action_FlipImage = new KActionMenu(i18n("Flip"),
                           "flip_image",
                           m_action_Transform,
                           "jpeglossless_flip");

    m_action_FlipImage->insert( new KAction(i18n("Horizontally"),
                                0,
                                this,
                                SLOT(slotFlip()),
                                m_action_FlipImage,
                                "flip_horizontal") );

    m_action_FlipImage->insert( new KAction(i18n("Vertically"),
                                0,
                                this,
                                SLOT(slotFlip()),
                                m_action_FlipImage,
                                "flip_vertical") );

    m_action_Convert2GrayScale = new KAction(i18n("Convert to Black and White"),
                                             "grayscaleconvert",
                                             0,
                                             this,
                                             SLOT(slotConvert2GrayScale()),
                                             m_action_Transform,
                                             "jpeglossless_convert2grayscale");

    m_action_Transform->insert(m_action_RotateImage);
    m_action_Transform->insert(m_action_FlipImage);
    m_action_Transform->insert(m_action_Convert2GrayScale);

    m_action_RotateImage->setEnabled(false);
    m_action_FlipImage->setEnabled(false);
    m_action_Convert2GrayScale->setEnabled(false);

    m_thread      = new JPEGLossLess::ActionThread(this);
    m_progressDlg = 0;


    connect(Digikam::AlbumManager::instance(),
            SIGNAL(signalAlbumItemsSelected(bool)),
            SLOT(slotItemsSelected(bool)));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

Plugin_JPEGLossless::~Plugin_JPEGLossless()
{
    // Remove JPEGLossLess plugin temporary folder in KDE tmp directory.

    delete m_thread;

    if (m_progressDlg)
        delete m_progressDlg;

    if (JPEGLossLess::MessageBox::instance())
        delete JPEGLossLess::MessageBox::instance();

}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin_JPEGLossless::slotFlip()
{
    Digikam::AlbumInfo* album =
        Digikam::AlbumManager::instance()->currentAlbum();
    if (!album) return;

    QStringList items(album->getSelectedItemsPath());
    if (items.count() <= 0) return;

    QString from(sender()->name());

    bool proceed = false;

    if (from == "flip_horizontal") {
        m_thread->flip(items, JPEGLossLess::FlipHorizontal);
        proceed = true;
    }
    else if (from == "flip_vertical") {
        m_thread->flip(items, JPEGLossLess::FlipVertical);
        proceed = true;
    }
    else {
        kdWarning() << "The impossible happened... unknown flip specified" << endl;
        return;
    }

    if (!proceed) return;
    m_total   = items.count();
    m_current = 0;

    if (!m_progressDlg) {
        m_progressDlg = new JPEGLossLess::ProgressDlg;
        connect(m_progressDlg, SIGNAL(signalCanceled()),
                SLOT(slotCancel()));
    }
    m_progressDlg->show();

    if (!m_thread->running())
        m_thread->start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin_JPEGLossless::slotRotate()
{
    Digikam::AlbumInfo* album = Digikam::AlbumManager::instance()->currentAlbum();
    if (!album) return;

    QStringList items(album->getSelectedItemsPath());
    if (items.count() <= 0) return;

    QString from(sender()->name());

    bool proceed = false;

    if (from == "rotate_90") {
        m_thread->rotate(items, JPEGLossLess::Rot90);
        proceed = true;
    }
    else if (from == "rotate_180") {
        m_thread->rotate(items, JPEGLossLess::Rot180);
        proceed = true;
    }
    else if (from == "rotate_270") {
        m_thread->rotate(items, JPEGLossLess::Rot270);
        proceed = true;
    }
    else {
        kdWarning() << "The impossible happened... unknown rotation angle specified" << endl;
        return;
    }

    if (!proceed) return;
    m_total   = items.count();
    m_current = 0;

    if (!m_progressDlg) {
        m_progressDlg = new JPEGLossLess::ProgressDlg;
        connect(m_progressDlg, SIGNAL(signalCanceled()),
                SLOT(slotCancel()));
    }
    m_progressDlg->show();

    if (!m_thread->running())
        m_thread->start();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin_JPEGLossless::slotConvert2GrayScale()
{
    Digikam::AlbumInfo* album = Digikam::AlbumManager::instance()->currentAlbum();
    if (!album) return;

    QStringList items(album->getSelectedItemsPath());
    if (items.count() <= 0) return;

    QString from(sender()->name());

    m_total   = items.count();
    m_current = 0;

    if (!m_progressDlg) {
        m_progressDlg = new JPEGLossLess::ProgressDlg;
        connect(m_progressDlg, SIGNAL(signalCanceled()),
                SLOT(slotCancel()));
    }
    m_progressDlg->show();

    m_thread->convert2grayscale(items);
    if (!m_thread->running())
        m_thread->start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin_JPEGLossless::slotCancel()
{
    m_thread->cancel();
    if (m_progressDlg) {
        m_progressDlg->reset();
    }

    Digikam::AlbumManager* man = Digikam::AlbumManager::instance();
    if (!man->currentAlbum()) return;
    man->refreshItemHandler(man->currentAlbum()->getSelectedItems());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin_JPEGLossless::slotItemsSelected(bool val)
{
    m_action_Transform->setEnabled(val);
    m_action_FlipImage->setEnabled(val);
    m_action_RotateImage->setEnabled(val);
    m_action_Convert2GrayScale->setEnabled(val);
}

void Plugin_JPEGLossless::customEvent(QCustomEvent *event)
{
    if (!event) return;

    JPEGLossLess::EventData *d = (JPEGLossLess::EventData*) event->data();
    if (!d) return;
    if (d->starting) {
        QString text;
        switch (d->action) {
        case(JPEGLossLess::Rotate): {
            text = i18n("Rotating Image\n%1").arg(d->fileName);
            break;
        }
        case(JPEGLossLess::Flip): {
            text = i18n("Flipping Image\n%1").arg(d->fileName);
            break;
        }
        case(JPEGLossLess::GrayScale): {
            text = i18n("Converting to Black & White\n%1").arg(d->fileName);
            break;
        }
        default: {
            kdWarning() << "Plugin_JPEGLossLess: Unknown event" << endl;
        }
        }

        m_progressDlg->setText(text);
    }
    else {

        if (!d->success) {

            QString text;
            switch (d->action) {
            case(JPEGLossLess::Rotate): {
                text = i18n("Failed to Rotate");
                break;
            }
            case(JPEGLossLess::Flip): {
                text = i18n("Failed to flip Image");
                break;
            }
            case(JPEGLossLess::GrayScale): {
                text = i18n("Failed to convert to Black & White");
                break;
            }
            default: {
                kdWarning() << "Plugin_JPEGLossLess: Unknown event" << endl;
            }
            }

            JPEGLossLess::MessageBox::showMsg(d->fileName, text);
        }

        m_current++;
        m_progressDlg->setProgress(m_current, m_total);
    }


    delete d;

    if (m_current >= m_total) {
        m_current     = 0;
        m_progressDlg->reset();
        Digikam::AlbumManager* man = Digikam::AlbumManager::instance();
        if (!man->currentAlbum()) return;
        man->refreshItemHandler(man->currentAlbum()->getSelectedItems());
    }
}

KIPI::Category Plugin_JPEGLossless::category() const
{
    return KIPI::IMAGESPLUGIN;
}
