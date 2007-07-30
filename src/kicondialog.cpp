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

#include "kicondialogui.h"
#include "kicondialog.h"
#include "kiconcanvas.h"

#include <config.h>

#include <kiconviewsearchline.h>

#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <QProgressBar>
#include <k3iconview.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qsortedlist.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qfileinfo.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qhbuttongroup.h>
#include <qdragobject.h>

/* NOTE: Must be in the same order as listbox */
enum ExtendedContext
{
    ALL = 0,
    RECENT = 1,
    // Action thru MimeType, subtract 1 to convert to KIcon::Context
    OTHER = 7
};

class KIconDialog::KIconDialogPrivate
{
  public:
    KIconDialogPrivate() {
       	m_bStrictIconSize = true;
    }
    ~KIconDialogPrivate() {}
    bool m_bStrictIconSize;
    QString custom;
    QString customLocation;
    int recentMax;
    QStringList recentList;
    ExtendedContext extendedContext;
    KIconDialogUI *ui; /* FIXME: KDE4 probably move this to the main class */
};

/*
 * KIconDialog: Dialog for selecting icons. Both system and user
 * specified icons can be chosen.
 */

KIconDialog::KIconDialog(QWidget *parent, const char*)
    : KDialog(parent, "IconDialog", true, i18n("Select Icon"), Ok|Cancel, Ok)
{
    d = new KIconDialogPrivate;
    mpLoader = KIconLoader::global();
    init();
    resize(minimumSize());
}

KIconDialog::KIconDialog(KIconLoader *loader, QWidget *parent,
	const char *name)
    : KDialog(parent, name, true, i18n("Select Icon"), Ok|Cancel, Ok)
{
    d = new KIconDialogPrivate;
    mpLoader = loader;
    init();
}

void KIconDialog::init()
{
    mGroupOrSize = KIcon::Desktop;
    d->extendedContext = ALL;
    mType = 0;
    setCustomLocation(QString::null); // Initialize mFileList

    // Read configuration
    KConfig *config = KGlobal::config();
    KConfigGroup group(config, "KIconDialog");
    d->recentMax = group.readNumEntry("RecentMax", 10);
    d->recentList = group.readPathListEntry("RecentIcons");

    d->ui = new KIconDialogUI( this );
    setMainWidget(d->ui);

    d->ui->searchLine->setIconView(d->ui->iconCanvas);
    d->ui->searchLine->setCaseSensitive(false);

    // Hack standard Gui item, as KDevDesigner won't let us
    d->ui->browseButton->setText(i18n("&Browse..."));

    connect(d->ui->browseButton, SIGNAL(clicked()), SLOT(slotBrowse()));
    connect(d->ui->listBox, SIGNAL(highlighted(int)), SLOT(slotContext(int)));
    connect(d->ui->iconCanvas, SIGNAL(executed(QListWidgetItem *)), SLOT(slotOk()));
    connect(d->ui->iconCanvas, SIGNAL(returnPressed(QListWidgetItem *)), SLOT(slotOk()));
    connect(d->ui->iconCanvas, SIGNAL(startLoading(int)), SLOT(slotStartLoading(int)));
    connect(d->ui->iconCanvas, SIGNAL(progress(int)), SLOT(slotProgress(int)));
    connect(d->ui->iconCanvas, SIGNAL(finished()), SLOT(slotFinished()));
    connect(this, SIGNAL(hidden()), d->ui->iconCanvas, SLOT(stopLoading()));

    // NOTE: this must be consistent with the IconType enum (see above)
    d->ui->listBox->insertItem(i18n("(All Icons)"));
    d->ui->listBox->insertItem(i18n("(Recent)"));
    d->ui->listBox->insertItem(i18n("Actions"));
    d->ui->listBox->insertItem(i18n("Applications"));
    d->ui->listBox->insertItem(i18n("Devices"));
    d->ui->listBox->insertItem(i18n("Filesystem"));
    d->ui->listBox->insertItem(i18n("File Types"));
    d->ui->listBox->insertItem(i18n("Miscellaneous"));
}

KIconDialog::~KIconDialog()
{
    // Write configuration
    KConfig *config = KGlobal::config();
    KConfigGroup group(config, "KIconDialog");
    group.writeEntry("RecentMax", d->recentMax, KConfigBase::Normal|KConfigBase::Global);
    group.writePathEntry("RecentIcons", d->recentList, ', ', KConfigBase::Normal|KConfigBase::Global);

    delete d;
}

void KIconDialog::slotAcceptIcons()
{
    //FIXME: not used anymore
}

void KIconDialog::showIcons()
{
    d->ui->iconCanvas->clear();
    QStringList filelist;

    KIcon::Context context = static_cast<KIcon::Context>(d->extendedContext - 1);
    switch (d->extendedContext)
    {
        case RECENT:
            filelist = d->recentList;
            break;
        case OTHER:
            filelist = mFileList;
            break;
        case ALL:
            filelist = mFileList;
            context = KIcon::Any;
            // ** Fallthrough to next case **
        default:
            QStringList list;
            if (d->m_bStrictIconSize)
                list=mpLoader->queryIcons(mGroupOrSize, context);
            else
                list=mpLoader->queryIconsByContext(mGroupOrSize, context);

            // Remove path & extension
            for ( QStringList::iterator it = list.begin(); it != list.end(); ++it)
                filelist.append(QFileInfo(*it).baseName());
    }

    // Remove duplicate icons FIXME: Qt4 we can just use QSet
    filelist.sort();
    QString prev;
    for ( QStringList::iterator it = filelist.begin(); it != filelist.end(); )
    {
        if (*it == prev)
        {
            it = filelist.remove(it);
        }
        else
        {
            prev = *it;
            ++it;
        }
    }

    d->ui->iconCanvas->setGroupOrSize(mGroupOrSize);
    d->ui->iconCanvas->loadFiles(filelist);
}

void KIconDialog::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize=b;
}

bool KIconDialog::strictIconSize() const
{
    return d->m_bStrictIconSize;
}

void KIconDialog::setIconSize( int size )
{
    // see KIconLoader, if you think this is weird
    if ( size == 0 )
        mGroupOrSize = KIcon::Desktop; // default Group
    else
        mGroupOrSize = -size; // yes, KIconLoader::queryIconsByContext is weird
}

int KIconDialog::iconSize() const
{
    // 0 or any other value ==> mGroupOrSize is a group, so we return 0
    return (mGroupOrSize < 0) ? -mGroupOrSize : 0;
}

#ifndef KDE_NO_COMPAT
QString KIconDialog::selectIcon(KIcon::Group group, KIcon::Context context, bool user)
{
    setup( group, context, false, 0, user );
    return openDialog();
}
#endif

void KIconDialog::setup(KIcon::Group group, KIcon::Context context,
                        bool strictIconSize, int iconSize, bool user )
{
    setup(group, context, strictIconSize, iconSize, user, false, false);
}

void KIconDialog::setup(KIcon::Group group, KIcon::Context context,
                        bool strictIconSize, int iconSize, bool user,
                        bool lockContext, bool lockBrowse )
{
    d->m_bStrictIconSize = strictIconSize;
    d->ui->iconCanvas->setStrictIconSize(strictIconSize);
    mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    mType = user;

    d->extendedContext = static_cast<ExtendedContext>( ( context == KIcon::Any ) ? ALL : context + 1 );

    // We cannot change layout because it is protected ;-(
    // FIXME: Qt4 we will be able to inherit from both QDialog and our GUI
    d->ui->listBox->setEnabled(!lockContext);
    d->ui->browseButton->setEnabled(!lockBrowse);
    d->ui->listBox->setHidden(lockContext && lockBrowse);
    d->ui->browseButton->setHidden(lockContext && lockBrowse);

    d->ui->listBox->setCurrentItem(d->extendedContext);
}

const QString & KIconDialog::customLocation( ) const
{
    return d->customLocation;
}

void KIconDialog::setCustomLocation( const QString& location )
{
    d->customLocation = location;

    if (location.isEmpty())
    {
        mFileList = KGlobal::dirs()->findAllResources("appicon", QString::fromLatin1("*.png"));
    } else {
        mFileList = mpLoader->queryIconsByDir(location);
    }
}

QString KIconDialog::openDialog()
{
    showIcons();

    if ( exec() == Accepted )
    {
         if (!d->custom.isEmpty())
             return d->custom;
         else
             return d->ui->iconCanvas->getCurrent();
    }
    else
    {
        return QString::null;
    }
}

void KIconDialog::showDialog()
{
    d->custom = QString::null;

    // Make it so minimumSize returns correct value
    d->ui->filterLabel->hide();
    d->ui->searchLine->hide();
    d->ui->progressBar->show();

    setModal(false);
    show();

    // FIXME: this should be before show() but it doesn't work ;-(
    resize(minimumSize());

    showIcons();
}

void KIconDialog::slotOk()
{
    QString key = !d->custom.isEmpty() ? d->custom : d->ui->iconCanvas->getCurrent();

    // Append to list of recent icons
    if (!d->recentList.contains(key))
    {
        d->recentList.push_back(key);

        // Limit recent list in size
        while ( (int)d->recentList.size() > d->recentMax )
            d->recentList.pop_front();
    }

    emit newIconName(key);
    KDialog::slotOk();
}

QString KIconDialog::getIcon(KIcon::Group group, KIcon::Context context,
                             bool strictIconSize, int iconSize, bool user,
                             QWidget *parent, const QString &caption)
{
    KIconDialog dlg(parent, "icon dialog");
    dlg.setup( group, context, strictIconSize, iconSize, user );
    if (!caption.isNull())
        dlg.setCaption(caption);

    return dlg.openDialog();
}

void KIconDialog::slotBrowse()
{
    // Create a file dialog to select a PNG, XPM or SVG file,
    // with the image previewer shown.
    // KFileDialog::getImageOpenUrl doesn't allow svg.
    KFileDialog dlg(QString::null, i18n("*.png *.xpm *.svg *.svgz|Icon Files (*.png *.xpm *.svg *.svgz)"),
                    this, "filedialog", true);
    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setCaption( i18n("Open") );
    dlg.setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
    KImageFilePreview *ip = new KImageFilePreview( &dlg );
    dlg.setPreviewWidget( ip );
    dlg.exec();

    QString file = dlg.selectedFile();
    if (!file.isEmpty())
    {
        d->custom = file;
        if ( mType == 1 )
            setCustomLocation(QFileInfo( file ).absolutePath());
        slotOk();
    }
}

void KIconDialog::slotContext(int id)
{
    d->extendedContext = static_cast<ExtendedContext>(id);
    showIcons();
}

void KIconDialog::slotStartLoading(int steps)
{
    if (steps < 10)
    	d->ui->progressBar->hide();
    else
    {
        d->ui->progressBar->setTotalSteps(steps);
        d->ui->progressBar->setProgress(0);
        d->ui->progressBar->show();
        d->ui->filterLabel->hide();
        d->ui->searchLine->hide();
    }
}

void KIconDialog::slotProgress(int p)
{
    d->ui->progressBar->setProgress(p);
}

void KIconDialog::slotFinished()
{
    d->ui->progressBar->hide();
    d->ui->filterLabel->show();
    d->ui->searchLine->show();
}

class KIconButton::KIconButtonPrivate
{
  public:
    KIconButtonPrivate() {
        m_bStrictIconSize = false;
        iconSize = 0; // let KIconLoader choose the default
    }
    ~KIconButtonPrivate() {}
    bool m_bStrictIconSize;
    bool lockUser;
    bool lockCustom;
    int iconSize;
};


/*
 * KIconButton: A "choose icon" pushbutton.
 */

KIconButton::KIconButton(QWidget *parent, const char *name)
    : QPushButton(parent, name)
{
    init( KIconLoader::global() );
}

KIconButton::KIconButton(KIconLoader *loader,
	QWidget *parent, const char *name)
    : QPushButton(parent, name)
{
    init( loader );
}

void KIconButton::init( KIconLoader *loader )
{
    d = new KIconButtonPrivate;
    mGroup = KIcon::Desktop;
    mContext = KIcon::Application;
    mbUser = false;

    mpLoader = loader;
    mpDialog = 0L;
    connect(this, SIGNAL(clicked()), SLOT(slotChangeIcon()));
}

KIconButton::~KIconButton()
{
    delete mpDialog;
    delete d;
}

void KIconButton::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize=b;
}

bool KIconButton::strictIconSize() const
{
    return d->m_bStrictIconSize;
}

void KIconButton::setIconSize( int size )
{
    d->iconSize = size;
}

int KIconButton::iconSize() const
{
    return d->iconSize;
}

void KIconButton::setIconType(KIcon::Group group, KIcon::Context context, bool user)
{
    mGroup = group;
    mContext = context;
    mbUser = user;
    d->lockUser = false;
    d->lockCustom = false;
}

void KIconButton::setIconType(KIcon::Group group, KIcon::Context context, bool user, bool lockUser, bool lockCustom)
{
    mGroup = group;
    mContext = context;
    mbUser = user;
    d->lockUser = lockUser;
    d->lockCustom = lockCustom;
}

void KIconButton::setIcon(const QString& icon)
{
    mIcon = icon;
    setIconSet(mpLoader->loadIconSet(mIcon, mGroup, d->iconSize));

    if (!mpDialog)
    {
        mpDialog = new KIconDialog(mpLoader, this);
        connect(mpDialog, SIGNAL(newIconName(const QString&)), SLOT(newIconName(const QString&)));
    }
}

const QString & KIconButton::customLocation( ) const
{
    return mpDialog ? mpDialog->customLocation() : QString::null;
}

void KIconButton::setCustomLocation(const QString &custom)
{
    if (!mpDialog)
    {
        mpDialog = new KIconDialog(mpLoader, this);
        connect(mpDialog, SIGNAL(newIconName(const QString&)), SLOT(newIconName(const QString&)));
    }

    mpDialog->setCustomLocation(custom);
}

void KIconButton::resetIcon()
{
    mIcon = QString::null;
    setIconSet(QIconSet());
}

void KIconButton::slotChangeIcon()
{
    if (!mpDialog)
    {
        mpDialog = new KIconDialog(mpLoader, this);
        connect(mpDialog, SIGNAL(newIconName(const QString&)), SLOT(newIconName(const QString&)));
    }

    mpDialog->setup( mGroup, mContext, d->m_bStrictIconSize, d->iconSize, mbUser, d->lockUser, d->lockCustom );
    mpDialog->showDialog();
}

void KIconButton::newIconName(const QString& name)
{
    if (name.isEmpty())
        return;

    QIconSet iconset = mpLoader->loadIconSet(name, mGroup, d->iconSize);
    setIconSet(iconset);
    mIcon = name;

    emit iconChanged(name);
}

void KIconCanvas::virtual_hook( int id, void* data )
{ K3IconView::virtual_hook( id, data ); }

void KIconDialog::virtual_hook( int id, void* data )
{ KDialog::virtual_hook( id, data ); }

#include "kicondialog.moc"
