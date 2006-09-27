/***************************************************************************
 *   Copyright (C) 2003 by Sébastien Laoût                                 *
 *   slaout@linux62.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <config.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtabwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <knuminput.h>
#include <kcolorcombo.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <qwhatsthis.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kgpgme.h>
#include <kdebug.h>

#include "basket.h"
#include "linklabel.h"
#include "settings.h"
#include "variouswidgets.h"
#include "note.h"

/** Settings */

// General:                                      // TODO: Use this grouping everywhere!
bool    Settings::s_useSystray           = true;
bool    Settings::s_usePassivePopup      = true;
bool    Settings::s_playAnimations       = true;
bool    Settings::s_showNotesToolTip     = true; // TODO: RENAME: useBasketTooltips
bool    Settings::s_confirmNoteDeletion  = true;
bool    Settings::s_bigNotes             = false;
bool    Settings::s_autoBullet           = true;
bool    Settings::s_exportTextTags       = true;
bool    Settings::s_useGnuPGAgent        = false;
bool    Settings::s_treeOnLeft           = true;
bool    Settings::s_filterOnTop          = true;
int     Settings::s_defImageX            = 300;
int     Settings::s_defImageY            = 200;
bool    Settings::s_enableReLockTimeout  = true;
int     Settings::s_reLockTimeoutMinutes = 0;
int     Settings::s_newNotesPlace        = 1;
int     Settings::s_viewTextFileContent  = false;
int     Settings::s_viewHtmlFileContent  = false;
int     Settings::s_viewImageFileContent = false;
int     Settings::s_viewSoundFileContent = false;
// Applications:
bool    Settings::s_htmlUseProg          = false; // TODO: RENAME: s_*App (with KService!)
bool    Settings::s_imageUseProg         = true;
bool    Settings::s_animationUseProg     = true;
bool    Settings::s_soundUseProg         = false;
QString Settings::s_htmlProg             = "quanta";
QString Settings::s_imageProg            = "kolourpaint";
QString Settings::s_animationProg        = "gimp";
QString Settings::s_soundProg            = "";
// Addictive Features:
bool    Settings::s_groupOnInsertionLine = false;
int     Settings::s_middleAction         = 0;
bool    Settings::s_showIconInSystray    = false; // TODO: RENAME: basketIconInSystray
bool    Settings::s_hideOnMouseOut       = false;
int     Settings::s_timeToHideOnMouseOut = 0;
bool    Settings::s_showOnMouseIn        = false;
int     Settings::s_timeToShowOnMouseIn  = 1;
// Rememberings:
int     Settings::s_defIconSize          = 32; // TODO: RENAME: importIconSize
bool    Settings::s_blinkedFilter        = false;
bool    Settings::s_startDocked          = false;
int     Settings::s_basketTreeWidth      = -1;
QPoint  Settings::s_mainWindowPosition   = QPoint();
QSize   Settings::s_mainWindowSize       = QSize();
bool    Settings::s_showEmptyBasketInfo  = true;
bool    Settings::s_spellCheckTextNotes  = true;

void Settings::loadConfig()
{
	LinkLook defaultSoundLook;
	LinkLook defaultFileLook;
	LinkLook defaultLocalLinkLook;
	LinkLook defaultNetworkLinkLook;
	LinkLook defaultLauncherLook; /* italic  bold    underlining                color      hoverColor  iconSize  preview */
	defaultSoundLook.setLook(        false,  false,  LinkLook::Never,           QColor(),  QColor(),   32,       LinkLook::None           );
	defaultFileLook.setLook(         false,  false,  LinkLook::Never,           QColor(),  QColor(),   32,       LinkLook::TwiceIconSize  );
	defaultLocalLinkLook.setLook(    true,   false,  LinkLook::OnMouseHover,    QColor(),  QColor(),   22,       LinkLook::TwiceIconSize  );
	defaultNetworkLinkLook.setLook(  false,  false,  LinkLook::OnMouseOutside,  QColor(),  QColor(),   16,       LinkLook::None           );
	defaultLauncherLook.setLook(     false,  true,   LinkLook::Never,           QColor(),  QColor(),   48,       LinkLook::None           );

	loadLinkLook(LinkLook::soundLook,       "Sound Look",        defaultSoundLook      );
	loadLinkLook(LinkLook::fileLook,        "File Look",         defaultFileLook       );
	loadLinkLook(LinkLook::localLinkLook,   "Local Link Look",   defaultLocalLinkLook  );
	loadLinkLook(LinkLook::networkLinkLook, "Network Link Look", defaultNetworkLinkLook);
	loadLinkLook(LinkLook::launcherLook,    "Launcher Look",     defaultLauncherLook   );

	KConfig* config = Global::config();
	config->setGroup("Main window"); // TODO: Split with a "System tray icon" group !
	setTreeOnLeft(           config->readBoolEntry("treeOnLeft",           true)  );
	setFilterOnTop(          config->readBoolEntry("filterOnTop",          true)  );
	setPlayAnimations(       config->readBoolEntry("playAnimations",       true)  );
	setShowNotesToolTip(     config->readBoolEntry("showNotesToolTip",     true)  );
	setBigNotes(             config->readBoolEntry("bigNotes",             false) );
	setConfirmNoteDeletion(  config->readBoolEntry("confirmNoteDeletion",  true)  );
	setAutoBullet(           config->readBoolEntry("autoBullet",           true)  );
	setExportTextTags(       config->readBoolEntry("exportTextTags",       true)  );
	setUseGnuPGAgent(        config->readBoolEntry("useGnuPGAgent",        false) );
	setBlinkedFilter(        config->readBoolEntry("blinkedFilter",        false) );
	setEnableReLockTimeout(  config->readNumEntry( "enableReLockTimeout",  true)  );
	setReLockTimeoutMinutes( config->readNumEntry( "reLockTimeoutMinutes", 0)     );
	setUseSystray(           config->readBoolEntry("useSystray",           true)  );
	setShowIconInSystray(    config->readBoolEntry("showIconInSystray",    false) );
	setStartDocked(          config->readBoolEntry("startDocked",          false) );
	setMiddleAction(         config->readNumEntry( "middleAction",         0)     );
	setGroupOnInsertionLine( config->readBoolEntry("groupOnInsertionLine", false) );
	setSpellCheckTextNotes(  config->readBoolEntry("spellCheckTextNotes",  true)  );
	setHideOnMouseOut(       config->readBoolEntry("hideOnMouseOut",       false) );
	setTimeToHideOnMouseOut( config->readNumEntry( "timeToHideOnMouseOut", 0)     );
	setShowOnMouseIn(        config->readBoolEntry("showOnMouseIn",        false) );
	setTimeToShowOnMouseIn(  config->readNumEntry( "timeToShowOnMouseIn",  1)     );
	setBasketTreeWidth(      config->readNumEntry( "basketTreeWidth",      -1)    );
	setUsePassivePopup(      config->readBoolEntry("usePassivePopup",      true)  );
	setMainWindowPosition(   config->readPointEntry("position"             )      );
	setMainWindowSize(       config->readSizeEntry( "size"                 )      );

	config->setGroup("Notification Messages");
	setShowEmptyBasketInfo(  config->readBoolEntry("emptyBasketInfo",      true)  );

	config->setGroup("Programs");
	setIsHtmlUseProg(        config->readBoolEntry("htmlUseProg",          false)         );
	setIsImageUseProg(       config->readBoolEntry("imageUseProg",         true)          );
	setIsAnimationUseProg(   config->readBoolEntry("animationUseProg",     true)          );
	setIsSoundUseProg(       config->readBoolEntry("soundUseProg",         false)         );
	setHtmlProg(             config->readEntry(    "htmlProg",             "quanta")      );
	setImageProg(            config->readEntry(    "imageProg",            "kolourpaint") );
	setAnimationProg(        config->readEntry(    "animationProg",        "gimp")        );
	setSoundProg(            config->readEntry(    "soundProg",            "")            );

	config->setGroup("Note Addition");
	setNewNotesPlace(        config->readNumEntry( "newNotesPlace",        1)             );
	setViewTextFileContent(  config->readBoolEntry("viewTextFileContent",  false)         );
	setViewHtmlFileContent(  config->readBoolEntry("viewHtmlFileContent",  false)         );
	setViewImageFileContent( config->readBoolEntry("viewImageFileContent", true)          );
	setViewSoundFileContent( config->readBoolEntry("viewSoundFileContent", true)          );

	config->setGroup("Insert Note Default Values");
	setDefImageX(   config->readNumEntry( "defImageX",   300) );
	setDefImageY(   config->readNumEntry( "defImageY",   200) );
	setDefIconSize( config->readNumEntry( "defIconSize", 32)  );

	config->setGroup("MainWindow Toolbar mainToolBar");
	// The first time we start, we define "Text Alongside Icons" for the main toolbar.
	// After that, the user is free to hide the text from the icons or customize as he/she want.
	// But it is a good default (Fitt's Laws, better looking, less "empty"-feeling), especially for this application.
//	if (!config->readBoolEntry("alreadySetIconTextRight", false)) {
//		config->writeEntry( "IconText",                "IconTextRight" );
//		config->writeEntry( "alreadySetIconTextRight", true            );
//	}
	if (!config->readBoolEntry("alreadySetToolbarSettings", false)) {
		config->writeEntry("IconText", "IconOnly"); // In 0.6.0 Alpha versions, it was made "IconTextRight". We're back to IconOnly
		config->writeEntry("Index",    "0");        // Make sure the main toolbar is the first...
		config->setGroup("MainWindow Toolbar richTextEditToolBar");
		config->writeEntry("Position", "Top");      // In 0.6.0 Alpha versions, it was made "Bottom"
		config->writeEntry("Index",    "1");        // ... and the rich text toolbar is on the right of the main toolbar
		config->setGroup("MainWindow Toolbar mainToolBar");
		config->writeEntry("alreadySetToolbarSettings", true);
	}
}

#include <iostream>

void Settings::saveConfig()
{
//	std::cout << "Settings::saveConfig()" << std::endl;

	saveLinkLook(LinkLook::soundLook,       "Sound Look"       );
	saveLinkLook(LinkLook::fileLook,        "File Look"        );
	saveLinkLook(LinkLook::localLinkLook,   "Local Link Look"  );
	saveLinkLook(LinkLook::networkLinkLook, "Network Link Look");
	saveLinkLook(LinkLook::launcherLook,    "Launcher Look"    );

 KConfig* config = Global::config();
	config->setGroup("Main window");
	config->writeEntry( "treeOnLeft",           treeOnLeft()           );
	config->writeEntry( "filterOnTop",          filterOnTop()          );
	config->writeEntry( "playAnimations",       playAnimations()       );
	config->writeEntry( "showNotesToolTip",     showNotesToolTip()     );
	config->writeEntry( "confirmNoteDeletion",  confirmNoteDeletion()  );
	config->writeEntry( "bigNotes",             bigNotes()             );
	config->writeEntry( "autoBullet",           autoBullet()           );
	config->writeEntry( "exportTextTags",       exportTextTags()       );
#ifdef HAVE_LIBGPGME
	if (KGpgMe::isGnuPGAgentAvailable())
		config->writeEntry( "useGnuPGAgent",    useGnuPGAgent()        );
#endif
	config->writeEntry( "blinkedFilter",        blinkedFilter()        );
	config->writeEntry( "enableReLockTimeout",  enableReLockTimeout()  );
	config->writeEntry( "reLockTimeoutMinutes", reLockTimeoutMinutes() );
	config->writeEntry( "useSystray",           useSystray()           );
	config->writeEntry( "showIconInSystray",    showIconInSystray()    );
	config->writeEntry( "startDocked",          startDocked()          );
	config->writeEntry( "middleAction",         middleAction()         );
	config->writeEntry( "groupOnInsertionLine", groupOnInsertionLine() );
	config->writeEntry( "spellCheckTextNotes",  spellCheckTextNotes()  );
	config->writeEntry( "hideOnMouseOut",       hideOnMouseOut()       );
	config->writeEntry( "timeToHideOnMouseOut", timeToHideOnMouseOut() );
	config->writeEntry( "showOnMouseIn",        showOnMouseIn()        );
	config->writeEntry( "timeToShowOnMouseIn",  timeToShowOnMouseIn()  );
	config->writeEntry( "basketTreeWidth",      basketTreeWidth()      );
	config->writeEntry( "usePassivePopup",      usePassivePopup()      );
	config->writeEntry( "position",             mainWindowPosition()   );
	config->writeEntry( "size",                 mainWindowSize()       );

	config->setGroup("Notification Messages");
	config->writeEntry( "emptyBasketInfo",      showEmptyBasketInfo()  );

	config->setGroup("Programs");
	config->writeEntry( "htmlUseProg",          isHtmlUseProg()        );
	config->writeEntry( "imageUseProg",         isImageUseProg()       );
	config->writeEntry( "animationUseProg",     isAnimationUseProg()   );
	config->writeEntry( "soundUseProg",         isSoundUseProg()       );
	config->writeEntry( "htmlProg",             htmlProg()             );
	config->writeEntry( "imageProg",            imageProg()            );
	config->writeEntry( "animationProg",        animationProg()        );
	config->writeEntry( "soundProg",            soundProg()            );

	config->setGroup("Note Addition");
	config->writeEntry( "newNotesPlace",        newNotesPlace()        );
	config->writeEntry( "viewTextFileContent",  viewTextFileContent()  );
	config->writeEntry( "viewHtmlFileContent",  viewHtmlFileContent()  );
	config->writeEntry( "viewImageFileContent", viewImageFileContent() );
	config->writeEntry( "viewSoundFileContent", viewSoundFileContent() );

	config->setGroup("Insert Note Default Values");
	config->writeEntry( "defImageX",         defImageX()         );
	config->writeEntry( "defImageY",         defImageY()         );
	config->writeEntry( "defIconSize",       defIconSize()       );
}

void Settings::loadLinkLook(LinkLook *look, const QString &name, const LinkLook &defaultLook)
{
 KConfig* config = Global::config();
	config->setGroup(name);

	QString underliningStrings[] = { "Always", "Never", "OnMouseHover", "OnMouseOutside" };
	QString defaultUnderliningString = underliningStrings[defaultLook.underlining()];

	QString previewStrings[] = { "None", "IconSize", "TwiceIconSize", "ThreeIconSize" };
	QString defaultPreviewString = previewStrings[defaultLook.preview()];

	bool    italic            = config->readBoolEntry(     "italic",      defaultLook.italic()     );
	bool    bold              = config->readBoolEntry(     "bold",        defaultLook.bold()       );
	QString underliningString = config->readEntry(         "underlining", defaultUnderliningString );
	QColor  color             = config->readPropertyEntry( "color",       defaultLook.color()      ).asColor();
	QColor  hoverColor        = config->readPropertyEntry( "hoverColor",  defaultLook.hoverColor() ).asColor();
	int     iconSize          = config->readNumEntry(      "iconSize",    defaultLook.iconSize()   );
	QString previewString     = config->readEntry(         "preview",     defaultPreviewString     );

	int underlining = 0;
	if      (underliningString == underliningStrings[1]) underlining = 1;
	else if (underliningString == underliningStrings[2]) underlining = 2;
	else if (underliningString == underliningStrings[3]) underlining = 3;

	int preview = 0;
	if      (previewString == previewStrings[1]) preview = 1;
	else if (previewString == previewStrings[2]) preview = 2;
	else if (previewString == previewStrings[3]) preview = 3;

	look->setLook(italic, bold, underlining, color, hoverColor, iconSize, preview);
}

void Settings::saveLinkLook(LinkLook *look, const QString &name)
{
 KConfig* config = Global::config();
	config->setGroup(name);

	QString underliningStrings[] = { "Always", "Never", "OnMouseHover", "OnMouseOutside" };
	QString underliningString = underliningStrings[look->underlining()];

	QString previewStrings[] = { "None", "IconSize", "TwiceIconSize", "ThreeIconSize" };
	QString previewString = previewStrings[look->preview()];

	config->writeEntry( "italic",      look->italic()     );
	config->writeEntry( "bold",        look->bold()       );
	config->writeEntry( "underlining", underliningString  );
	config->writeEntry( "color",       look->color()      );
	config->writeEntry( "hoverColor",  look->hoverColor() );
	config->writeEntry( "iconSize",    look->iconSize()   );
	config->writeEntry( "preview",     previewString      );
}

void Settings::setBigNotes(bool big)
{
	if (big == s_bigNotes)
		return;

	s_bigNotes = big;
	// Big notes for accessibility reasons OR Standard small notes:
	Note::NOTE_MARGIN      = (big ? 4 : 2);
	Note::INSERTION_HEIGHT = (big ? 5 : 3);
	Note::EXPANDER_WIDTH   = 9;
	Note::EXPANDER_HEIGHT  = 9;
	Note::GROUP_WIDTH      = 2*Note::NOTE_MARGIN + Note::EXPANDER_WIDTH;
	Note::HANDLE_WIDTH     = Note::GROUP_WIDTH;
	Note::RESIZER_WIDTH    = Note::GROUP_WIDTH;
	Note::TAG_ARROW_WIDTH  = 5 + (big ? 4 : 0);
	Note::EMBLEM_SIZE      = 16;
	Note::MIN_HEIGHT       = 2*Note::NOTE_MARGIN + Note::EMBLEM_SIZE;

	if (Global::bnpView)
		Global::bnpView->relayoutAllBaskets();
}

void Settings::setAutoBullet(bool yes)
{
	s_autoBullet = yes;
	if (Global::bnpView && Global::bnpView->currentBasket()) {
		Global::bnpView->currentBasket()->editorPropertiesChanged();
	}
}

/** GeneralPage */

GeneralPage::GeneralPage(QWidget * parent, const char * name)
 : KCModule(parent, name)
{
	QVBoxLayout *layout = new QVBoxLayout(this, /*margin=*/0, KDialogBase::spacingHint());
	QHBoxLayout *hLay;
	QLabel      *label;
	HelpLabel   *hLabel;

	QGridLayout *gl = new QGridLayout(layout, /*nRows=*/3, /*nCols=*/3);
	gl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 2);

	// Basket Tree Position:
	m_treeOnLeft = new QComboBox(this);
	m_treeOnLeft->insertItem(i18n("On left"));
	m_treeOnLeft->insertItem(i18n("On right"));
	label = new QLabel(m_treeOnLeft, i18n("Bask&et tree position:"), this);
	gl->addWidget(label,        0, 0);
	gl->addWidget(m_treeOnLeft, 0, 1);
	connect( m_treeOnLeft, SIGNAL(activated(int)), this, SLOT(changed()) );

	// Filter Bar Position:
	m_filterOnTop = new QComboBox(this);
	m_filterOnTop->insertItem(i18n("On top"));
	m_filterOnTop->insertItem(i18n("On bottom"));
	label = new QLabel(m_filterOnTop, i18n("&Filter bar position:"), this);
	gl->addWidget(label,         1, 0);
	gl->addWidget(m_filterOnTop, 1, 1);
	connect( m_filterOnTop, SIGNAL(activated(int)), this, SLOT(changed()) );

	// Use Baloons to Report Results of Global Actions:
	hLay = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	m_usePassivePopup = new QCheckBox(i18n("&Use balloons to report results of global actions"), this);
	connect( m_usePassivePopup, SIGNAL(stateChanged(int)), this, SLOT(changed()) );
	hLabel = new HelpLabel(
		i18n("What are global actions?"),
		("<p>" + i18n("You can configure global shortcuts to do some actions without having to show the main window. For instance, you can paste the clipboard content, take a color from "
		              "a point of the screen, etc. You can also use the mouse scroll wheel over the system tray icon to change the current basket. Or use the middle mouse button "
		              "on that icon to paste the current selection.") + "</p>" +
		"<p>" + i18n("When doing so, %1 pops up a little balloon message to inform you the action has been successfully done. You can disable that balloon.") + "</p>" +
		"<p>" + i18n("Note that those messages are smart enough to not appear if the main window is visible. This is because you already see the result of your actions in the main window.") + "</p>")
			.arg(kapp->aboutData()->programName()),
		this);
	hLay->addWidget(m_usePassivePopup);
	hLay->addWidget(hLabel);
	hLay->addStretch();
	layout->addLayout(hLay);

	// System Tray Icon:
	QGroupBox *gbSys = new QGroupBox(3, Qt::Vertical, i18n("System Tray Icon"), this);
	layout->addWidget(gbSys);
	QVBoxLayout *sysLay = new QVBoxLayout(gbSys, /*margin=*/0, KDialogBase::spacingHint());

	// Dock in System Tray:
	m_useSystray = new QCheckBox(i18n("Doc&k in system tray"), gbSys);
	sysLay->addWidget(m_useSystray);
	connect( m_useSystray, SIGNAL(stateChanged(int)), this, SLOT(changed()) );

	m_systray = new QWidget(gbSys);
	QVBoxLayout *subSysLay = new QVBoxLayout(m_systray, /*margin=*/0, KDialogBase::spacingHint());
	sysLay->addWidget(m_systray);

	// Show Current Basket Icon in System Tray Icon:
	m_showIconInSystray = new QCheckBox(i18n("Show c&urrent basket icon in system tray icon"), m_systray);
	subSysLay->addWidget(m_showIconInSystray);
	connect(m_showIconInSystray, SIGNAL(stateChanged(int)), this, SLOT(changed()));

	QGridLayout *gs = new QGridLayout(0, /*nRows=*/2, /*nCols=*/3);
	subSysLay->addLayout(gs);
	gs->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 2);

	// Hide Main Window when Mouse Goes out of it for Some Time:
	m_timeToHideOnMouseOut = new KIntNumInput(0, m_systray);
	m_hideOnMouseOut = new QCheckBox(i18n("H&ide main window when mouse goes out of it for"), m_systray);
	m_timeToHideOnMouseOut->setRange(0, 600, 1, false);
	m_timeToHideOnMouseOut->setSuffix(i18n(" tenths of seconds"));
	gs->addWidget(m_hideOnMouseOut,       0, 0);
	gs->addWidget(m_timeToHideOnMouseOut, 0, 1);
	connect(m_hideOnMouseOut, SIGNAL(stateChanged(int)), this, SLOT(changed()));
	connect(m_timeToHideOnMouseOut, SIGNAL(valueChanged (int)), this, SLOT(changed()));
//	subSysLay->addWidget(

	// Show Main Window when Mouse Hovers over the System Tray Icon for Some Time:
	m_timeToShowOnMouseIn = new KIntNumInput(0, m_systray);
	m_showOnMouseIn  = new QCheckBox(i18n("Show &main window when mouse hovers over the system tray icon for"), m_systray);
	m_timeToShowOnMouseIn->setRange(0, 600, 1, false);
	m_timeToShowOnMouseIn->setSuffix(i18n(" tenths of seconds"));
	gs->addWidget(m_showOnMouseIn,       1, 0);
	gs->addWidget(m_timeToShowOnMouseIn, 1, 1);
	connect(m_showOnMouseIn, SIGNAL(stateChanged(int)), this, SLOT(changed()));
	connect(m_timeToShowOnMouseIn, SIGNAL(valueChanged (int)), this, SLOT(changed()));

	connect( m_hideOnMouseOut, SIGNAL(toggled(bool)), m_timeToHideOnMouseOut, SLOT(setEnabled(bool)) );
	connect( m_showOnMouseIn,  SIGNAL(toggled(bool)), m_timeToShowOnMouseIn,  SLOT(setEnabled(bool)) );

	connect( m_useSystray,     SIGNAL(toggled(bool)), m_systray,              SLOT(setEnabled(bool)) );

	layout->insertStretch(-1);
	load();
}

void GeneralPage::load()
{
	m_treeOnLeft->setCurrentItem( (int)!Settings::treeOnLeft() );
	m_filterOnTop->setCurrentItem( (int)!Settings::filterOnTop() );

	m_usePassivePopup->setChecked(Settings::usePassivePopup());

	m_useSystray->setChecked(           Settings::useSystray()           );
	m_systray->setEnabled(              Settings::useSystray()           );

	m_showIconInSystray->setChecked(    Settings::showIconInSystray()    );

	m_hideOnMouseOut->setChecked(       Settings::hideOnMouseOut()       );
	m_timeToHideOnMouseOut->setValue(   Settings::timeToHideOnMouseOut() );
	m_timeToHideOnMouseOut->setEnabled( Settings::hideOnMouseOut()       );

	m_showOnMouseIn->setChecked(        Settings::showOnMouseIn()        );
	m_timeToShowOnMouseIn->setValue(    Settings::timeToShowOnMouseIn()  );
	m_timeToShowOnMouseIn->setEnabled(  Settings::showOnMouseIn()        );


}

void GeneralPage::save()
{
	Settings::setTreeOnLeft(           ! m_treeOnLeft->currentItem()    );
	Settings::setFilterOnTop(          ! m_filterOnTop->currentItem()   );

	Settings::setUsePassivePopup(      m_usePassivePopup->isChecked()   );

	Settings::setUseSystray(           m_useSystray->isChecked()        );
	Settings::setShowIconInSystray(    m_showIconInSystray->isChecked() );
	Settings::setShowOnMouseIn(        m_showOnMouseIn->isChecked()     );
	Settings::setTimeToShowOnMouseIn(  m_timeToShowOnMouseIn->value()   );
	Settings::setHideOnMouseOut(       m_hideOnMouseOut->isChecked()    );
	Settings::setTimeToHideOnMouseOut( m_timeToHideOnMouseOut->value()  );
}

void GeneralPage::defaults()
{
	// TODO
}

/** BasketsPage */

BasketsPage::BasketsPage(QWidget * parent, const char * name)
 : KCModule(parent, name)
{
	QVBoxLayout *layout = new QVBoxLayout(this, /*margin=*/0, KDialogBase::spacingHint());
	QHBoxLayout *hLay;
	HelpLabel   *hLabel;

	// Appearance:

	QGroupBox *appearanceBox = new QGroupBox(3, Qt::Vertical, i18n("Appearance"), this);
	layout->addWidget(appearanceBox);

	m_playAnimations = new QCheckBox(i18n("Ani&mate changes in baskets"), appearanceBox);
	connect( m_playAnimations, SIGNAL(stateChanged(int)), this, SLOT(changed()) );

	m_showNotesToolTip = new QCheckBox(i18n("Sho&w tooltips in baskets"), appearanceBox);
	connect( m_showNotesToolTip, SIGNAL(stateChanged(int)), this, SLOT(changed()) );

	m_bigNotes = new QCheckBox(i18n("Bi&g notes"), appearanceBox);
	connect( m_bigNotes, SIGNAL(stateChanged(int)), this, SLOT(changed()) );

	// Behavior:

	QGroupBox *behaviorBox = new QGroupBox(5, Qt::Vertical, i18n("Behavior"), this);
	layout->addWidget(behaviorBox);

	m_autoBullet = new QCheckBox(i18n("T&ransform lines starting with * or - to lists in text editors"), behaviorBox);
	connect( m_autoBullet, SIGNAL(stateChanged(int)), this, SLOT(changed()) );

	m_confirmNoteDeletion = new QCheckBox(i18n("Ask confirmation before &deleting notes"), behaviorBox);
	connect( m_confirmNoteDeletion, SIGNAL(stateChanged(int)), this, SLOT(changed()) );

	QWidget *widget = new QWidget(behaviorBox);
	hLay = new QHBoxLayout(widget, /*margin=*/0, KDialogBase::spacingHint());
	m_exportTextTags = new QCheckBox(i18n("E&xport tags in texts"), widget);
	connect( m_exportTextTags, SIGNAL(stateChanged(int)), this, SLOT(changed()) );

	QPixmap pixmapHelp(KGlobal::dirs()->findResource("data", "basket/images/tag_export_help.png"));
	QMimeSourceFactory::defaultFactory()->setPixmap("__resource_help_tag_export.png", pixmapHelp);
	hLabel = new HelpLabel(
		i18n("When does this apply?"),
		"<p>" + i18n("It does apply when you copy and paste, or drag and drop notes to a text editor.") + "</p>" +
		"<p>" + i18n("If enabled, this property lets you paste the tags as textual equivalents.") + "<br>" +
		i18n("For instance, a list of notes with the <b>To Do</b> and <b>Done</b> tags are exported as lines preceded by <b>[ ]</b> or <b>[x]</b>, "
		     "representing an empty checkbox and a checked box.") + "</p>" +
		"<p align='center'><img src=\"__resource_help_tag_export.png\"></p>",
		widget);
	hLay->addWidget(m_exportTextTags);
	hLay->addWidget(hLabel);
	hLay->addStretch();

	m_groupOnInsertionLineWidget = new QWidget(behaviorBox);
	QHBoxLayout *hLayV = new QHBoxLayout(m_groupOnInsertionLineWidget, /*margin=*/0, KDialogBase::spacingHint());
	m_groupOnInsertionLine = new QCheckBox(i18n("&Group a new note when clicking on the right of the insertion line"), m_groupOnInsertionLineWidget);
	QPixmap pixmap(KGlobal::dirs()->findResource("data", "basket/images/insertion_help.png"));
	QMimeSourceFactory::defaultFactory()->setPixmap("__resource_help_insertion_line.png", pixmap);
	HelpLabel *helpV = new HelpLabel(
		i18n("How to group a new note?"),
		i18n("<p>When this option is enabled, the insertion-line not only allows you to insert notes at the cursor position, but also allows you to group a new note with the one under the cursor:</p>") +
		"<p align='center'><img src=\"__resource_help_insertion_line.png\"></p>" +
		i18n("<p>Place your mouse between notes, where you want to add a new one.<br>"
		"Click on the <b>left</b> of the insertion-line middle-mark to <b>insert</b> a note.<br>"
		"Click on the <b>right</b> to <b>group</b> a note, with the one <b>below or above</b>, depending on where your mouse is.</p>"),
		m_groupOnInsertionLineWidget);
	hLayV->addWidget(m_groupOnInsertionLine);
	hLayV->addWidget(helpV);
	hLayV->insertStretch(-1);
	layout->addWidget(m_groupOnInsertionLineWidget);
	connect(m_groupOnInsertionLine, SIGNAL(stateChanged(int)), this, SLOT(changed()));

	widget = new QWidget(behaviorBox);
	QGridLayout *ga = new QGridLayout(widget, /*nRows=*/3, /*nCols=*/4, /*margin=*/0, KDialogBase::spacingHint());
	ga->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 3);

	m_middleAction = new QComboBox(widget);
	m_middleAction->insertItem( i18n("Do nothing")                    );
	m_middleAction->insertItem( i18n("Paste clipboard")               );
	m_middleAction->insertItem( i18n("Insert image note")             );
	m_middleAction->insertItem( i18n("Insert link note")              );
	m_middleAction->insertItem( i18n("Insert launcher note")          );
	/*m_middleAction->insertItem(*/ i18n("Insert color note")             /*)*/;
	/*m_middleAction->insertItem(*/ i18n("Grab screen zone")              /*)*/;
	/*m_middleAction->insertItem(*/ i18n("Insert color from screen")      /*)*/;
	/*m_middleAction->insertItem(*/ i18n("Load note from file")           /*)*/;
	/*m_middleAction->insertItem(*/ i18n("Import Launcher from KDE Menu") /*)*/;
	/*m_middleAction->insertItem(*/ i18n("Import icon")                   /*)*/;
	QLabel *labelM = new QLabel(m_middleAction, i18n("&Shift+middle-click anywhere:"), widget);
	ga->addWidget(labelM,                                          0, 0);
	ga->addWidget(m_middleAction,                                  0, 1);
	ga->addWidget(new QLabel(i18n("at cursor position"), widget),  0, 2);
	connect( m_middleAction, SIGNAL(activated(int)), this, SLOT(changed()) );

	// Protection:

	QGroupBox *protectionBox = new QGroupBox(3, Qt::Vertical, i18n("Password Protection"), this);
	layout->addWidget(protectionBox);
	widget = new QWidget(protectionBox);

	// Re-Lock timeout configuration
	hLay = new QHBoxLayout(widget, /*margin=*/0, KDialogBase::spacingHint());
	m_enableReLockTimeoutMinutes = new QCheckBox(i18n("Automatically &lock protected baskets when closed for"), widget);
	hLay->addWidget(m_enableReLockTimeoutMinutes);
	m_reLockTimeoutMinutes = new KIntNumInput(widget);
	m_reLockTimeoutMinutes->setMinValue(0);
	m_reLockTimeoutMinutes->setSuffix(i18n(" minutes"));
	hLay->addWidget(m_reLockTimeoutMinutes);
	//label = new QLabel(i18n("minutes"), this);
	//hLay->addWidget(label);
	hLay->addStretch();
//	layout->addLayout(hLay);
	connect( m_enableReLockTimeoutMinutes, SIGNAL(stateChanged(int)), this,                   SLOT(changed())        );
	connect( m_reLockTimeoutMinutes,       SIGNAL(valueChanged(int)), this,                   SLOT(changed())        );
	connect( m_enableReLockTimeoutMinutes, SIGNAL(toggled(bool)),     m_reLockTimeoutMinutes, SLOT(setEnabled(bool)) );

#ifdef HAVE_LIBGPGME
	m_useGnuPGAgent = new QCheckBox(i18n("Use GnuPG agent for &private/public key protected baskets"), protectionBox);
//	hLay->addWidget(m_useGnuPGAgent);
	connect( m_useGnuPGAgent, SIGNAL(stateChanged(int)), this, SLOT(changed()) );
#endif

	layout->insertStretch(-1);
	load();
}

void BasketsPage::load()
{
	m_playAnimations->setChecked(Settings::playAnimations());
	m_showNotesToolTip->setChecked(Settings::showNotesToolTip());
	m_bigNotes->setChecked(Settings::bigNotes());

	m_autoBullet->setChecked(Settings::autoBullet());
	m_confirmNoteDeletion->setChecked(Settings::confirmNoteDeletion());
	m_exportTextTags->setChecked(Settings::exportTextTags());

	m_groupOnInsertionLine->setChecked(Settings::groupOnInsertionLine());
	m_middleAction->setCurrentItem(Settings::middleAction());

	// The correctness of this code depends on the default of enableReLockTimeout
	// being true - otherwise, the reLockTimeoutMinutes widget is not disabled properly.
	m_enableReLockTimeoutMinutes->setChecked(Settings::enableReLockTimeout());
	m_reLockTimeoutMinutes->setValue(Settings::reLockTimeoutMinutes());
#ifdef HAVE_LIBGPGME
	m_useGnuPGAgent->setChecked(Settings::useGnuPGAgent());

	if (KGpgMe::isGnuPGAgentAvailable()) {
		m_useGnuPGAgent->setChecked(Settings::useGnuPGAgent());
	} else {
		m_useGnuPGAgent->setChecked(false);
		m_useGnuPGAgent->setEnabled(false);
	}
#endif
}

void BasketsPage::save()
{
	Settings::setPlayAnimations(       m_playAnimations->isChecked()       );
	Settings::setShowNotesToolTip(     m_showNotesToolTip->isChecked()     );
	Settings::setBigNotes(             m_bigNotes->isChecked()             );

	Settings::setAutoBullet(           m_autoBullet->isChecked()           );
	Settings::setConfirmNoteDeletion(  m_confirmNoteDeletion->isChecked()  );
	Settings::setExportTextTags(       m_exportTextTags->isChecked()       );

	Settings::setGroupOnInsertionLine( m_groupOnInsertionLine->isChecked() );
	Settings::setMiddleAction(         m_middleAction->currentItem()       );

	Settings::setEnableReLockTimeout(  m_enableReLockTimeoutMinutes->isChecked());
	Settings::setReLockTimeoutMinutes(m_reLockTimeoutMinutes->value());
#ifdef HAVE_LIBGPGME
	Settings::setUseGnuPGAgent(        m_useGnuPGAgent->isChecked()        );
#endif
}

void BasketsPage::defaults()
{
	// TODO
}

/** class NewNotesPage: */

NewNotesPage::NewNotesPage(QWidget * parent, const char * name)
 : KCModule(parent, name)
{
	QVBoxLayout *layout = new QVBoxLayout(this, /*margin=*/0, KDialogBase::spacingHint());
	QHBoxLayout *hLay;
	QLabel      *label;

	// Place of New Notes:

	hLay = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	m_newNotesPlace = new QComboBox(this);
	label = new QLabel(m_newNotesPlace, i18n("&Place of new notes:"), this);
	m_newNotesPlace->insertItem(i18n("On top"));
	m_newNotesPlace->insertItem(i18n("On bottom"));
	m_newNotesPlace->insertItem(i18n("At current note"));
	hLay->addWidget(label);
	hLay->addWidget(m_newNotesPlace);
	hLay->addStretch();
	//layout->addLayout(hLay);
	label->hide();
	m_newNotesPlace->hide();
	connect( m_newNotesPlace, SIGNAL(textChanged(const QString &)), this, SLOT(changed()) );

	// New Images Size:

	hLay = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	m_imgSizeX = new KIntNumInput(this);
	m_imgSizeX->setMinValue(1);
	m_imgSizeX->setMaxValue(4096);
	m_imgSizeX->setReferencePoint(100);
	connect( m_imgSizeX, SIGNAL(valueChanged(int)), this, SLOT(changed()) );
	label = new QLabel(m_imgSizeX, i18n("&New images size:"), this);
	hLay->addWidget(label);
	hLay->addWidget(m_imgSizeX);
	m_imgSizeY = new KIntNumInput(this);
	m_imgSizeY->setMinValue(1);
	m_imgSizeY->setMaxValue(4096);
	m_imgSizeY->setReferencePoint(100);
	connect( m_imgSizeY, SIGNAL(valueChanged(int)), this, SLOT(changed()) );
	label = new QLabel(m_imgSizeY, i18n("&by"), this);
	hLay->addWidget(label);
	hLay->addWidget(m_imgSizeY);
	label = new QLabel(i18n("pixels"), this);
	hLay->addWidget(label);
	m_pushVisualize = new QPushButton(i18n("&Visualize..."), this);
	hLay->addWidget(m_pushVisualize);
	hLay->addStretch();
	layout->addLayout(hLay);
	connect( m_pushVisualize, SIGNAL(clicked()), this, SLOT(visualize()) );

	// View File Content:

	QVButtonGroup *buttonGroup = new QVButtonGroup(i18n("View Content of Added Files for the Following Types"), this);
	m_viewTextFileContent  = new QCheckBox( i18n("Plain &text"),         buttonGroup );
	m_viewHtmlFileContent  = new QCheckBox( i18n("&HTML page"),          buttonGroup );
	m_viewImageFileContent = new QCheckBox( i18n("&Image or animation"), buttonGroup );
	m_viewSoundFileContent = new QCheckBox( i18n("&Sound"),              buttonGroup );
	layout->addWidget(buttonGroup);
	connect( m_viewTextFileContent,  SIGNAL(stateChanged(int)), this, SLOT(changed()) );
	connect( m_viewHtmlFileContent,  SIGNAL(stateChanged(int)), this, SLOT(changed()) );
	connect( m_viewImageFileContent, SIGNAL(stateChanged(int)), this, SLOT(changed()) );
	connect( m_viewSoundFileContent, SIGNAL(stateChanged(int)), this, SLOT(changed()) );

	layout->insertStretch(-1);
	load();
}

void NewNotesPage::load()
{
	m_newNotesPlace->setCurrentItem(Settings::newNotesPlace());

	m_imgSizeX->setValue(Settings::defImageX());
	m_imgSizeY->setValue(Settings::defImageY());

	m_viewTextFileContent->setChecked(  Settings::viewTextFileContent()  );
	m_viewHtmlFileContent->setChecked(  Settings::viewHtmlFileContent()  );
	m_viewImageFileContent->setChecked( Settings::viewImageFileContent() );
	m_viewSoundFileContent->setChecked( Settings::viewSoundFileContent() );
}

void NewNotesPage::save()
{
	Settings::setNewNotesPlace(m_newNotesPlace->currentItem());

	Settings::setDefImageX(m_imgSizeX->value());
	Settings::setDefImageY(m_imgSizeY->value());

	Settings::setViewTextFileContent(  m_viewTextFileContent->isChecked()  );
	Settings::setViewHtmlFileContent(  m_viewHtmlFileContent->isChecked()  );
	Settings::setViewImageFileContent( m_viewImageFileContent->isChecked() );
	Settings::setViewSoundFileContent( m_viewSoundFileContent->isChecked() );
}

void NewNotesPage::defaults()
{
	// TODO
}

void NewNotesPage::visualize()
{
	ViewSizeDialog size(this, m_imgSizeX->value(), m_imgSizeY->value());
	size.exec();
	m_imgSizeX->setValue(size.width());
	m_imgSizeY->setValue(size.height());
}

/** class NotesAppearancePage: */

NotesAppearancePage::NotesAppearancePage(QWidget * parent, const char * name)
 : KCModule(parent, name)
{
	QVBoxLayout *layout = new QVBoxLayout(this, /*margin=*/0, KDialogBase::spacingHint());
	QTabWidget *tabs = new QTabWidget(this);
	layout->addWidget(tabs);

	m_soundLook       = new LinkLookEditWidget(this, i18n("Conference audio record"),                         "sound",       tabs);
	m_fileLook        = new LinkLookEditWidget(this, i18n("Annual report"),                                   "document",    tabs);
	m_localLinkLook   = new LinkLookEditWidget(this, i18n("Home folder"),                                     "folder_home", tabs);
	m_networkLinkLook = new LinkLookEditWidget(this, "www.kde.org",             KMimeType::iconForURL("http://www.kde.org"), tabs);
	m_launcherLook    = new LinkLookEditWidget(this, i18n("Launch %1").arg(kapp->aboutData()->programName()), "basket",      tabs);
	tabs->addTab(m_soundLook,       i18n("&Sounds")       );
	tabs->addTab(m_fileLook,        i18n("&Files")        );
	tabs->addTab(m_localLinkLook,   i18n("&Local Links")  );
	tabs->addTab(m_networkLinkLook, i18n("&Network Links"));
	tabs->addTab(m_launcherLook,    i18n("Launc&hers")    );

	load();
}

void NotesAppearancePage::load()
{
	m_soundLook->set(LinkLook::soundLook);
	m_fileLook->set(LinkLook::fileLook);
	m_localLinkLook->set(LinkLook::localLinkLook);
	m_networkLinkLook->set(LinkLook::networkLinkLook);
	m_launcherLook->set(LinkLook::launcherLook);
}

void NotesAppearancePage::save()
{
	m_soundLook->saveChanges();
	m_fileLook->saveChanges();
	m_localLinkLook->saveChanges();
	m_networkLinkLook->saveChanges();
	m_launcherLook->saveChanges();
	Global::bnpView->linkLookChanged();
}

void NotesAppearancePage::defaults()
{
	// TODO
}

/** class ApplicationsPage: */

ApplicationsPage::ApplicationsPage(QWidget * parent, const char * name)
 : KCModule(parent, name)
{
	/* Applications page */
	QVBoxLayout *layout = new QVBoxLayout(this, /*margin=*/0, KDialogBase::spacingHint());

	m_htmlUseProg  = new QCheckBox(i18n("Open &text notes with a custom application:"), this);
	m_htmlProg     = new RunCommandRequester("", i18n("Open text notes with:"), this);
	QHBoxLayout *hLayH = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	hLayH->insertSpacing(-1, 20);
	hLayH->addWidget(m_htmlProg);
	connect(m_htmlUseProg, SIGNAL(stateChanged(int)), this, SLOT(changed()));
	connect(m_htmlProg->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(changed()));

	m_imageUseProg = new QCheckBox(i18n("Open &image notes with a custom application:"), this);
	m_imageProg    = new RunCommandRequester("", i18n("Open image notes with:"), this);
	QHBoxLayout *hLayI = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	hLayI->insertSpacing(-1, 20);
	hLayI->addWidget(m_imageProg);
	connect(m_imageUseProg, SIGNAL(stateChanged(int)), this, SLOT(changed()));
	connect(m_imageProg->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(changed()));

	m_animationUseProg = new QCheckBox(i18n("Open a&nimation notes with a custom application:"), this);
	m_animationProg    = new RunCommandRequester("", i18n("Open animation notes with:"), this);
	QHBoxLayout *hLayA = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	hLayA->insertSpacing(-1, 20);
	hLayA->addWidget(m_animationProg);
	connect(m_animationUseProg, SIGNAL(stateChanged(int)), this, SLOT(changed()));
	connect(m_animationProg->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(changed()));

	m_soundUseProg = new QCheckBox(i18n("Open so&und notes with a custom application:"), this);
	m_soundProg    = new RunCommandRequester("", i18n("Open sound notes with:"), this);
	QHBoxLayout *hLayS = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	hLayS->insertSpacing(-1, 20);
	hLayS->addWidget(m_soundProg);
	connect(m_soundUseProg, SIGNAL(stateChanged(int)), this, SLOT(changed()));
	connect(m_soundProg->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(changed()));

	QString whatsthis = i18n(
		"<p>If checked, the application defined below will be used when opening that type of note.</p>"
		"<p>Otherwise, the application you've configured to in Konqueror will be used.</p>");

	QWhatsThis::add(m_htmlUseProg,      whatsthis);
	QWhatsThis::add(m_imageUseProg,     whatsthis);
	QWhatsThis::add(m_animationUseProg, whatsthis);
	QWhatsThis::add(m_soundUseProg,     whatsthis);

	whatsthis = i18n(
		"<p>Define the application to use for opening that type of note instead of the "
		"application configured in Konqueror.</p>");

	QWhatsThis::add(m_htmlProg,      whatsthis);
	QWhatsThis::add(m_imageProg,     whatsthis);
	QWhatsThis::add(m_animationProg, whatsthis);
	QWhatsThis::add(m_soundProg,     whatsthis);

	layout->addWidget(m_htmlUseProg);
	layout->addItem(hLayH);
	layout->addWidget(m_imageUseProg);
	layout->addItem(hLayI);
	layout->addWidget(m_animationUseProg);
	layout->addItem(hLayA);
	layout->addWidget(m_soundUseProg);
	layout->addItem(hLayS);

	layout->addSpacing(KDialogBase::spacingHint());

	QHBoxLayout *hLay = new QHBoxLayout(0L, /*margin=*/0, /*spacing=*/0);
	HelpLabel *hl1 = new HelpLabel(
		i18n("How to change the application used to open Web links?"),
		i18n("<p>When opening Web links, they are opened in different applications, depending on the content of the link "
		     "(a Web page, an image, a PDF document...), like if they were files on your computer.</p>"
		     "<p>Here is how to do if you want every Web addresses to be opened in your Web browser. "
		     "It is useful if you are not using KDE (if you are using eg. GNOME, XFCE...).</p>"
		     "<ul>"
		     "<li>Open the KDE Control Center (if it is not available, try to type \"kcontrol\" in a command line terminal);</li>"
		     "<li>Go to the \"KDE Components\" and then \"Components Selector\" section;</li>"
		     "<li>Choose \"Web Browser\", check \"In the following browser:\" and enter the name of your Web browser (like \"firefox\" or \"epiphany\").</li>"
		     "</ul>"
		     "<p>Now, when you click <i>any</i> link that start with \"http://...\", it will be opened in your Web browser (eg. Mozilla Firefox or Epiphany or...).</p>"
		     "<p>For a more fine-grained configuration (like openning only Web pages in your Web browser), read the second help link.</p>"),
		this);
	hLay->addWidget(hl1);
	hLay->addStretch();
	layout->addLayout(hLay);

	hLay = new QHBoxLayout(0L, /*margin=*/0, /*spacing=*/0);
	HelpLabel *hl2 = new HelpLabel(
		i18n("How to change the applications used to open files and links?"),
		i18n("<p>Here is how to set the application to be used for each type of file. "
		     "This also applys to Web links if you choose not to open them systematically in a Web browser (see the first help link). "
		     "The default settings should be good enough for you, but this tip is useful if you are using GNOME, XFCE, or another environment than KDE.</p>"
		     "<p>This is an exemple how to open HTML pages in your Web browser (and keep using the other applications for other addresses or files). "
		     "Repeat those steps for each type of file you want to open in a specific application.</p>"
		     "<ul>"
		     "<li>Open the KDE Control Center (if it is not available, try to type \"kcontrol\" in a command line terminal);</li>"
		     "<li>Go to the \"KDE Components\" and then \"File Associations\" section;</li>"
		     "<li>In the tree, expand \"text\" and click \"html\";</li>"
		     "<li>In the applications list, add your Web browser as the first entry;</li>"
		     "<li>Do the same for the type \"application -> xhtml+xml\".</li>"
		     "</ul>"),
		this);
	hLay->addWidget(hl2);
	hLay->addStretch();
	layout->addLayout(hLay);

	connect( m_htmlUseProg,      SIGNAL(toggled(bool)), m_htmlProg,      SLOT(setEnabled(bool)) );
	connect( m_imageUseProg,     SIGNAL(toggled(bool)), m_imageProg,     SLOT(setEnabled(bool)) );
	connect( m_animationUseProg, SIGNAL(toggled(bool)), m_animationProg, SLOT(setEnabled(bool)) );
	connect( m_soundUseProg,     SIGNAL(toggled(bool)), m_soundProg,     SLOT(setEnabled(bool)) );

	layout->insertStretch(-1);
	load();
}

void ApplicationsPage::load()
{
	m_htmlProg->setRunCommand(Settings::htmlProg());
	m_htmlUseProg->setChecked(Settings::isHtmlUseProg());
	m_htmlProg->setEnabled(Settings::isHtmlUseProg());

	m_imageProg->setRunCommand(Settings::imageProg());
	m_imageUseProg->setChecked(Settings::isImageUseProg());
	m_imageProg->setEnabled(Settings::isImageUseProg());

	m_animationProg->setRunCommand(Settings::animationProg());
	m_animationUseProg->setChecked(Settings::isAnimationUseProg());
	m_animationProg->setEnabled(Settings::isAnimationUseProg());

	m_soundProg->setRunCommand(Settings::soundProg());
	m_soundUseProg->setChecked(Settings::isSoundUseProg());
	m_soundProg->setEnabled(Settings::isSoundUseProg());
}

void ApplicationsPage::save()
{
	Settings::setIsHtmlUseProg(      m_htmlUseProg->isChecked()      );
	Settings::setHtmlProg(           m_htmlProg->runCommand()        );

	Settings::setIsImageUseProg(     m_imageUseProg->isChecked()     );
	Settings::setImageProg(          m_imageProg->runCommand()       );

	Settings::setIsAnimationUseProg( m_animationUseProg->isChecked() );
	Settings::setAnimationProg(      m_animationProg->runCommand()   );

	Settings::setIsSoundUseProg(     m_soundUseProg->isChecked()     );
	Settings::setSoundProg(          m_soundProg->runCommand()       );
}

void ApplicationsPage::defaults()
{
	// TODO
}

#include "settings.moc"
