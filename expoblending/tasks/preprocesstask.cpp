/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a plugin to blend bracketed images.
 *
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

#include "preprocesstask.h"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <ktempdir.h>
// Local includes

#include "kpmetadata.h"
#include "kpversion.h"
#include "kpwriteimage.h"

using namespace KIPIPlugins;

namespace KIPIExpoBlendingPlugin
{

PreProcessTask::PreProcessTask(QObject* const parent, const KUrl& workDir, int id, ItemPreprocessedUrls& targetUrls,
                               const KUrl& sourceUrl, const RawDecodingSettings& rawSettings, 
			       const KUrl::List& fileUrl,const QString& alignPath, const bool align)
    : Task(parent, PREPROCESS_INPUT, workDir), id(id), preProcessedUrl(&targetUrls),
      fileUrl(sourceUrl), settings(rawSettings),
      urls(fileUrl),binaryPath(alignPath), align(align)
{}

PreProcessTask::PreProcessTask(const KUrl& workDir, int id, ItemPreprocessedUrls& targetUrls,
                               const KUrl& sourceUrl, const RawDecodingSettings& rawSettings, 
			       const KUrl::List& fileUrl,const QString& alignPath, const bool align)
    : Task(0, PREPROCESS_INPUT, workDir), id(id), preProcessedUrl(&targetUrls),
      fileUrl(sourceUrl), settings(rawSettings),
      urls(fileUrl),binaryPath(alignPath), align(align)
{}

PreProcessTask::~PreProcessTask()
{}

void PreProcessTask::requestAbort()
{
    Task::requestAbort();

    if (!rawProcess.isNull())
    {
        rawProcess->cancel();
    }
}

void PreProcessTask::run()
{
    if (KPMetadata::isRawFile(fileUrl))
    {
        preProcessedUrl->preprocessedUrl = tmpDir;

        if (!convertRaw())
        {
            successFlag = false;
            return;
        }
    }
    else
    {
        // NOTE: in this case, preprocessed Url is the original file Url.
        preProcessedUrl->preprocessedUrl = fileUrl;
    }

    preProcessedUrl->previewUrl          = tmpDir;

    if (!computePreview(preProcessedUrl->preprocessedUrl))
    {
        successFlag = false;
        return;
    }

    successFlag = true;
    
    ItemUrlsMap preProcessedUrlsMap;
    QString     errors;
    
    if(!enfuseAlign(urls, preProcessedUrlsMap, align, binaryPath, errors))
    {
      successFlag = false;
      return;
    }
    
    return;
}

bool PreProcessTask::enfuseAlign(const KUrl::List& inUrls, ItemUrlsMap& preProcessedUrlsMap,
                                        bool align, const QString& alignPath, QString& errors)
{
  
    QString prefix = KStandardDirs::locateLocal("tmp", QString("kipi-expoblending-preprocessing-tmp-") +
                                                       QString::number(QDateTime::currentDateTime().toTime_t()));
    KTempDir* preprocessingTmpDir = new KTempDir(prefix);
    
    if (align)
    {
        // Re-align images

        alignProcess = new KProcess;
        alignProcess->clearProgram();
        alignProcess->setWorkingDirectory(preprocessingTmpDir->name());
        alignProcess->setOutputChannelMode(KProcess::MergedChannels);

        QStringList args;
        args << alignPath;
        args << "-v";
        args << "-a";
        args << "aligned";

        foreach(const KUrl& url, inUrls)
        {
            args << url.toLocalFile();
        }

        alignProcess->setProgram(args);

        kDebug() << "Align command line: " << alignProcess->program();

        alignProcess->start();

        if (!alignProcess->waitForFinished(-1))
        {
            errors = getProcessError(alignProcess);
            return false;
        }

        uint    i=0;
        QString temp;
        preProcessedUrlsMap.clear();

        foreach(const KUrl& url, inUrls)
        {
            KUrl previewUrl;
            KUrl alignedUrl = KUrl(preprocessingTmpDir->name() + temp.sprintf("aligned%04i", i) + QString(".tif"));
            if (!computePreview(alignedUrl, previewUrl, preprocessingTmpDir))
                return false;

            preProcessedUrlsMap.insert(url, ItemPreprocessedUrls(alignedUrl, previewUrl));
            i++;
        }

        for (QMap<KUrl, ItemPreprocessedUrls>::const_iterator it = preProcessedUrlsMap.constBegin() ; it != preProcessedUrlsMap.constEnd(); ++it)
        {
            kDebug() << "Pre-processed output urls map: " << it.key() << " , "
                                                          << it.value().preprocessedUrl << " , "
                                                          << it.value().previewUrl << " ; ";
        }
        kDebug() << "Align exit status    : "         << alignProcess->exitStatus();
        kDebug() << "Align exit code      : "         << alignProcess->exitCode();

        if (alignProcess->exitStatus() != QProcess::NormalExit)
            return false;

        if (alignProcess->exitCode() == 0)
        {
            // Process finished successfully !
            return true;
        }

        errors = getProcessError(alignProcess);
        return false;
    }
    else
    {
        for (QMap<KUrl, ItemPreprocessedUrls>::const_iterator it = preProcessedUrlsMap.constBegin() ; it != preProcessedUrlsMap.constEnd(); ++it)
        {
            kDebug() << "Pre-processed output urls map: " << it.key() << " , "
                                                          << it.value().preprocessedUrl << " , "
                                                          << it.value().previewUrl << " ; ";
        }
        kDebug() << "Alignment not performed.";
        return true;
    }
}


bool PreProcessTask::computePreview(const KUrl& inUrl)
{
    KUrl& outUrl = preProcessedUrl->previewUrl;

    QFileInfo fi(inUrl.toLocalFile());
    outUrl.setFileName(fi.completeBaseName().replace('.', '_') + QString("-preview.jpg"));

    QImage img;
    if (img.load(inUrl.toLocalFile()))
    {
        QImage preview = img.scaled(1280, 1024, Qt::KeepAspectRatio);
        bool saved     = preview.save(outUrl.toLocalFile(), "JPEG");
        // save exif information also to preview image for auto rotation
        if (saved)
        {
            KPMetadata metaIn(inUrl.toLocalFile());
            KPMetadata metaOut(outUrl.toLocalFile());
            metaOut.setImageOrientation(metaIn.getImageOrientation());
            metaOut.setImageDimensions(QSize(preview.width(), preview.height()));
            metaOut.applyChanges();
        }
        kDebug() << "Preview Image url: " << outUrl << ", saved: " << saved;
        return saved;
    }
    else
    {
        errString = i18n("Input image cannot be loaded for preview generation");
    }
    return false;
}

bool PreProcessTask::computePreview(const KUrl& inUrl, KUrl& outUrl, KTempDir* preprocessingTmpDir)
{
    outUrl = preprocessingTmpDir->name();
    QFileInfo fi(inUrl.toLocalFile());
    outUrl.setFileName(QString(".") + fi.completeBaseName().replace('.', '_') + QString("-preview.jpg"));

    QImage img;
    if (img.load(inUrl.toLocalFile()))
    {
        QImage preview = img.scaled(1280, 1024, Qt::KeepAspectRatio);
        bool saved     = preview.save(outUrl.toLocalFile(), "JPEG");
        // save exif information also to preview image for auto rotation
        if (saved)
        {
            KPMetadata metaIn(inUrl.toLocalFile());
            KPMetadata metaOut(outUrl.toLocalFile());
            metaOut.setImageOrientation(metaIn.getImageOrientation());
            metaOut.applyChanges();
        }
        kDebug() << "Preview Image url: " << outUrl << ", saved: " << saved;
        return saved;
    }
    return false;
}


bool PreProcessTask::convertRaw()
{
    const KUrl& inUrl = fileUrl;
    KUrl &outUrl      = preProcessedUrl->preprocessedUrl;

    int        width, height, rgbmax;
    QByteArray imageData;

    rawProcess = new KDcraw;
    bool decoded = rawProcess->decodeRAWImage(inUrl.toLocalFile(), settings, imageData, width, height, rgbmax);
    delete rawProcess;

    if (decoded)
    {
        uchar* sptr  = (uchar*)imageData.data();
        float factor = 65535.0 / rgbmax;
        unsigned short tmp16[3];

        // Set RGB color components.
        for (int i = 0 ; !isAbortedFlag && (i < width * height) ; ++i)
        {
            // Swap Red and Blue and re-ajust color component values
            tmp16[0] = (unsigned short)((sptr[5]*256 + sptr[4]) * factor);      // Blue
            tmp16[1] = (unsigned short)((sptr[3]*256 + sptr[2]) * factor);      // Green
            tmp16[2] = (unsigned short)((sptr[1]*256 + sptr[0]) * factor);      // Red
            memcpy(&sptr[0], &tmp16[0], 6);
            sptr += 6;
        }

        if (isAbortedFlag)
        {
            errString = i18n("Operation canceled.");
            return false;
        }

        KPMetadata metaIn, metaOut;
        metaIn.load(inUrl.toLocalFile());
        KPMetadata::MetaDataMap m = metaIn.getExifTagsDataList(QStringList("Photo"), true);
        KPMetadata::MetaDataMap::iterator it;
        for (it = m.begin(); it != m.end(); ++it)
        {
            metaIn.removeExifTag(it.key().toAscii().data(), false);
        }
        metaOut.setData(metaIn.data());
        metaOut.setImageProgramId(QString("Kipi-plugins"), QString(kipiplugins_version));
        metaOut.setImageDimensions(QSize(width, height));
        metaOut.setExifTagString("Exif.Image.DocumentName", inUrl.fileName());
        metaOut.setXmpTagString("Xmp.tiff.Make",  metaOut.getExifTagString("Exif.Image.Make"));
        metaOut.setXmpTagString("Xmp.tiff.Model", metaOut.getExifTagString("Exif.Image.Model"));
        metaOut.setImageOrientation(KPMetadata::ORIENTATION_NORMAL);

        QByteArray prof = KPWriteImage::getICCProfilFromFile(settings.outputColorSpace);

        KPWriteImage wImageIface;
        wImageIface.setCancel(&isAbortedFlag);
        wImageIface.setImageData(imageData, width, height, true, false, prof, metaOut);
        QFileInfo fi(inUrl.toLocalFile());
        outUrl.setFileName(fi.completeBaseName().replace('.', '_') + QString(".tif"));

        if (!wImageIface.write2TIFF(outUrl.toLocalFile()))
        {
            errString = i18n("Tiff image creation failed.");
            return false;
        }
        else
        {
            metaOut.save(outUrl.toLocalFile());
        }
    }
    else
    {
        errString = i18n("Raw file conversion failed.");
        return false;
    }

    kDebug() << "Convert RAW output url: " << outUrl;

    return true;
}
QString PreProcessTask::getProcessError(KProcess* const proc) const
{
    if (!proc) return QString();

    QString std = proc->readAll();
    return (i18n("Cannot run %1:\n\n %2", proc->program()[0], std));
}

}  // namespace KIPIExpoBlendingPlugin