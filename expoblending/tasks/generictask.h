/* ============================================================
 * 
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2013-06-28
 * Description : a plugin to blend bracketed images.
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2013 by Soumajyoti Sarkar <ergy dot ergy at gmail dot com>
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


#ifndef GENERICTASK_H
#define GENERICTASK_H

// KDE includes

#include <threadweaver/Job.h>
#include <kprocess.h>
#include <ktempdir.h>


#include "task.h"

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/rawdecodingsettings.h>

//Local includes
#include "enfusesettings.h"
#include "actions.h"
#include "kpmetadata.h"


using namespace KIPIPlugins;
using namespace KDcrawIface;

namespace KIPIExpoBlendingPlugin
{

class GenericTask : public Task
{
  
   Q_OBJECT
   
    
public:
    
    bool                		cancel;

    KUrl::List          		urls;
    Action              		action;
    KUrl                		outputUrl;
    EnfuseSettings      		enfuseSettings;
    const RawDecodingSettings   	settings;
    bool 				align;
    QString             		binaryPath;
    bool 				enfuseVersion4x;
   
    
    KProcess*           		enfuseProcess;
    KProcess*           		alignProcess;
    
    QString      			errString;
    
    KTempDir*                        	preprocessingTmpDir;

protected:

    bool         successFlag;
    bool         isAbortedFlag;
    const KUrl   tmpDir;

public:

    GenericTask(QObject* const parent, const KUrl::List& fileUrl, const Action& action);
    GenericTask(QObject* const parent, const KUrl::List& fileUrl, const Action& action, 
			 const RawDecodingSettings& rawSettings, const bool align, const QString& alignPath);
    GenericTask(QObject* const parent, const KUrl::List& fileUrl, const Action& action, const KUrl& outputUrl,
			         const EnfuseSettings& settings, const QString& alignPath, bool version);
    GenericTask(const KUrl::List& fileUrl, const Action& action, const KUrl& outputUrl,
			         const EnfuseSettings& settings, const QString& alignPath, bool version);

    ~GenericTask();

    bool success() const;
    void requestAbort();
    
    void setEnfuseVersion(const double version);
    bool getXmpRational(const char* xmpTagName, long& num, long& den, KPMetadata& meta);
    float getAverageSceneLuminance(const KUrl& url);
    bool startEnfuse(const KUrl::List& inUrls, KUrl& outUrl,
                               const EnfuseSettings& settings,
                               const QString& enfusePath, QString& errors);
    
    bool startPreProcessing(const KUrl::List& inUrls, ItemUrlsMap& preProcessedUrlsMap,
                                      bool align, const RawDecodingSettings& settings,
                                      const QString& alignPath, QString& errors);
    bool computePreview(const KUrl& inUrl, KUrl& outUrl);
    bool convertRaw(const KUrl& inUrl, KUrl& outUrl, const RawDecodingSettings& settings);
    void cleanAlignTmpDir();

Q_SIGNALS:

    void starting(const KIPIExpoBlendingPlugin::ActionData& ad);
    void finished(const KIPIExpoBlendingPlugin::ActionData& ad);


protected:

    void run() ;
    QString getProcessError(KProcess* const proc) const;
        
};

}  // namespace KIPIExpoBlendingPlugin

#endif /* GENERICTASK_H */
