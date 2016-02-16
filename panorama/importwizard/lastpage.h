/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a plugin to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending plugin
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LAST_PAGE_H
#define LAST_PAGE_H

// Local includes

#include "kpwizardpage.h"
#include "actions.h"

using namespace KIPIPlugins;

namespace KIPIPanoramaPlugin
{

class Manager;

class LastPage : public KPWizardPage
{
    Q_OBJECT

public:

    LastPage(Manager* const mngr, KPWizardDialog* const dlg);
    ~LastPage();

private:

    void copyFiles();
    void checkFiles();
    QString panoFileName(const QString& fileTemplate) const;

    void initializePage();
    bool validatePage();

Q_SIGNALS:

    void signalCopyFinished();

private Q_SLOTS:

    void slotTemplateChanged(const QString&);
    void slotPtoCheckBoxChanged(int);
    void slotAction(const KIPIPanoramaPlugin::ActionData&);

private:

    struct Private;
    Private* const d;
};

}   // namespace KIPIPanoramaPlugin

#endif /* LAST_PAGE_H */
