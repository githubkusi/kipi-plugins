/* ============================================================
 * File  : imagesgallery.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-09-05
 * Description : Images gallery HTML export
 *
 * Adapted and improved for DigikamPlugins from the konqueror plugin
 * 'kdeaddons/konq-plugins/kimgalleryplugin/' by Gilles Caulier.
 *
 * Copyright 2001, 2003 by Lukas Tinkl <lukas at kde.org> and
 * Andreas Schlapbach <schlpbch at iam.unibe.ch> for orginal source
 * of 'kimgalleryplugin' from KDE CVS
 *
 * Copyright 2003-2004 by Gilles Caulier <caulier dot gilles at free.fr> for
 * DigikamPlugins port.
 *
 * Copyright 2003-2004 by Gregory Kokanosky <gregory dot kokanosky at free.fr>
 * for images navigation mode patchs.
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

extern "C"
{
#include <unistd.h>
}

// Include files for Qt

#include <qtextstream.h>
#include <qfile.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qimage.h>
#include <qprogressdialog.h>
#include <qtextcodec.h>
#include <qstringlist.h>

// Include files for KDE

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kinstance.h>
#include <kconfig.h>
#include <kaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kimageio.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>

// KIPI includes

#include <libkipi/imagecollection.h>

// Local includes

#include "imgallerydialog.h"
#include "imagesgallery.h"
#include "resizeimage.h"
#include "listimageserrordialog.h"
#include "imagesgallery.moc"


ImagesGallery::ImagesGallery( KIPI::Interface* interface )
{
    KImageIO::registerFormats();
    Activate( interface );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

ImagesGallery::~ImagesGallery()
{
    delete m_configDlg;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::writeSettings(void)
{
  m_config = new KConfig("kipirc");
  m_config->setGroup("ImagesGallery Settings");

  // HTML Look dialogbox setup tab

  m_config->writeEntry("MainPageTitle", m_configDlg->getMainTitle());
  m_config->writeEntry("ImagesPerRow", m_configDlg->getImagesPerRow());
  m_config->writeEntry("PrintImageName", m_configDlg->printImageName());
  m_config->writeEntry("PrintImageSize", m_configDlg->printImageSize());
  m_config->writeEntry("PrintFileSize", m_configDlg->printImageProperty());
  m_config->writeEntry("PrintPageCreationDate", m_configDlg->printPageCreationDate());
  m_config->writeEntry("CreatePageForPhotos", m_configDlg->getCreatePageForPhotos());
  m_config->writeEntry("OpenInWebBrowser", m_configDlg->OpenGalleryInWebBrowser());
  m_config->writeEntry("WebBrowserName", m_configDlg->getWebBrowserName());
  m_config->writeEntry("FontName", m_configDlg->getFontName());
  m_config->writeEntry("FontSize", m_configDlg->getFontSize());
  m_config->writeEntry("FontColor", m_configDlg->getForegroundColor());
  m_config->writeEntry("BackgroundColor", m_configDlg->getBackgroundColor());
  m_config->writeEntry("BordersImagesSize", m_configDlg->getBordersImagesSize());
  m_config->writeEntry("BordersImagesColor", m_configDlg->getBordersImagesColor());

  // ALBUM dialogbox setup tab

  m_config->writeEntry("GalleryPath", m_configDlg->getImageName());
  m_config->writeEntry("NotUseOriginalImageSize", m_configDlg->useNotOriginalImageSize());
  m_config->writeEntry("ImagesResize", m_configDlg->getImagesResize());
  m_config->writeEntry("TargetImagesCompressionSet", m_configDlg->useSpecificTargetimageCompression());
  m_config->writeEntry("TargetImagesCompression", m_configDlg->getTargetImagesCompression());
  m_config->writeEntry("TargetImagesFormat", m_configDlg->getTargetImagesFormat());
  m_config->writeEntry("TargetImagesColorDepthSet", m_configDlg->colorDepthSetTargetImages());
  m_config->writeEntry("TargetImagesColorDepthValue", m_configDlg->getColorDepthTargetImages());
  m_config->writeEntry("UseCommentFile", m_configDlg->useCommentFile());
  m_config->writeEntry("UseCommentsAlbum", m_configDlg->useCommentsAlbum());
  m_config->writeEntry("UseCollectionAlbum", m_configDlg->useCollectionAlbum());
  m_config->writeEntry("UseDateAlbum", m_configDlg->useDateAlbum());
  m_config->writeEntry("PrintImageNb", m_configDlg->useNbImagesAlbum());

  // THUMNAILS dialogbox setup tab

  m_config->writeEntry("ThumbnailsSize", m_configDlg->getThumbnailsSize());
  m_config->writeEntry("ThumbnailsCompressionSet", m_configDlg->useSpecificThumbsCompression());
  m_config->writeEntry("ThumbnailsCompression", m_configDlg->getThumbsCompression());
  m_config->writeEntry("ThumbnailsFormat", m_configDlg->getImageFormat());
  m_config->writeEntry("ThumbnailsColorDepthSet", m_configDlg->colorDepthSetThumbnails());
  m_config->writeEntry("ThumbnailsColorDepthValue", m_configDlg->getColorDepthThumbnails());

  m_config->sync();
  delete m_config;
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::readSettings(void)
{
  QColor* ColorFont;
  QColor* ColorBackground;
  QColor* ColorBordersImages;

  m_config = new KConfig("kipirc");
  m_config->setGroup("ImagesGallery Settings");

  // HTML Look dialogbox setup tab

  m_configDlg->setMainTitle( m_config->readEntry("MainPageTitle", i18n("KIPI Albums Images Galleries")) );
  m_configDlg->setImagesPerRow( m_config->readEntry("ImagesPerRow", "4").toInt() );

  if (m_config->readEntry("PrintImageName", "true") == "true")
     m_configDlg->setPrintImageName( true );
  else
     m_configDlg->setPrintImageName( false );

  if (m_config->readEntry("PrintImageSize", "true") == "true")
     m_configDlg->setPrintImageSize( true );
  else
     m_configDlg->setPrintImageSize( false );

  if (m_config->readEntry("PrintFileSize", "true") == "true")
     m_configDlg->setPrintImageProperty( true );
  else
     m_configDlg->setPrintImageProperty( false );

  if (m_config->readEntry("PrintPageCreationDate", "true") == "true")
     m_configDlg->setPrintPageCreationDate( true );
  else
     m_configDlg->setPrintPageCreationDate( false );

  if(m_config->readEntry("CreatePageForPhotos", "true") == "true")
     m_configDlg->setCreatePageForPhotos( true );
  else
     m_configDlg->setCreatePageForPhotos( false );

  if (m_config->readEntry("OpenInWebBrowser", "true") == "true")
     m_configDlg->setOpenGalleryInWebBrowser( true );
  else
     m_configDlg->setOpenGalleryInWebBrowser( false );

  m_configDlg->setWebBrowserName( m_config->readEntry("WebBrowserName", "Konqueror") );

  m_configDlg->setFontName( m_config->readEntry("FontName", "Helvetica") );
  m_configDlg->setFontSize( m_config->readEntry("FontSize", "14").toInt() );
  ColorFont = new QColor( 208, 255, 208 );
  m_configDlg->setForegroundColor( m_config->readColorEntry("FontColor", ColorFont));
  ColorBackground = new QColor( 51, 51, 51 );
  m_configDlg->setBackgroundColor( m_config->readColorEntry("BackgroundColor", ColorBackground));
  m_configDlg->setBordersImagesSize( m_config->readEntry("BordersImagesSize", "1").toInt() );
  ColorBordersImages = new QColor( 208, 255, 208 );
  m_configDlg->setBordersImagesColor( m_config->readColorEntry("BordersImagesColor", ColorBordersImages));

  delete ColorFont;
  delete ColorBackground;
  delete ColorBordersImages;

  // ALBUM dialogbox setup tab

  m_configDlg->setImageName( m_config->readEntry("GalleryPath", KGlobalSettings::documentPath()) );

  if (m_config->readEntry("NotUseOriginalImageSize", "true") == "true")
     m_configDlg->setNotUseOriginalImageSize( true );
  else
     m_configDlg->setNotUseOriginalImageSize( false );

  m_configDlg->setImagesResizeFormat( m_config->readEntry("ImagesResize", "640").toInt() );

  if (m_config->readEntry("TargetImagesCompressionSet", "false") == "true")
     m_configDlg->setUseSpecificTargetimageCompression( true );
  else
     m_configDlg->setUseSpecificTargetimageCompression( false );

  m_configDlg->setTargetImagesCompression( m_config->readEntry("TargetImagesCompression", "75").toInt() );

  m_configDlg->setTargetImagesFormat( m_config->readEntry("TargetImagesFormat", "JPEG") );

  if (m_config->readEntry("TargetImagesColorDepthSet", "false") == "true")
     m_configDlg->setColorDepthSetTargetImages( true );
  else
     m_configDlg->setColorDepthSetTargetImages( false );

  m_configDlg->setColorDepthTargetImages( m_config->readEntry("TargetImagesColorDepthValue", "32") );

  if (m_config->readEntry("UseCommentFile", "true") == "true")
     m_configDlg->setUseCommentFile( true );
  else
     m_configDlg->setUseCommentFile( false );

  if (m_config->readEntry("UseCommentsAlbum", "true") == "true")
     m_configDlg->setUseCommentsAlbum( true );
  else
     m_configDlg->setUseCommentsAlbum( false );

  if (m_config->readEntry("UseCollectionAlbum", "true") == "true")
     m_configDlg->setUseCollectionAlbum( true );
  else
     m_configDlg->setUseCollectionAlbum( false );

  if (m_config->readEntry("UseDateAlbum", "true") == "true")
     m_configDlg->setUseDateAlbum( true );
  else
     m_configDlg->setUseDateAlbum( false );

  if (m_config->readEntry("PrintImageNb", "true") == "true")
     m_configDlg->setUseNbImagesAlbum( true );
  else
     m_configDlg->setUseNbImagesAlbum( false );

  // THUMNAILS dialogbox setup tab

  m_configDlg->setThumbnailsSize( m_config->readEntry("ThumbnailsSize", "140").toInt() );

  if (m_config->readEntry("ThumbnailsCompressionSet", "false") == "true")
     m_configDlg->setUseSpecificThumbsCompression( true );
  else
     m_configDlg->setUseSpecificThumbsCompression( false );

  m_configDlg->setThumbsCompression( m_config->readEntry("ThumbnailsCompression", "75").toInt() );

  m_configDlg->setImageFormat( m_config->readEntry("ThumbnailsFormat", "JPEG") );

  if (m_config->readEntry("ThumbnailsColorDepthSet", "false") == "true")
     m_configDlg->setColorDepthSetThumbnails( true );
  else
     m_configDlg->setColorDepthSetThumbnails( false );

  m_configDlg->setColorDepthThumbnails( m_config->readEntry("ThumbnailsColorDepthValue", "32") );

  // Read File Filter settings in kipirc file.

  m_config->setGroup("Album Settings");
  QString Temp = m_config->readEntry("File Filter", "*.jpg *.jpeg *.tif *.tiff *.gif *.png *.bmp");
  m_imagesFileFilter = Temp.lower() + " " + Temp.upper();

  delete m_config;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::Activate( KIPI::Interface* interface )
{
    QString Path;
    m_progressDlg = 0L;

    m_configDlg = new KIGPDialog( interface, 0);
    readSettings();

    if ( m_configDlg->exec() == QDialog::Accepted )
        {
        KURL SubUrl, MainUrl;
        writeSettings();
        m_recurseSubDirectories = false;
        m_LevelRecursion = 1;
        m_resizeImagesWithError.clear();
        m_StreamMainPageAlbumPreview = "";
        m_imagesPerRow = m_configDlg->getImagesPerRow();
        QStringList ListAlbums(m_configDlg->getAlbumsSelection());

        // Create the main target folder.

        QDir TargetDir;
        QString MainTPath= m_configDlg->getImageName() + "/KIPIHTMLExport";

        if (TargetDir.exists (MainTPath) == TRUE)
           {
           if (KMessageBox::warningYesNo(0,
               i18n("The target directory\n'%1'\nalready exist. Do you want overwrite it? (all data "
                    "in this directory will be lost!)").arg(MainTPath)) == KMessageBox::Yes)
              {
              if (!KIO::NetAccess::del(KURL(MainTPath)))
                 {
                 KMessageBox::error(0, i18n("Cannot remove folder '%1'!").arg(MainTPath));
                 return;
                 }
              }
           else
              return;
           }

        if (TargetDir.mkdir( MainTPath ) == false)
           {
           KMessageBox::sorry(0, i18n("Couldn't create directory '%1'").arg(MainTPath));
           return;
           }

        // Build all Albums HTML export.

        if ( ListAlbums.count() > 1 )
           {
           KGlobal::dirs()->addResourceType("kipi_data", KGlobal::dirs()->kde_default("data") + "kipi");
           QString dir = KGlobal::dirs()->findResourceDir("kipi_data", "gohome.png");
           dir = dir + "gohome.png";

           KURL srcURL(dir);
           KURL destURL(m_configDlg->getImageName() + "/KIPIHTMLExport/gohome.png");
           KIO::NetAccess::copy(srcURL, destURL);
           }

        for ( QStringList::Iterator it = ListAlbums.begin(); it != ListAlbums.end(); ++it )
            {
            m_album           = ->findAlbum( *it );
            m_album->openDB();
            Path              = m_album->getPath();
            m_AlbumTitle      = m_album->getTitle();
            m_AlbumComments   = m_album->getComments();
            m_AlbumCollection = m_album->getCollection();
            m_AlbumDate       = m_album->getDate().toString ( Qt::LocalDate ) ;
            m_album->closeDB();
            Path = Path + "/";

            SubUrl = m_configDlg->getImageName() + "/DigikamHTMLExport/" + m_AlbumTitle + "/" + "index.html";

            if ( !SubUrl.isEmpty() && SubUrl.isValid())
               {
               // Create the target sub folder for the current album.

               QString SubTPath= m_configDlg->getImageName() + "/DigikamHTMLExport/" + m_AlbumTitle;

               if (TargetDir.mkdir( SubTPath ) == false)
                   {
                   KMessageBox::sorry(0, i18n("Couldn't create directory '%1'").arg(SubTPath));
                   return;
                   }

               m_progressDlg = new QProgressDialog(0, "progressDlg", true );

               connect(m_progressDlg, SIGNAL( cancelled() ),
                       this, SLOT( slotCancelled() ) );

               m_progressDlg->setCaption( i18n("Album \"%1\"").arg(m_AlbumTitle) );
               m_progressDlg->setCancelButtonText(i18n("&Cancel"));
               m_cancelled = false;
               m_progressDlg->show();
               kapp->processEvents();
               m_useCommentFile = m_configDlg->useCommentFile();

               if ( !createHtml( SubUrl, Path, m_LevelRecursion > 0 ? m_LevelRecursion + 1 : 0 ,
                                 m_configDlg->getImageFormat(), m_configDlg->getTargetImagesFormat()) )
                  {
                  delete m_progressDlg;

                  if (!KIO::NetAccess::del(KURL(MainTPath)))
                     {
                     KMessageBox::error(0, i18n("Cannot remove folder %1 !").arg(MainTPath));
                     return;
                     }

                  return;
                  }
               }

            delete m_progressDlg;
          }

        // Lauch an error dialog if some resize images operations failed.

        if ( m_resizeImagesWithError.isEmpty() == false )
           {
           listImagesErrorDialog *ErrorImagesDialog = new listImagesErrorDialog(0,
                                                  i18n("Error during resize images process"),
                                                  i18n("Cannot resized or thumnailized this images files :"),
                                                  m_resizeImagesWithError);
           ErrorImagesDialog->exec();
           }

        // Create the main HTML page if many Albums selected.

        if ( ListAlbums.count () > 1 )
           {
           MainUrl = m_configDlg->getImageName() + "/DigikamHTMLExport/" + "index.htm";
           QFile MainPageFile( MainUrl.path() );

           if ( MainPageFile.open(IO_WriteOnly) )
              {
              QTextStream stream(&MainPageFile);
              stream.setEncoding(QTextStream::UnicodeUTF8);
              createHead(stream);
              createBodyMainPage(stream, MainUrl);
              MainPageFile.close();

              if (m_configDlg->OpenGalleryInWebBrowser() == true)
                 invokeWebBrowser(MainUrl.url());
              }
           else
              {
              KMessageBox::sorry(0,i18n("Couldn't open file '%1'").arg(MainUrl.path(+1)));
              return;
              }
           }
        else
           {
           if (m_configDlg->OpenGalleryInWebBrowser() == true)
              invokeWebBrowser(SubUrl.url());
           }
        }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImagesGallery::createDirectory(QDir thumb_dir, QString imgGalleryDir, QString dirName)
{
    if (!thumb_dir.exists())
        {
        thumb_dir.setPath( imgGalleryDir );

        if (!(thumb_dir.mkdir(dirName, false)))
            {
            KMessageBox::sorry(0, i18n("Couldn't create directory '%1' in '%2'")
                                  .arg(dirName).arg(imgGalleryDir));
            return false;
            }
        else
            {
            thumb_dir.setPath( imgGalleryDir + "/" + dirName + "/" );
            return true;
            }
        }
    else
        return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::createHead(QTextStream& stream)
{
    QString chsetName = QTextCodec::codecForLocale()->mimeName();

    stream << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">"
           << endl;
    stream << "<html>" << endl;
    stream << "<head>" << endl;
    stream << "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">" << endl;
    stream << "<meta name=\"Generator\" content=\"Albums Images gallery generated by Digikam "
              "[http://digikam.sourceforge.net]\">"  << endl;
    stream << "<meta name=\"date\" content=\"" + KGlobal::locale()->formatDate(QDate::currentDate()) + "\">"
           << endl;
    stream << "<title>" << m_configDlg->getMainTitle() << "</title>" << endl;
    this->createCSSSection(stream);
    stream << "</head>" << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::createCSSSection(QTextStream& stream)
{
    QString backgroundColor = m_configDlg->getBackgroundColor().name();
    QString foregroundColor = m_configDlg->getForegroundColor().name();
    QString bordersImagesColor = m_configDlg->getBordersImagesColor().name();

    // Adding a touch of style

    stream << "<style type='text/css'>\n";
    stream << "BODY {color: " << foregroundColor << "; background: " << backgroundColor << ";" << endl;
    stream << "          font-family: " << m_configDlg->getFontName() << ", sans-serif;" << endl;
    stream << "          font-size: " << m_configDlg->getFontSize() << "pt; margin: 4%; }" << endl;
    stream << "H1       {color: " << foregroundColor << ";}" << endl;
    stream << "TABLE    {text-align: center; margin-left: auto; margin-right: auto;}" << endl;
    stream << "TD       { color: " << foregroundColor << "; padding: 1em}" << endl;
    stream << "IMG.photo      { border: " << m_configDlg->getBordersImagesSize() << "px solid "
           << bordersImagesColor << "; }" << endl;
    stream << "</style>" << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

QString ImagesGallery::extension(const QString& imageFormat)
{
    if (imageFormat == "PNG")
        return ".png";

    if (imageFormat == "JPEG")
        return ".jpg";

    Q_ASSERT(false);
    return "";
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::createBody(QTextStream& stream, const QString& sourceDirName,
                               const QStringList& subDirList, const QDir& imageDir,
                               const KURL& url, const QString& imageFormat,
                               const QString& TargetimagesFormat)
{
    int numOfImages = imageDir.count();
    qDebug("Num of images in %s : %i", imageDir.path().ascii(), numOfImages);
    const QString imgGalleryDir = url.directory();
    const QString today(KGlobal::locale()->formatDate(QDate::currentDate()));

    stream << "<body>"<<endl;

    // Display gohome icon

    if (QFile::exists(imgGalleryDir + QString::fromLatin1("/../gohome.png")))
       {
       stream << "<p><a href=\"../index.htm\"><img src=\"../gohome.png\" border=\"0\"  title=\""
              << i18n("Albums list") << "\" alt=\"" << i18n("Albums list") << "\"></a></p>"
              << endl;
       }

    stream << "<h1>" << i18n("Images Gallery for Album ") << "&quot;" << m_AlbumTitle << "&quot;"
           << "</h1>" << endl;

    if (m_configDlg->useCommentsAlbum() == true ||
        m_configDlg->useCollectionAlbum() == true ||
        m_configDlg->useDateAlbum() == true ||
        m_configDlg->useNbImagesAlbum() == true)
       {
       stream << "<table width=\"100%\" border=1 cellpadding=0 cellspacing=0 "
                 "style=\"page-break-before: always\">\n" << endl;
       stream << "<col width=\"20%\"><col width=\"80%\">"<<endl;
       stream << "<tr valign=top><td align=left>\n" << endl;

       if (m_configDlg->useCommentsAlbum() && !m_AlbumComments.isEmpty())
           stream << i18n("<i>Comment:</i>") << "<br>\n" << endl;

       if (m_configDlg->useCollectionAlbum() && !m_AlbumCollection.isEmpty())
           stream << i18n("<i>Collection:</i>") << "<br>\n" << endl;

       if (m_configDlg->useDateAlbum() == true)
           stream << i18n("<i>Date:</i>") << "<br>\n" << endl;

       if (m_configDlg->useNbImagesAlbum() == true)
           stream << i18n("<i>Images:</i>") << "\n" << endl;

       stream << "</td><td align=left>\n" << endl;

       if (m_configDlg->useCommentsAlbum() && !m_AlbumComments.isEmpty())
           {
           stream << EscapeSgmlText(QTextCodec::codecForLocale(), m_AlbumComments, true, true)
                  << "<br>\n" << endl;
           }

       if (m_configDlg->useCollectionAlbum() && !m_AlbumCollection.isEmpty())
           stream << m_AlbumCollection << "<br>\n" << endl;

       if (m_configDlg->useDateAlbum() == true)
           stream << m_AlbumDate << "<br>\n" << endl;

       if (m_configDlg->useNbImagesAlbum() == true)
           stream << numOfImages << "\n" << endl;

       stream << "</td></tr></table>\n" << endl;
       }
    else
       stream << "<hr>\n" << endl;

    if (m_recurseSubDirectories && subDirList.count() > 2)      // This Option is disable actually
        {                                                       // (m_recurseSubDirectories always = 1).

        // subDirList.count() is always >= 2 because of the "." and ".." directories

        QString Temp = i18n("<i>Subdirectories:</i>");
        stream << Temp << "<br>" << endl;

        for (QStringList::ConstIterator it = subDirList.begin(); it != subDirList.end(); it++)
            {
            if (*it == "." || *it == "..")
                continue;                            // Disregard the "." and ".." directories

            stream << "<a href=\"" << *it << "/" << url.fileName() << "\">" << *it << "</a><br>" << endl;
            }

        stream << "<hr>" << endl;
        }

    stream << "<table>" << endl;

    // Table with images

    int imgIndex;
    QFileInfo imginfo;
    QPixmap imgProp;

    for (imgIndex = 0; !m_cancelled && (imgIndex < numOfImages);)
        {
        stream << "<tr>" << endl;

        for (int col = 0 ; !m_cancelled && (col < m_imagesPerRow) && (imgIndex < numOfImages) ; ++col)
            {
            const QString imgName = imageDir[imgIndex];

            const QString targetImgName = imgName + extension(TargetimagesFormat);

            QDir targetImagesDir( imgGalleryDir + QString::fromLatin1("/images/"));

            qDebug("Creating thumbnail for %s", imgName.ascii());

            if (createThumb(imgName, sourceDirName, imgGalleryDir, imageFormat, TargetimagesFormat))
                {
                // user requested the creation of html pages for each photo

                if ( m_configDlg->getCreatePageForPhotos() )
                   stream << "<td align='center'>\n<a href=\"pages/" << targetImgName << ".html\">";
                else
                   stream << "<td align='center'>\n<a href=\"images/" << targetImgName << "\">";

                const QString imgNameFormat = imgName;

                const QString imgPath("thumbs/" + imgNameFormat + extension(imageFormat));
                stream << "<img class=\"photo\" src=\"" << imgPath << "\" width=\"" << m_imgWidth << "\" ";
                stream << "height=\"" << m_imgHeight << "\" alt=\"" << imgPath;

                QString sep = "\" title=\"";

                if (m_configDlg->printImageName())
                    {
                    stream << sep << imgName;
                    sep = ", ";
                    }

                if (m_configDlg->printImageProperty())
                    {
                    imgProp.load( targetImagesDir.absFilePath(targetImgName, true) );
                    stream << sep << imgProp.width() << "&nbsp;x&nbsp;" << imgProp.height();
                    sep = ", ";
                    }

                if (m_configDlg->printImageSize())
                    {
                    imginfo.setFile( targetImagesDir, targetImgName );
                    stream << sep << (imginfo.size() / 1024) << "&nbsp;" <<  i18n("KB");
                    sep = ", ";
                    }

                if ( m_useCommentFile )
                   {
                   QString imgComment = (*m_commentMap)[imgName];

                   if ( !imgComment.isEmpty() )
                      {
                      stream << sep
                             << EscapeSgmlText(QTextCodec::codecForLocale(), imgComment, true, true);
                      }
                   }

                stream << "\">" << endl;

                // For each first image of current Album we add a preview in main HTML page.

                if ( imgIndex == 0)
                   {
                   QString Temp, Temp2;
                   Temp2 = "<a href=\"" + m_AlbumTitle + "/" + "index.html" + "\">";
                   m_StreamMainPageAlbumPreview.append ( Temp2 );
                   Temp2 = "<img class=\"photo\" src=\"" + m_AlbumTitle + "/" + imgPath + "\" width=\""
                           + Temp.setNum(m_imgWidth) + "\" ";
                   m_StreamMainPageAlbumPreview.append ( Temp2 );
                   Temp2 = "height=\"" + Temp.setNum(m_imgHeight) + "\" alt=\"" + imgPath + "\" ";
                   m_StreamMainPageAlbumPreview.append ( Temp2 );
                   Temp2 = "title=\"" + m_AlbumTitle + " [ " + Temp.setNum(numOfImages) + i18n(" images")
                           + " ]\"></a>\n";
                   m_StreamMainPageAlbumPreview.append ( Temp2 );
                   Temp2 = "<a href=\"" + m_AlbumTitle + "/" + "index.html" + "\">" + m_AlbumTitle + "</a>"
                           + " [ " + Temp.setNum(numOfImages) + i18n(" images") + " ]" + "<br>\n";
                   m_StreamMainPageAlbumPreview.append ( Temp2 );
                   }

                m_progressDlg->setLabelText(
                               i18n("Creating target image and thumbnail for \n'%1'\nPlease wait!")
                               .arg(imgName) );
                kapp->processEvents();
                }
            else
                {
                qDebug("Creating thumbnail for %s failed !", imgName.ascii());
                m_progressDlg->setLabelText(
                               i18n("Creating target image and thumbnail for\n%1\nfailed!")
                               .arg(imgName) );
                kapp->processEvents();
                }

            stream << "</a>" << endl;

            if (m_configDlg->printImageName())
                {
                stream << "<div>" << imgName << "</div>" << endl;
                }

            if (m_configDlg->printImageProperty())
                {
                imgProp.load( targetImagesDir.absFilePath(targetImgName, true) );
                stream << "<div>" << imgProp.width() << " x " << imgProp.height() << "</div>" << endl;
                }

            if (m_configDlg->printImageSize())
                {
                imginfo.setFile( targetImagesDir, targetImgName );
                stream << "<div>(" << (imginfo.size() / 1024) << " " <<  i18n("KB") << ") "
                       << "</div>" << endl;
                }


            stream << "</td>" << endl;
            ++imgIndex;
            m_progressDlg->setTotalSteps( numOfImages );
            m_progressDlg->setProgress( imgIndex );
            kapp->processEvents();
            }

        stream << "</tr>" << endl;
        }

    // Close the HTML and page creation info if necessary.

    stream << "</table>\n<hr>\n" << endl;

    // create HTML pages if requested.

    if( m_configDlg->getCreatePageForPhotos() )
      {
      KGlobal::dirs()->addResourceType("digikam_data", KGlobal::dirs()->kde_default("data") + "digikam");
      QString dir = KGlobal::dirs()->findResourceDir("digikam_data", "up.png");
      dir = dir + "up.png";

      KURL srcURL(dir);
      KURL destURL(imgGalleryDir + QString::fromLatin1("/up.png"));
      KIO::NetAccess::copy(srcURL, destURL);

      for (imgIndex = 0 ; !m_cancelled && (imgIndex < numOfImages) ; )
        {
        const QString imgName = imageDir[imgIndex];

        const QString targetImgName = imgName + extension(TargetimagesFormat);

        QString previousImgName = "";

        if ( imgIndex != 0 )
           {
           previousImgName = imageDir[imgIndex - 1];
           previousImgName = previousImgName + extension(TargetimagesFormat);
           }

        QString nextImgName = "" ;

        if ( imgIndex != numOfImages -1)
           {
           nextImgName = imageDir[imgIndex + 1];
           nextImgName = nextImgName + extension(TargetimagesFormat);
           }

        QString imgComment = "";

        if ( m_useCommentFile )
           imgComment = (*m_commentMap)[imgName];

        if ( createPage(imgGalleryDir,  targetImgName , previousImgName , nextImgName , imgComment) )
           {
           m_progressDlg->setLabelText( i18n("Creating html page for \n'%1'\nPlease wait!").arg(imgName) );
           kapp->processEvents();
           }
        else
          {
          qDebug("Creating html page for %s failed !", imgName.ascii());
          m_progressDlg->setLabelText( i18n("Creating html page for\n%1\nfailed!").arg(imgName) );
          kapp->processEvents();
          }

      ++imgIndex;
      m_progressDlg->setTotalSteps( numOfImages );
      m_progressDlg->setProgress( imgIndex );
      kapp->processEvents();
      }
    }

    if (m_configDlg->printPageCreationDate())
        {
        QString Temp;
        KGlobal::dirs()->addResourceType("digikam_data", KGlobal::dirs()->kde_default("data") + "digikam");
        QString dir = KGlobal::dirs()->findResourceDir("digikam_data", "valid-html401.png");
        dir = dir + "valid-html401.png";

        KURL srcURL(dir);
        KURL destURL(imgGalleryDir + QString::fromLatin1("/thumbs/valid-html401.png"));
        KIO::NetAccess::copy(srcURL, destURL);

        stream << "<p>"  << endl;
        Temp = i18n("Valid HTML 4.01!");
        stream << "<img src=\"thumbs/valid-html401.png\" alt=\"" << Temp
               << "\" height=\"31\" width=\"88\"  title=\"" << Temp <<  "\" />" << endl;
        Temp = i18n("Images gallery created with "
                    "<a href=\"http://digikam.sourceforge.net\">Digikam</a> on %1").arg(today);
        stream << Temp << endl;
        stream << "</p>" << endl;
        }

    stream << "</body>\n</html>\n" << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::createBodyMainPage(QTextStream& stream, KURL& url)
{
    QString Temp;
    const QString today(KGlobal::locale()->formatDate(QDate::currentDate()));

    Temp = m_configDlg->getMainTitle();
    stream << "<body>\n<h1>" << Temp << "</h1><p>\n" << endl;

    Temp = i18n("<i>Albums list:</i>");
    stream << Temp << "<br>" << endl;
    stream << "<hr>" << endl;

    stream << "<p> " << m_StreamMainPageAlbumPreview << "</p>" << endl;

    stream << "<hr>" << endl;

    if (m_configDlg->printPageCreationDate())
        {
        QString Temp;
        KGlobal::dirs()->addResourceType("digikam_data", KGlobal::dirs()->kde_default("data") + "digikam");
        QString dir = KGlobal::dirs()->findResourceDir("digikam_data", "valid-html401.png");
        dir = dir + "valid-html401.png";

        KURL srcURL(dir);
        KURL destURL(url.directory() + QString::fromLatin1("/valid-html401.png"));
        KIO::NetAccess::copy(srcURL, destURL);

        stream << "<p>"  << endl;
        Temp = i18n("Valid HTML 4.01!");
        stream << "<img src=\"valid-html401.png\" alt=\"" << Temp
               << "\" height=\"31\" width=\"88\" title=\"" << Temp <<  "\" />" << endl;
        Temp = i18n("Images gallery created with "
                    "<a href=\"http://digikam.sourceforge.net\">Digikam</a> on %1").arg(today);
        stream << Temp << endl;
        stream << "</p>" << endl;
        }

    stream << "</body>\n</html>\n" << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImagesGallery::createHtml(const KURL& url, const QString& sourceDirName, int recursionLevel,
                               const QString& imageFormat, const QString& TargetimagesFormat)
{
    if (m_cancelled) return false;

    QStringList subDirList;

    if (m_recurseSubDirectories && (recursionLevel >= 0))  // RecursionLevel == 0 means endless
        {                                                  // Recursive subdirectories is not
                                                           // used in Digikam actually.
        QDir toplevel_dir = QDir( sourceDirName );
        toplevel_dir.setFilter( QDir::Dirs | QDir::Readable | QDir::Writable );
        subDirList = toplevel_dir.entryList();

        for (QStringList::ConstIterator it = subDirList.begin(); it != subDirList.end() && !m_cancelled; it++)
            {
            const QString currentDir = *it;
            kapp->processEvents();

            if (currentDir == "." || currentDir == "..")   // Disregard the "." and ".." directories
                continue;

            QDir subDir = QDir( url.directory() + "/" + currentDir );

            if (!subDir.exists())
                {
                subDir.setPath( url.directory() );

                if (!(subDir.mkdir(currentDir, false)))
                    {
                    KMessageBox::sorry(0, i18n("Couldn't create directory '%1' in '%2'")
                                       .arg(currentDir).arg(url.directory()));
                    continue;
                    }
                else
                    subDir.setPath( url.directory() + "/" + currentDir );
                }

            if (!createHtml( KURL( subDir.path() + "/" + url.fileName() ), sourceDirName + "/" + currentDir,
                            recursionLevel > 1 ? recursionLevel - 1 : 0, imageFormat, TargetimagesFormat))
               return false;
            }
       }

    if ( m_useCommentFile )
       loadComments();

    kdDebug() << "sourceDirName: " << sourceDirName << endl;

    // Sort the images files formats running with thumbnails construction.

    QDir imageDir( sourceDirName, m_imagesFileFilter.latin1(),
                   QDir::Name|QDir::IgnoreCase, QDir::Files|QDir::Readable);

    const QString imgGalleryDir = url.directory();
    kdDebug() << "imgGalleryDir: " << imgGalleryDir << endl;

    // Create the "thumbs" subdirectory

    QDir thumb_dir( imgGalleryDir + QString::fromLatin1("/thumbs/"));

    if (createDirectory(thumb_dir, imgGalleryDir, "thumbs") == false)
        return false;

    // Create the "images" subdirectory

    QDir images_dir( imgGalleryDir + QString::fromLatin1("/images/"));

    if (createDirectory(images_dir, imgGalleryDir, "images") == false)
        return false;

    QDir pages_dir( imgGalleryDir + QString::fromLatin1("/pages/"));

    if (m_configDlg->getCreatePageForPhotos())
       {
       kdDebug() << "Create photos :" << m_configDlg->getCreatePageForPhotos() << endl;

       if (createDirectory(pages_dir, imgGalleryDir, "pages") == false)
         return false;
       }

    // Create HTML page.

    QFile file( url.path() );
    kdDebug() << "url.path(): " << url.path() << ", thumb_dir: "<< thumb_dir.path()
              << ", imageDir: "<< imageDir.path() << ", pagesDir: "<< pages_dir.path()<<endl;

    if ( imageDir.exists() && file.open(IO_WriteOnly) )
        {
        QTextStream stream(&file);
        stream.setEncoding(QTextStream::UnicodeUTF8);
        createHead(stream);
        createBody(stream, sourceDirName, subDirList, imageDir, url, imageFormat, TargetimagesFormat);
        file.close();
        return !m_cancelled;
        }
    else
        {
        KMessageBox::sorry(0,i18n("Couldn't open file '%1'").arg(url.path(+1)));
        return false;
        }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::loadComments(void)
{
    // We considering no default images comments for the current album.

    m_useCommentFile = false;

    QDir currentAlbumDir(m_album->getPath());
    currentAlbumDir.setSorting (QDir::Files|QDir::NoSymLinks);
    QStringList AllAlbumItems = currentAlbumDir.entryList();

    m_commentMap = new CommentMap;
    m_album->openDB();

    for ( QStringList::Iterator it = AllAlbumItems.begin(); it != AllAlbumItems.end(); ++it )
        {
        QString Item(*it);
        kapp->processEvents();

        if ( Item != "" && Item != "." && Item != ".." )
           {
           QString Comment(m_album->getItemComments(*it));

           if (Comment != "")
              {
              // An image comment have been found in the current album !

              m_useCommentFile = true;
              m_commentMap->insert(Item, Comment);
              }
           }
        }

    m_album->closeDB();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImagesGallery::createPage(const QString& imgGalleryDir, const QString& imgName,
                               const QString& previousImgName, const QString& nextImgName,
                               const QString& comment)
{
    const QDir pagesDir(imgGalleryDir + QString::fromLatin1("/pages/"));
    const QDir targetImagesDir(imgGalleryDir + QString::fromLatin1("/images/"));
    const QDir thumbsDir(imgGalleryDir + QString::fromLatin1("/thumbs/"));

    // Html pages filenames

    const QString pageFilename = pagesDir.path() + QString::fromLatin1("/") + imgName
                                 + QString::fromLatin1(".html");
    const QString nextPageFilename =  nextImgName + QString::fromLatin1(".html");
    const QString previousPageFilename =  previousImgName + QString::fromLatin1(".html");

    // Thumbs filenames

    const QString previousThumb = QString::fromLatin1("../thumbs/")
                                  + previousImgName.left( previousImgName.findRev('.', -1) )
                                  + extension(m_configDlg->getImageFormat());

    const QString nextThumb = QString::fromLatin1("../thumbs/")
                              + nextImgName.left( nextImgName.findRev('.', -1) )
                              + extension(m_configDlg->getImageFormat());

    QFile file( pageFilename );

    if ( pagesDir.exists() && file.open(IO_WriteOnly) )
       {
       QTextStream stream(&file);
       stream.setEncoding(QTextStream::UnicodeUTF8);

       QString chsetName = QTextCodec::codecForLocale()->mimeName();
       stream << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 "
                 "Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">" << endl;
       stream << "<html>" << endl;
       stream << "<head>" << endl;
       stream << "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">" << endl;
       stream << "<meta name=\"Generator\" content=\"Albums Images gallery generated by Digikam "
                 "[http://digikam.sourceforge.net]\">"  << endl;
       stream << "<meta name=\"date\" content=\""
                 + KGlobal::locale()->formatDate(QDate::currentDate()) + "\">" << endl;
       stream << "<title>" << m_configDlg->getMainTitle() << " : "<< imgName <<"</title>" << endl;
       this->createCSSSection(stream);
       stream << "</head>" << endl;
       stream<<"<body>"<< endl;;

       stream << "<div align=\"center\">" << endl;

       QPixmap imgProp;

       int prevW = 0;
       int prevH = 0;
       int nextW = 0;
       int nextH = 0;

       if (imgProp.load( targetImagesDir.absFilePath(previousImgName, true) ))
          {
          prevW = imgProp.width();
          prevH = imgProp.height();
          }

       if (imgProp.load( targetImagesDir.absFilePath(nextImgName, true) ))
          {
          nextW = imgProp.width();
          nextH = imgProp.height();
          }

       kdDebug() << previousImgName << ":"<<prevW<<"/"<<prevH << "       "
                 <<  nextImgName << ":"<<nextW<<"/"<<nextH<< endl;

       // Navigation thumbs need to be 64x64 at most

       if ( prevW < prevH )
          {
          prevH = (NAV_THUMB_MAX_SIZE  * prevH) / prevW;
          prevW = NAV_THUMB_MAX_SIZE;
          }
       else if ( prevW == prevH )
          {
          prevH = NAV_THUMB_MAX_SIZE;
          prevW = NAV_THUMB_MAX_SIZE;
          }
       else
          {
          prevW = (NAV_THUMB_MAX_SIZE  * prevW) / prevH;
          prevH = NAV_THUMB_MAX_SIZE;
          }

       if ( nextW < nextH )
          {
          nextH = (NAV_THUMB_MAX_SIZE  * nextH) / nextW;
          nextW = NAV_THUMB_MAX_SIZE;
          }
       else if ( nextW == nextH )
          {
          nextH = NAV_THUMB_MAX_SIZE ;
          nextW = NAV_THUMB_MAX_SIZE;
          }
       else
          {
          nextW = (NAV_THUMB_MAX_SIZE  * nextW) / nextH;
          nextH = NAV_THUMB_MAX_SIZE;
          }

       kdDebug() << previousImgName << ":"<<prevW<<"/"<<prevH << "       "
                 <<  nextImgName << ":"<<nextW<<"/"<<nextH<< endl;

       if (previousImgName != "")
          {
          stream << "<a href=\"" << previousPageFilename << "\"><img class=\"photo\" src=\""
                 << previousThumb << "\" alt=\"" << i18n("Previous") <<  "\" title=\""
                 << i18n("Previous") << "\" height=\""<< prevH << "\" width=\"" << prevW
                 << "\"></a>&nbsp; | &nbsp;" << endl;
          }

       stream << "<a href=\"../index.html\"><img src=\"../up.png\" border=\"0\" title=\""
              << i18n("Album index") << "\" alt=\"" << i18n("Album index") << "\"></a>" << endl;

       if (QFile::exists(imgGalleryDir + QString::fromLatin1("/../gohome.png")))
          {
          stream << "&nbsp; | &nbsp;<a href=\"../../index.htm\"><img src=\"../../gohome.png\" "
                    "border=\"0\" title=\"" << i18n("Albums list") << "\" alt=\""
                 << i18n("Albums list") << "\"></a>" <<endl;
          }

       if (nextImgName != "")
          {
          stream << "&nbsp; | &nbsp;<a href=\"" << nextPageFilename << "\"><img class=\"photo\" src=\""
                 << nextThumb << "\" alt=\"" << i18n("Next") <<"\" title=\"" << i18n("Next")
                 << "\" height=\"" << nextH << "\" width=\"" << nextW << "\"></a>" << endl;
          }

       stream << "<br><hr><br>" << endl;

       // Add comment if it exists

       if ( comment != "" )
          {
          stream << "<div align=\"center\">"
                 << EscapeSgmlText(QTextCodec::codecForLocale(), comment, true, true)
                 << "</div>" << endl;
          }

       stream <<"<br>" << endl;

       stream << "<img class=\"photo\" src=\"../images/" << imgName << "\" alt=\"" << imgName;

       // Add info about image if requested

       QString sep = "\" title=\"";

       QFileInfo imginfo;

       if (m_configDlg->printImageName())
          {
          stream << sep << imgName;
          sep = ", ";
          }

       if (m_configDlg->printImageProperty())
          {
          imgProp.load( targetImagesDir.absFilePath(imgName, true) );
          kdDebug() << targetImagesDir.path() << "/" << imgName << endl;
          stream << sep << imgProp.width() << "&nbsp;x&nbsp;" << imgProp.height();
          sep = ", ";
          }

       if (m_configDlg->printImageSize())
          {
          imginfo.setFile( targetImagesDir, imgName );
          stream << sep << (imginfo.size() / 1024) << "&nbsp;" <<  i18n("KB");
          }

       stream << "\"><br><br></div>" << endl;

       // Footer

       if (m_configDlg->printPageCreationDate())
          {
          stream << "<hr>" << endl;
          QString valid = i18n("Valid HTML 4.01!");
          const QString today(KGlobal::locale()->formatDate(QDate::currentDate()));
          stream << "<div><img src=\"../thumbs/valid-html401.png\" alt=\"" << valid
                 << "\" height=\"31\" width=\"88\"  title=\"" << valid <<  "\" />" << endl;
          valid =  i18n("Images gallery created with "
                        "<a href=\"http://digikam.sourceforge.net\">Digikam</a> on %1").arg(today);
          stream << valid << "</div>" << endl;
          }

       stream << "</body></html>" << endl;
       file.close();

       return true;
       }

    return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImagesGallery::createThumb( const QString& imgName, const QString& sourceDirName,
                                 const QString& imgGalleryDir, const QString& imageFormat,
                                 const QString& TargetimagesFormat)
{
    bool threadDone = false;
    bool useBrokenImage = false;

    const QString pixPath = sourceDirName + imgName;

    // Create the target images with resizing factor.

    const QString TargetImageNameFormat = imgName + extension(TargetimagesFormat);

    const QString TargetImagesbDir = imgGalleryDir + QString::fromLatin1("/images/");
    int extentTargetImages;

    if (m_configDlg->useNotOriginalImageSize() == true)
        extentTargetImages = m_configDlg->getImagesResize();
    else
        extentTargetImages = -1;    // Use original image size.

    m_targetImgWidth = 640;         // Default resize values.
    m_targetImgHeight = 480;

    m_threadedImageResizing = new ResizeImage(this, pixPath, TargetImagesbDir, TargetimagesFormat,
                                              TargetImageNameFormat, &m_targetImgWidth, &m_targetImgHeight,
                                              extentTargetImages, m_configDlg->colorDepthSetTargetImages(),
                                              m_configDlg->getColorDepthTargetImages(),
                                              m_configDlg->useSpecificTargetimageCompression(),
                                              m_configDlg->getTargetImagesCompression(), &threadDone,
                                              &useBrokenImage);

    do            // TODO: added a queue like JPEGLossLess
       {
       usleep (100);
       }
    while  ( m_threadedImageResizing->running() == true );

    delete m_threadedImageResizing;

    if ( threadDone == false || useBrokenImage == true )
        m_resizeImagesWithError.append(pixPath);

    if ( threadDone == false ) return false;

    // Create the thumbnails.

    threadDone = false;
    useBrokenImage = false;

    const QString ImageNameFormat = imgName + extension(imageFormat);
    const QString thumbDir = imgGalleryDir + QString::fromLatin1("/thumbs/");
    int extent = m_configDlg->getThumbnailsSize();

    m_imgWidth = 120; // Setting the size of the images is
    m_imgHeight = 90; // required to generate faster 'loading' pages

    m_threadedImageResizing = new ResizeImage(this, pixPath, thumbDir, imageFormat, ImageNameFormat,
                                              &m_imgWidth, &m_imgHeight, extent,
                                              m_configDlg->colorDepthSetThumbnails(),
                                              m_configDlg->getColorDepthThumbnails(),
                                              m_configDlg->useSpecificThumbsCompression(),
                                              m_configDlg->getThumbsCompression(), &threadDone,
                                              &useBrokenImage);

    do            // TODO: added a queue like JPEGLossLess
       {
       usleep (100);
       }
    while  ( m_threadedImageResizing->running() == true );

    delete m_threadedImageResizing;

    return (threadDone);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::slotCancelled()
{
    m_cancelled = true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ImagesGallery::invokeWebBrowser(KURL url)
{
    if (m_configDlg->getWebBrowserName() == "Konqueror")
       kapp->invokeBrowser(url.url());       // Open Konqueror browser to show the main HTML page.

    if (m_configDlg->getWebBrowserName() == "Mozilla")
       {
       m_webBrowserProc = new KProcess;
       *m_webBrowserProc << "mozilla";
       *m_webBrowserProc << url.url();

       if (m_webBrowserProc->start() == false)
          KMessageBox::error(0, i18n("Cannot start 'mozilla' web browser.\nPlease, check your installation!"));
       }

    if (m_configDlg->getWebBrowserName() == "Netscape")
       {
       m_webBrowserProc = new KProcess;
       *m_webBrowserProc << "netscape";
       *m_webBrowserProc << url.url();

       if (m_webBrowserProc->start() == false)
          KMessageBox::error(0, i18n("Cannot start 'netscape' web browser.\nPlease, check your installation!"));
       }

    if (m_configDlg->getWebBrowserName() == "Opera")
       {
       m_webBrowserProc = new KProcess;
       *m_webBrowserProc << "opera";
       *m_webBrowserProc << url.url();

       if (m_webBrowserProc->start() == false)
          KMessageBox::error(0, i18n("Cannot start 'opera' web browser.\nPlease, check your installation!"));
       }

    if (m_configDlg->getWebBrowserName() == "Dillo")
       {
       m_webBrowserProc = new KProcess;
       *m_webBrowserProc << "dillo";
       *m_webBrowserProc << url.url();

       if (m_webBrowserProc->start() == false)
          KMessageBox::error(0, i18n("Cannot start 'dillo' web browser.\nPlease, check your installation!"));
       }

    if (m_configDlg->getWebBrowserName() == "Galeon")
       {
       m_webBrowserProc = new KProcess;
       *m_webBrowserProc << "galeon";
       *m_webBrowserProc << url.url();

       if (m_webBrowserProc->start() == false)
          KMessageBox::error(0, i18n("Cannot start 'galeon' web browser.\nPlease, check your installation!"));
       }

    if (m_configDlg->getWebBrowserName() == "Amaya")
       {
       m_webBrowserProc = new KProcess;
       *m_webBrowserProc << "amaya";
       *m_webBrowserProc << url.url();

       if (m_webBrowserProc->start() == false)
          KMessageBox::error(0, i18n("Cannot start 'amaya' web browser.\nPlease, check your installation!"));
       }

    if (m_configDlg->getWebBrowserName() == "Quanta")
       {
       m_webBrowserProc = new KProcess;
       *m_webBrowserProc << "quanta";
       *m_webBrowserProc << url.url();

       if (m_webBrowserProc->start() == false)
          KMessageBox::error(0, i18n("Cannot start 'quanta' web editor.\nPlease, check your installation!"));
       }

    if (m_configDlg->getWebBrowserName() == "Screem")
       {
       m_webBrowserProc = new KProcess;
       *m_webBrowserProc << "screem";
       *m_webBrowserProc << url.url();

       if (m_webBrowserProc->start() == false)
          KMessageBox::error(0, i18n("Cannot start 'screem' web editor.\nPlease, check your installation!"));
       }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Source code from Koffice 1.3

QString ImagesGallery::EscapeSgmlText(const QTextCodec* codec,
                      const QString& strIn,
                      const bool quot /* = false */ ,
                      const bool apos /* = false */ )
{
    QString strReturn;
    QChar ch;

    for (uint i=0 ; i<strIn.length() ; ++i)
    {
        ch=strIn[i];
        switch (ch.unicode())
        {
        case 38: // &
            {
                strReturn+="&amp;";
                break;
            }
        case 60: // <
            {
                strReturn+="&lt;";
                break;
            }
        case 62: // >
            {
                strReturn+="&gt;";
                break;
            }
        case 34: // "
            {
                if (quot)
                    strReturn+="&quot;";
                else
                    strReturn+=ch;
                break;
            }
        case 39: // '
            {
                // NOTE: HTML does not define &apos; by default (only XML/XHTML does)
                if (apos)
                    strReturn+="&apos;";
                else
                    strReturn+=ch;
                break;
            }
        default:
            {
                // verify that the character ch can be expressed in the
                // encoding in which we will write the HTML file.
                if (codec)
                {
                    if (!codec->canEncode(ch))
                    {
                        strReturn+=QString("&#%1;").arg(ch.unicode());
                        break;
                    }
                }
                strReturn+=ch;
                break;
            }
        }
    }

    return strReturn;
}

