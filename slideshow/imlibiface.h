/* ============================================================
 * File  : imlibiface.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-16
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#ifndef IMLIBIFACE_H
#define IMLIBIFACE_H

class QWidget;
class QString;

class ImImageSSPrivate;
class ImlibIface;
class ImlibIfacePrivate;

// ---------------------------------------------------------------

class ImImageSS
{
    friend class ImlibIface;
    
public:
    
    ImImageSS(ImlibIface *imIface, const QString& file);
    ~ImImageSS();

    bool valid();
    void fitSize(int width, int height);
    void render();
    QPixmap* qpixmap();
    QString  filename();  
    
private:

    ImImageSSPrivate *d;
    ImlibIface     *imIface_;
};

// ---------------------------------------------------------------

class ImlibIface
{
    friend class ImImageSS;
    
public:

    ImlibIface(QWidget *parent);
    ~ImlibIface();

    void paint(ImImageSS *image, int sx, int sy,
               int dx, int dy, int dw, int dh);
    void* imlibData();

private:

    ImlibIfacePrivate *d;

};


#endif /* IMLIBIFACE_H */
