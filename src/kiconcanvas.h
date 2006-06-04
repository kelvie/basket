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

#ifndef _KICONCANVAS_H_
#define _KICONCANVAS_H_

#include <qstring.h>
#include <qstringlist.h>
#include <kiconview.h>

class QTimer;
class KIconLoader;
class QDragObject;
class QIconLoader;

/**
 * Icon canvas for KIconDialog.
 */
class KIO_EXPORT KIconCanvas: public KIconView
/* NOTE: Why export this? */
{
    Q_OBJECT

public:
    KIconCanvas(QWidget *parent=0L, const char *name=0L);
    ~KIconCanvas();

    /**
     * Load icons into the canvas.
     */
    void loadFiles(const QStringList& files);

    /**
     * Returns the current icon.
     */
    QString getCurrent() const;

    void setIconLoader(KIconLoader *loader);

    void setGroupOrSize(int groupOrSize);

    void setStrictIconSize(bool strictIconSize);

public slots:
    void stopLoading();

signals:
    /**
     * Emitted when the current icon has changed.
     */
    void nameChanged(QString);
    /* KDE 4: Make it const QString */

    void startLoading(int);
    void progress(int);
    void finished();

private slots:
    void slotLoadFiles();
    void slotCurrentChanged(QIconViewItem *item);

private:
    QStringList mFiles;
    QTimer *mpTimer;
    KIconLoader *mpLoader;

protected:
    virtual void virtual_hook( int id, void* data );
    virtual QDragObject *dragObject();
    void loadIcon(const QString &path);

private:
    class KIconCanvasPrivate;
    KIconCanvasPrivate *d;
};

#endif
