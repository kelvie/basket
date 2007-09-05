/* vi: ts=8 sts=4 sw=4
 * kate: space-indent on; indent-width 4; mixedindent off; indent-mode cstyle;
 *
 * This file is part of the KDE project, module kfile.
 * Copyright (C) 2006 Luke Sandell <lasandell@gmail.com>
 *           (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
 *           (C) 2000 Geert Jansen <jansen@kde.org>
 *           (C) 2000 Kurt Granroth <granroth@kde.org>
 *           (C) 1997 Christoph Neerfeld <chris@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#include "kiconcanvas.h"
#include "kicondialog.h"

#include <config.h>

#include <kiconviewsearchline.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kimagefilepreview.h>
#include <kurldrag.h>
#include <k3multipledrag.h>

#include <qsortedlist.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <qdragobject.h>
#include <cmath>
#include <math.h>
#include <algorithm>

#ifdef HAVE_LIBART
#include <svgicons/ksvgiconengine.h>
#include <svgicons/ksvgiconpainter.h>
#endif

class KIconCanvasItem : public QTreeWidgetItem
{
  public:
    KIconCanvasItem ( QIconView * parent, const QString & key, const QPixmap & pixmap )
        : QTreeWidgetItem(parent)
    {
        setText(QFileInfo(key).baseName());
        setKey(key);
        setPixmap(pixmap);
        setDragEnabled(true);
        setDropEnabled(false);

    }

    int compare(QTreeWidgetItem *rhs) const
    {
        return QString::localeAwareCompare(text().toLower(), rhs->text().toLower());
    }
};

class KIconCanvas::KIconCanvasPrivate
{
  public:
    KIconCanvasPrivate()
    {
        m_bLoading = false;
        mSize = 0;
    }
    ~KIconCanvasPrivate()
    {
    }
    bool m_bLoading;
    QString mSetCurrent;
    int mSize;
#ifdef HAVE_LIBART
    KSvgIconEngine mSvgEngine;
#endif
    bool mStrictIconSize;
};

/*
 * KIconCanvas: Iconview for the iconloader dialog.
 */

KIconCanvas::KIconCanvas(QWidget *parent, const char *name)
    : K3IconView(parent, name)
{
    d = new KIconCanvasPrivate;
    mpLoader = KIconLoader::global();
    mpTimer = new QTimer(this);
    connect(mpTimer, SIGNAL(timeout()), SLOT(slotLoadFiles()));
    connect(this, SIGNAL(currentChanged(QTreeWidgetItem *)),
	    SLOT(slotCurrentChanged(QTreeWidgetItem *)));
    setAcceptDrops(false);
    setShowToolTips(true);
    setStrictIconSize(false);
}

KIconCanvas::~KIconCanvas()
{
    delete mpTimer;
    delete d;
}

void KIconCanvas::setIconLoader(KIconLoader *loader)
{
    mpLoader = loader;
}

void KIconCanvas::loadIcon(const QString &name)
{
    QImage img;
    QString path = mpLoader->iconPath(name,-d->mSize);
    // Use the extension as the format. Works for XPM and PNG, but not for SVG
    QString ext = path.right(3).toUpper();
    int maxSize = std::min(d->mSize, 60);

    if (ext != "SVG" && ext != "VGZ")
        img.load(path);
#ifdef HAVE_LIBART
   else
       if (d->mSvgEngine.load(maxSize, maxSize, path))
        img = *d->mSvgEngine.painter()->image();
#endif

   if (img.isNull())
       return;

   // For non-KDE icons
   if (d->mStrictIconSize && (img.width() != d->mSize || img.height() != d->mSize))
       return;

   if (img.width() > maxSize || img.height() > maxSize)
   {
       if (img.width() > img.height()) {
           int height = (int) ((float(maxSize) / img.width()) * img.height());
           img = img.smoothScale(maxSize, height);
       } else {
           int width = (int) ((float(maxSize) / img.height()) * img.width());
           img = img.smoothScale(width, maxSize);
       }
   }
   QPixmap pm;
   pm.fromImage(img);

   (void) new KIconCanvasItem(this, name, pm);
}

void KIconCanvas::loadFiles(const QStringList& files)
{
    clear();
    mFiles = files;
    emit startLoading(mFiles.count());
    mpTimer->start(10, true); // #86680
    d->m_bLoading = false;
}

void KIconCanvas::slotLoadFiles()
{
    setResizeMode(Fixed);
    QApplication::setOverrideCursor(waitCursor);

    // disable updates to not trigger paint events when adding child items
    setUpdatesEnabled( false );

    d->m_bLoading = true;
    int count;
    QStringList::ConstIterator it;
    QStringList::ConstIterator end(mFiles.end());
    for (it=mFiles.begin(), count=0; it!=end; ++it, count++)
    {
        loadIcon(*it);

        // Calling kapp->processEvents() makes the iconview flicker like hell
        // (it's being repainted once for every new item), so we don't do this.
        // Instead, we directly repaint the progress bar without going through
        // the event-loop. We do that just once for every 10th item so that
        // the progress bar doesn't flicker in turn. (pfeiffer)
        // FIXME: Qt4 will have double buffering
        if ( count % 10 == 0) {
            emit progress(count);
        }
        if ( !d->m_bLoading ) // user clicked on a button that will load another set of icons
            break;
    }

    // enable updates since we have to draw the whole view now
    sort();
    d->m_bLoading = false;
    setUpdatesEnabled( true );
    QApplication::restoreOverrideCursor();
    emit finished();
    setResizeMode(Adjust);
}

QString KIconCanvas::getCurrent() const
{
   return currentItem() ? currentItem()->key() : QString::null;
}

void KIconCanvas::stopLoading()
{
    d->m_bLoading = false;
}

void KIconCanvas::slotCurrentChanged(QTreeWidgetItem *item)
{
    emit nameChanged((item != 0L) ? item->text() : QString::null);
}

void KIconCanvas::setGroupOrSize( int groupOrSize )
{
    d->mSize = ((int)groupOrSize >= 0) ?
            mpLoader->currentSize((KIcon::Group)groupOrSize) :
            -groupOrSize;
}

void KIconCanvas::setStrictIconSize( bool strictIconSize )
{
    d->mStrictIconSize = strictIconSize;
}

QDragObject *KIconCanvas::dragObject()
{
    // We use QImageDrag rather than K3URLDrag so that the user can't drag an icon out of the theme!
    // TODO: support SVG?
    QPixmap *pixmap = currentIndex()->pixmap();
    QPoint pos = viewportToContents( viewport()->mapFromGlobal( QCursor::pos() ) );
    QPoint hot;
    hot.setX(pos.x() - currentItem()->pos().x() - (currentItem()->width() - pixmap->width()) / 2);
    hot.setY(pos.y() - currentItem()->pos().y() - (currentItem()->height() - pixmap->height()) / 2);
    QImageDrag *drag = new QImageDrag( pixmap->convertToImage(), this );
    drag->setPixmap(*pixmap, hot);
    return drag;
}

#include "kiconcanvas.moc"
