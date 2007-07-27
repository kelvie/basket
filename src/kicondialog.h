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

#ifndef __KIconDialog_h__
#define __KIconDialog_h__

#include <qstring.h>
#include <qstringlist.h>
#include <qpushbutton.h>

#include <kicontheme.h>
#include <kdialog.h>
#include <k3iconview.h>

#include <kiconcanvas.h> // FIXME: BCI KDE 3 expects KIconCanvas to be defined in kicondialog.h

class KIconDialogUI;

class QComboBox;
class QTimer;
class QKeyEvent;
class QRadioButton;
class KProgress;
class KIconLoader;

/**
 * Dialog for interactive selection of icons. Use the function
 * getIcon() let the user select an icon.
 *
 * @short An icon selection dialog.
 */
class KIO_EXPORT KIconDialog: public KDialog
{
    Q_OBJECT

public:
    /**
     * Constructs an icon selection dialog using the global iconloader.
     */
    KIconDialog(QWidget *parent=0L, const char *name=0L);
    /**
     * Constructs an icon selection dialog using a specific iconloader.
     */
    KIconDialog(KIconLoader *loader, QWidget *parent=0,
	    const char *name=0);
    /**
     * Destructs the dialog.
     */
    ~KIconDialog();

    /**
     * Sets a strict icon size policy for allowed icons. When true,
     * only icons of the specified group's size in getIcon() are shown.
     * When false, icons not available at the desired group's size will
     * also be selectable.
     */
    void setStrictIconSize(bool b);
    /**
     * Returns true if a strict icon size policy is set.
     */
    bool strictIconSize() const;

    /**
     * gets the custom icon directory
     */
    const QString & customLocation() const;

    /**
     * sets a custom icon directory
     * @since 3.1
     */
    void setCustomLocation( const QString& location );

    /**
     * Sets the size of the icons to be shown / selected.
     * @see KIcon::StdSizes
     * @see iconSize
     */
    void setIconSize(int size);

    /**
     * Returns the iconsize set via setIconSize() or 0, if the default
     * iconsize will be used.
     */
    int iconSize() const;

#ifndef KDE_NO_COMPAT
    /**
     * @deprecated in KDE 3.0, use the static method getIcon instead.
     */
    QString selectIcon(KIcon::Group group=KIcon::Desktop, KIcon::Context
	    context=KIcon::Application, bool user=false);
#endif

    /**
     * Allows you to set the same parameters as in the class method
     * getIcon().
     *
     */
    void setup( KIcon::Group group,
                KIcon::Context context = KIcon::Application,
                bool strictIconSize = false, int iconSize = 0,
                bool user = false );
    /* FIXME: KDE4 remove -- same as next method with default arguments */

    /**
     * Allows you to set the same parameters as in the class method
     * getIcon(), as well as two additional parameters to lock
     * the choice between system and user dirs and to lock the custom user
     * dir itself.
     *
     * @since 3.3
     */
    void setup( KIcon::Group group, KIcon::Context context,
                bool strictIconSize, int iconSize, bool user, bool lockContext,
                bool lockBrowse );
    // FIXME: KDE4 add default arguments (right now this would cause ambiguity with previous method)

    /**
     * exec()utes this modal dialog and returns the name of the selected icon,
     * or QString::null if the dialog was aborted.
     * @returns the name of the icon, suitable for loading with KIconLoader.
     * @see getIcon
     */
    QString openDialog();

    /**
     * show()es this dialog and emits a newIcon(const QString&) signal when
     * successful. QString::null will be emitted if the dialog was aborted.
     */
    void showDialog();

    /**
     * Pops up the dialog an lets the user select an icon.
     *
     * @param group The icon group this icon is intended for. Providing the
     * group shows the icons in the dialog with the same appearance as when
     * used outside the dialog.
     * @param context The initial icon context. Initially, the icons having
     * this context are shown in the dialog. The user can change this.
     * @param strictIconSize When true, only icons of the specified group's size
     * are shown, otherwise icon not available in the desired group's size
     * will also be selectable.
     * @param iconSize the size of the icons -- the default of the icongroup
     *        if set to 0
     * @param user Begin with the "user icons" instead of "system icons".
     * @param parent The parent widget of the dialog.
     * @param caption The caption to use for the dialog.
     * @return The name of the icon, suitable for loading with KIconLoader.
     * @version New in 3.0
     */
    static QString getIcon(KIcon::Group group=KIcon::Desktop,
                           KIcon::Context context=KIcon::Application,
                           bool strictIconSize=false, int iconSize = 0,
                           bool user=false, QWidget *parent=0,
                           const QString &caption=QString::null);

signals:
    void newIconName(const QString&);

protected slots:
    void slotOk();

private slots:
    void slotContext(int);
    void slotStartLoading(int);
    void slotProgress(int);
    void slotFinished();
    void slotAcceptIcons();
    void slotBrowse();
private:
    void init();
    void showIcons();

    int mGroupOrSize;
    KIcon::Context mContext;
    int mType;

    QStringList mFileList;

    // FIXME: the following fields are obsolete, remove in KDE4
    QComboBox *mpCombo;
    QPushButton *mpBrowseBut;
    QRadioButton *mpRb1, *mpRb2;
    KProgress *mpProgress;

    KIconLoader *mpLoader;

    KIconCanvas *mpCanvas;     // FIXME: obsolete, remove in KDE4
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KIconDialogPrivate;
    KIconDialogPrivate *d;
};


/**
 * A pushbutton for choosing an icon. Pressing on the button will open a
 * KIconDialog for the user to select an icon. The current icon will be
 * displayed on the button.
 *
 * @see KIconDialog
 * @short A push button that allows selection of an icon.
 */
class KIO_EXPORT KIconButton: public QPushButton
{
    Q_OBJECT
    Q_PROPERTY( QString icon READ icon WRITE setIcon RESET resetIcon )
    Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY( bool strictIconSize READ strictIconSize WRITE setStrictIconSize )
    Q_PROPERTY( QString customLocation READ customLocation WRITE setCustomLocation )

public:
    /**
     * Constructs a KIconButton using the global iconloader.
     */
    KIconButton(QWidget *parent=0L, const char *name=0L);

    /**
     * Constructs a KIconButton using a specific KIconLoader.
     */
    KIconButton(KIconLoader *loader, QWidget *parent, const char *name=0L);
    /**
     * Destructs the button.
     */
    ~KIconButton();

    /**
     * Sets a strict icon size policy for allowed icons. When true,
     * only icons of the specified group's size in setIconType are allowed,
     * and only icons of that size will be shown in the icon dialog.
     */
    void setStrictIconSize(bool b);
    /**
     * Returns true if a strict icon size policy is set.
     */
    bool strictIconSize() const;

    /**
     * Sets the icon group and context. Use K3Icon::NoGroup if you want to
     * allow icons for any group in the given context.
     */
    void setIconType(KIcon::Group group, KIcon::Context context, bool user=false);


    /**
     * Same as above method, but allows you to specify whether user and custom mode should be locked.
     */
    void setIconType(KIcon::Group group, KIcon::Context context, bool user, bool lockContext, bool lockBrowse);
    /* FIXME: KDE4 this should replace the above method using default params */

     /**
     * sets a custom icon directory
     */
    void setCustomLocation(const QString &custom);

    /**
     * get the custom icon directory
     */
    const QString & customLocation() const;

    /**
     * Sets the button's initial icon.
     */
    void setIcon(const QString& icon);

    /**
     * Resets the icon (reverts to an empty button).
     */
    void resetIcon();

    /**
     * Returns the name of the selected icon.
     */
    QString icon() const { return mIcon; }

    /**
     * Sets the size of the icon to be shown / selected.
     * @see KIcon::StdSizes
     * @see iconSize
     */
    void setIconSize( int size );

    /**
     * Returns the iconsize set via setIconSize() or 0, if the default
     * iconsize will be used.
     */
    int iconSize() const;

signals:
    /**
     * Emitted when the icon has changed.
     */
    void iconChanged(QString icon);
    /* FIXME: KDE4: Make it const QString & */

private slots:
    void slotChangeIcon();
    void newIconName(const QString& name);

private:
    void init( KIconLoader *loader );

    bool mbUser;
    KIcon::Group mGroup;
    KIcon::Context mContext;

    QString mIcon;
    KIconDialog *mpDialog;
    KIconLoader *mpLoader;
    class KIconButtonPrivate;
    KIconButtonPrivate *d;
};


#endif // __KIconDialog_h__
