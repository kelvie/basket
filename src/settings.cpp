/***************************************************************************
 *   Copyright (C) 2003 by S�astien Laot                                 *
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

#include "linklabel.h"
#include "settings.h"
#include "container.h"
#include "variouswidgets.h"
#include "note.h"

/** Settings */

// General:                                      // TODO: Use this grouping everywhere!
bool    Settings::s_useSystray           = true;
bool    Settings::s_usePassivePopup      = true;
bool    Settings::s_playAnimations       = true;
bool    Settings::s_showNotesToolTip     = true; // TODO: RENAME: useBasketTooltips
bool    Settings::s_bigNotes             = false;
bool    Settings::s_exportTextTags       = true;
bool    Settings::s_useGnuPGAgent        = false;
bool    Settings::s_treeOnLeft           = true;
bool    Settings::s_filterOnTop          = true;
int     Settings::s_defImageX            = 300;
int     Settings::s_defImageY            = 200;
int     Settings::s_newNotesPlace        = 1;
int     Settings::s_viewTextFileContent  = false;
int     Settings::s_viewHtmlFileContent  = false;
int     Settings::s_viewImageFileContent = false;
int     Settings::s_viewSoundFileContent = false;
// Applications:
bool    Settings::s_textUseProg          = false; // TODO: RENAME: s_*App (with KService!)
bool    Settings::s_htmlUseProg          = false;
bool    Settings::s_imageUseProg         = true;
bool    Settings::s_animationUseProg     = true;
bool    Settings::s_soundUseProg         = false;
QString Settings::s_textProg             = "";
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

	KConfig *config = KGlobal::config();
	config->setGroup("Main window"); // TODO: Split with a "System tray icon" group !
	setTreeOnLeft(           config->readBoolEntry("treeOnLeft",           true)  );
	setFilterOnTop(          config->readBoolEntry("filterOnTop",          true)  );
	setPlayAnimations(       config->readBoolEntry("playAnimations",       true)  );
	setShowNotesToolTip(     config->readBoolEntry("showNotesToolTip",     true)  );
	setBigNotes(             config->readBoolEntry("bigNotes",             false) );
	setExportTextTags(       config->readBoolEntry("exportTextTags",       true)  );
	setUseGnuPGAgent(        config->readBoolEntry("useGnuPGAgent",        false) );
	setBlinkedFilter(        config->readBoolEntry("blinkedFilter",        false) );
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
	setIsTextUseProg(        config->readBoolEntry("textUseProg",          false)         );
	setIsHtmlUseProg(        config->readBoolEntry("htmlUseProg",          false)         );
	setIsImageUseProg(       config->readBoolEntry("imageUseProg",         true)          );
	setIsAnimationUseProg(   config->readBoolEntry("animationUseProg",     true)          );
	setIsSoundUseProg(       config->readBoolEntry("soundUseProg",         false)         );
	setTextProg(             config->readEntry(    "textProg",             "")            );
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

void Settings::saveConfig()
{
	saveLinkLook(LinkLook::soundLook,       "Sound Look"       );
	saveLinkLook(LinkLook::fileLook,        "File Look"        );
	saveLinkLook(LinkLook::localLinkLook,   "Local Link Look"  );
	saveLinkLook(LinkLook::networkLinkLook, "Network Link Look");
	saveLinkLook(LinkLook::launcherLook,    "Launcher Look"    );

	KConfig *config = KGlobal::config();
	config->setGroup("Main window");
	config->writeEntry( "treeOnLeft",           treeOnLeft()           );
	config->writeEntry( "filterOnTop",          filterOnTop()          );
	config->writeEntry( "playAnimations",       playAnimations()       );
	config->writeEntry( "showNotesToolTip",     showNotesToolTip()     );
	config->writeEntry( "bigNotes",             bigNotes()             );
	config->writeEntry( "exportTextTags",       exportTextTags()       );
#ifdef HAVE_LIBGPGME
	if(KGpgMe::isGnuPGAgentAvailable())
		config->writeEntry( "useGnuPGAgent",        useGnuPGAgent()        );
#endif
	config->writeEntry( "blinkedFilter",        blinkedFilter()        );
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
	config->writeEntry( "textUseProg",          isTextUseProg()        );
	config->writeEntry( "htmlUseProg",          isHtmlUseProg()        );
	config->writeEntry( "imageUseProg",         isImageUseProg()       );
	config->writeEntry( "animationUseProg",     isAnimationUseProg()   );
	config->writeEntry( "soundUseProg",         isSoundUseProg()       );
	config->writeEntry( "textProg",             textProg()             );
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
	KConfig *config = KGlobal::config();
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
	KConfig *config = KGlobal::config();
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

	if (Global::basketTree)
		Global::basketTree->relayoutAllBaskets();
}

/** SettingsDialog */

SettingsDialog::SettingsDialog(QWidget *parent)
 : KDialogBase(KDialogBase::IconList, i18n("Configure"),
			   KDialogBase::Default | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel, KDialogBase::Ok, parent, /*name=*/"ConfigureApplication")
{
	QFrame *page1 = addPage( i18n("General"),              QString::null, DesktopIcon("basket",            KIcon::SizeMedium) );
	QFrame *page3 = addPage( i18n("Notes Look"),           QString::null, DesktopIcon("colorize",          KIcon::SizeMedium) );
	QFrame *page4 = addPage( i18n("Applications"),         QString::null, DesktopIcon("run",               KIcon::SizeMedium) );
	QFrame *page6 = addPage( i18n("Addictive Features"),   QString::null, DesktopIcon("package_utilities", KIcon::SizeMedium) );

	/* Main Window page */
	QVBoxLayout *layout = new QVBoxLayout(page1, /*margin=*/0, KDialogBase::spacingHint());
	QHBoxLayout *hLay;
	QLabel      *label;

	m_useSystray = new QCheckBox(i18n("Doc&k in system tray"), page1);
	m_useSystray->setChecked(Settings::useSystray());
	layout->addWidget(m_useSystray);

	m_usePassivePopup = new QCheckBox(i18n("&Use balloons to report results of global actions"), page1);
	m_usePassivePopup->setChecked(Settings::usePassivePopup());
	layout->addWidget(m_usePassivePopup);

	m_playAnimations = new QCheckBox(i18n("Ani&mate changes in baskets"), page1);
	m_playAnimations->setChecked(Settings::playAnimations());
	layout->addWidget(m_playAnimations);

	m_showNotesToolTip = new QCheckBox(i18n("Sho&w tooltips in baskets"), page1);
	m_showNotesToolTip->setChecked(Settings::showNotesToolTip());
	layout->addWidget(m_showNotesToolTip);

	m_bigNotes = new QCheckBox(i18n("Bi&g notes"), page1);
	m_bigNotes->setChecked(Settings::bigNotes());
	layout->addWidget(m_bigNotes);

	hLay = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	m_exportTextTags = new QCheckBox(i18n("E&xport tags in texts"), page1);
	m_exportTextTags->setChecked(Settings::exportTextTags());

	QPixmap pixmapHelp(KGlobal::dirs()->findResource("data", "basket/images/tag_export_help.png"));
	QMimeSourceFactory::defaultFactory()->setPixmap("__resource_help_tag_export.png", pixmapHelp);
	HelpLabel *hLabel = new HelpLabel(
			i18n("When does this apply?"),
			i18n("<p>It does apply when you copy and paste, or drag and drop notes to a text editor.</p>"
			     "<p>If enabled, this property allows you to paste the tags as textual equivalents.<br>"
			     "For instance, a list of notes with the <b>To Do</b> and <b>Done</b> tags are exported as lines preceded by <b>[ ]</b> or <b>[x]</b>, "
			     "representing an empty  checkbox and a checked box.</p>") +
			"<p align='center'><img src=\"__resource_help_tag_export.png\"></p>",
			page1);
	hLay->addWidget(m_exportTextTags);
	hLay->addWidget(hLabel);
	hLay->addStretch();
	layout->addLayout(hLay);

#ifdef HAVE_LIBGPGME
	m_useGnuPGAgent = new QCheckBox(i18n("Use GnuPG agent for &password protected baskets"), page1);
	m_useGnuPGAgent->setChecked(Settings::useGnuPGAgent());
	layout->addWidget(m_useGnuPGAgent);

	if(KGpgMe::isGnuPGAgentAvailable())
	{
		m_useGnuPGAgent->setChecked(Settings::useGnuPGAgent());
	}
	else
	{
		m_useGnuPGAgent->setChecked(false);
		m_useGnuPGAgent->setEnabled(false);
	}
#endif
	QGridLayout *gl = new QGridLayout(layout, /*nRows=*/3, /*nCols=*/3);
	gl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 2);

	// Tabs bar Position:
	m_treeOnLeft = new QComboBox(page1);
	m_treeOnLeft->insertItem(i18n("On left"));
	m_treeOnLeft->insertItem(i18n("On right"));
	m_treeOnLeft->setCurrentItem( (int)!Settings::treeOnLeft() );
	QLabel *label1 = new QLabel(m_treeOnLeft, i18n("Bask&ets tree position:"), page1);
	gl->addWidget(label1,       0, 0);
	gl->addWidget(m_treeOnLeft, 0, 1);

	// Filter bar Position:
	m_filterOnTop = new QComboBox(page1);
	m_filterOnTop->insertItem(i18n("On top"));
	m_filterOnTop->insertItem(i18n("On bottom"));
	m_filterOnTop->setCurrentItem( (int)!Settings::filterOnTop() );
	QLabel *label9 = new QLabel(m_filterOnTop, i18n("&Filter bar position:"), page1);
	gl->addWidget(label9,        1, 0);
	gl->addWidget(m_filterOnTop, 1, 1);

	// New Notes Place:
	hLay = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	m_newNotesPlace = new QComboBox(page1);
	label = new QLabel(m_newNotesPlace, i18n("New notes &place:"), page1);
	m_newNotesPlace->insertItem(i18n("On top"));
	m_newNotesPlace->insertItem(i18n("On bottom"));
	m_newNotesPlace->insertItem(i18n("At current note"));
	m_newNotesPlace->setCurrentItem(Settings::newNotesPlace());
	hLay->addWidget(label);
	hLay->addWidget(m_newNotesPlace);
	hLay->addStretch();
	//layout->addLayout(hLay);
	label->hide();
	m_newNotesPlace->hide();

	// View File Content:
	QVButtonGroup *buttonGroup = new QVButtonGroup(i18n("View Content of Added Files for the Following Types"), page1);
	m_viewTextFileContent  = new QCheckBox( i18n("&Text"),               buttonGroup );
	m_viewHtmlFileContent  = new QCheckBox( i18n("&HTML page"),          buttonGroup );
	m_viewImageFileContent = new QCheckBox( i18n("&Image or animation"), buttonGroup );
	m_viewSoundFileContent = new QCheckBox( i18n("&Sound"),              buttonGroup );
	m_viewTextFileContent->setChecked(  Settings::viewTextFileContent()  );
	m_viewHtmlFileContent->setChecked(  Settings::viewHtmlFileContent()  );
	m_viewImageFileContent->setChecked( Settings::viewImageFileContent() );
	m_viewSoundFileContent->setChecked( Settings::viewSoundFileContent() );
	layout->addWidget(buttonGroup);

	// New Images Size:
	hLay = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	m_imgSizeX = new KIntNumInput(page1);
	m_imgSizeX->setMinValue(1);
	m_imgSizeX->setMaxValue(4096);
	m_imgSizeX->setReferencePoint(100);
	label = new QLabel(m_imgSizeX, i18n("&New images size:"), page1);
	hLay->addWidget(label);
	hLay->addWidget(m_imgSizeX);
	m_imgSizeY = new KIntNumInput(page1);
	m_imgSizeY->setMinValue(1);
	m_imgSizeY->setMaxValue(4096);
	m_imgSizeY->setReferencePoint(100);
	label = new QLabel(m_imgSizeY, i18n("&by"), page1);
	hLay->addWidget(label);
	hLay->addWidget(m_imgSizeY);
	label = new QLabel(i18n("pixels"), page1);
	hLay->addWidget(label);
	m_pushVisualize = new QPushButton(i18n("&Visualize..."), page1);
	hLay->addWidget(m_pushVisualize);
	hLay->addStretch();
	layout->addLayout(hLay);
	m_imgSizeX->setValue(         Settings::defImageX()         );
	m_imgSizeY->setValue(         Settings::defImageY()         );
	connect( m_pushVisualize, SIGNAL(clicked()), this, SLOT(visualize()) );

	layout->insertStretch(-1);

	/* Links page */
	QVBoxLayout *layout3 = new QVBoxLayout(page3, /*margin=*/0, KDialogBase::spacingHint());
	QLabel *label3 = new QLabel(i18n("Customize look of file, sound, link and launcher notes:"), page3);
	QTabWidget *tabs = new QTabWidget(page3);
	layout3->addWidget(label3);
	layout3->addWidget(tabs);

	m_soundLook       = new LinkLookEditWidget(LinkLook::soundLook,       i18n("Conference audio record"),                         "sound",       tabs);
	m_fileLook        = new LinkLookEditWidget(LinkLook::fileLook,        i18n("Annual report"),                                   "document",    tabs);
	m_localLinkLook   = new LinkLookEditWidget(LinkLook::localLinkLook,   i18n("Home folder"),                                     "folder_home", tabs);
	m_networkLinkLook = new LinkLookEditWidget(LinkLook::networkLinkLook,      "www.kde.org",        KMimeType::iconForURL("http://www.kde.org"), tabs);
	m_launcherLook    = new LinkLookEditWidget(LinkLook::launcherLook,    i18n("Launch %1").arg(kapp->aboutData()->programName()), "basket",      tabs);
	tabs->addTab(m_soundLook,       i18n("&Sounds")       );
	tabs->addTab(m_fileLook,        i18n("&Files")        );
	tabs->addTab(m_localLinkLook,   i18n("&Local Links")  );
	tabs->addTab(m_networkLinkLook, i18n("&Network Links"));
	tabs->addTab(m_launcherLook,    i18n("Launc&hers")    );

	/* Applications page */
	QVBoxLayout *layout4 = new QVBoxLayout(page4, /*margin=*/0, KDialogBase::spacingHint());

	m_textUseProg  = new QCheckBox(i18n("Open &text notes with a custom application:"), page4);
	m_textProg     = new RunCommandRequester(Settings::textProg(), i18n("Open text notes with:"), page4);
	m_textUseProg->setChecked(Settings::isTextUseProg());
	m_textProg->setEnabled(Settings::isTextUseProg());
	QHBoxLayout *hLayT = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	hLayT->insertSpacing(-1, 20);
	hLayT->addWidget(m_textProg);

	m_htmlUseProg  = new QCheckBox(i18n("Open &rich text notes with a custom application:"), page4);
	m_htmlProg     = new RunCommandRequester(Settings::htmlProg(), i18n("Open rich text notes with:"), page4);
	m_htmlUseProg->setChecked(Settings::isHtmlUseProg());
	m_htmlProg->setEnabled(Settings::isHtmlUseProg());
	QHBoxLayout *hLayH = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	hLayH->insertSpacing(-1, 20);
	hLayH->addWidget(m_htmlProg);

	m_imageUseProg = new QCheckBox(i18n("Open &image notes with a custom application:"), page4);
	m_imageProg    = new RunCommandRequester(Settings::imageProg(), i18n("Open image notes with:"), page4);
	m_imageUseProg->setChecked(Settings::isImageUseProg());
	m_imageProg->setEnabled(Settings::isImageUseProg());
	QHBoxLayout *hLayI = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	hLayI->insertSpacing(-1, 20);
	hLayI->addWidget(m_imageProg);

	m_animationUseProg = new QCheckBox(i18n("Open a&nimation notes with a custom application:"), page4);
	m_animationProg    = new RunCommandRequester(Settings::animationProg(), i18n("Open animation notes with:"), page4);
	m_animationUseProg->setChecked(Settings::isAnimationUseProg());
	m_animationProg->setEnabled(Settings::isAnimationUseProg());
	QHBoxLayout *hLayA = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	hLayA->insertSpacing(-1, 20);
	hLayA->addWidget(m_animationProg);

	m_soundUseProg = new QCheckBox(i18n("Open so&und notes with a custom application:"), page4);
	m_soundProg    = new RunCommandRequester(Settings::soundProg(), i18n("Open sound notes with:"), page4);
	m_soundUseProg->setChecked(Settings::isSoundUseProg());
	m_soundProg->setEnabled(Settings::isSoundUseProg());
	QHBoxLayout *hLayS = new QHBoxLayout(0L, /*margin=*/0, KDialogBase::spacingHint());
	hLayS->insertSpacing(-1, 20);
	hLayS->addWidget(m_soundProg);

	QString whatsthis = i18n(
		"<p>If checked, the application defined below will be used when opening that type of note.</p>"
		"<p>Otherwise, the application you've configured to in Konqueror will be used.</p>");

	QWhatsThis::add(m_textUseProg,      whatsthis);
	QWhatsThis::add(m_htmlUseProg,      whatsthis);
	QWhatsThis::add(m_imageUseProg,     whatsthis);
	QWhatsThis::add(m_animationUseProg, whatsthis);
	QWhatsThis::add(m_soundUseProg,     whatsthis);

	whatsthis = i18n(
		"<p>Define the application to use for opening that type of note instead of the "
		"application configured in Konqueror.</p>");

	QWhatsThis::add(m_textProg,      whatsthis);
	QWhatsThis::add(m_htmlProg,      whatsthis);
	QWhatsThis::add(m_imageProg,     whatsthis);
	QWhatsThis::add(m_animationProg, whatsthis);
	QWhatsThis::add(m_soundProg,     whatsthis);

	layout4->addWidget(m_textUseProg);
	layout4->addItem(hLayT);
	layout4->addWidget(m_htmlUseProg);
	layout4->addItem(hLayH);
	layout4->addWidget(m_imageUseProg);
	layout4->addItem(hLayI);
	layout4->addWidget(m_animationUseProg);
	layout4->addItem(hLayA);
	layout4->addWidget(m_soundUseProg);
	layout4->addItem(hLayS);

	layout4->addSpacing(KDialogBase::spacingHint());

	hLay = new QHBoxLayout(0L, /*margin=*/0, /*spacing=*/0);
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
		page4);
	hLay->addWidget(hl1);
	hLay->addStretch();
	layout4->addLayout(hLay);

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
		page4);
	hLay->addWidget(hl2);
	hLay->addStretch();
	layout4->addLayout(hLay);
	layout4->insertStretch(-1);

	connect( m_textUseProg,      SIGNAL(toggled(bool)), m_textProg,      SLOT(setEnabled(bool)) );
	connect( m_htmlUseProg,      SIGNAL(toggled(bool)), m_htmlProg,      SLOT(setEnabled(bool)) );
	connect( m_imageUseProg,     SIGNAL(toggled(bool)), m_imageProg,     SLOT(setEnabled(bool)) );
	connect( m_animationUseProg, SIGNAL(toggled(bool)), m_animationProg, SLOT(setEnabled(bool)) );
	connect( m_soundUseProg,     SIGNAL(toggled(bool)), m_soundProg,     SLOT(setEnabled(bool)) );

	/* Addicitive features */
	QVBoxLayout *layout6 = new QVBoxLayout(page6, /*margin=*/0, KDialogBase::spacingHint());

	layout6->addWidget( new QLabel(i18n("This page groups non-standard options that, once learned,\n"
	                                    "can be very powerful and time saving in the context of %1:")
	                                    .arg(kapp->aboutData()->programName()), page6) );

	m_groupOnInsertionLineWidget = new QWidget(page6);
	QHBoxLayout *hLayV = new QHBoxLayout(m_groupOnInsertionLineWidget, /*margin=*/0, KDialogBase::spacingHint());
	m_groupOnInsertionLine = new QCheckBox(i18n("&Group a new note when clicking on the right of the insertion line"), m_groupOnInsertionLineWidget);
	m_groupOnInsertionLine->setChecked(Settings::groupOnInsertionLine());
	QPixmap pixmap(KGlobal::dirs()->findResource("data", "basket/images/insertion_help.png"));
	QMimeSourceFactory::defaultFactory()->setPixmap("__resource_help_insertion_line.png", pixmap);
	HelpLabel *helpV = new HelpLabel(i18n("How to group a new note?"),
		i18n("<p>When this option is enabled, the insertion-line not only allows you to insert notes at the cursor position, but also allows you to group a new note with the one under the cursor:</p>") +
		"<p align='center'><img src=\"__resource_help_insertion_line.png\"></p>" +
		i18n("<p>Place your mouse between notes, where you want to add a new one.<br>"
		     "Click on the <b>left</b> of the insertion-line middle-mark to <b>insert</b> a note.<br>"
		     "Click on the <b>right</b> to <b>group</b> a note, with the one <b>below or above</b>, depending on where your mouse is.</p>"), m_groupOnInsertionLineWidget);
	hLayV->addWidget(m_groupOnInsertionLine);
	hLayV->addWidget(helpV);
	hLayV->insertStretch(-1);
	layout6->addWidget(m_groupOnInsertionLineWidget);

	QGridLayout *ga = new QGridLayout(layout6, /*nRows=*/3, /*nCols=*/4);
	ga->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 3);

	m_middleAction = new QComboBox(page6);
	m_middleAction->insertItem( i18n("Do nothing")            );
	m_middleAction->insertItem( i18n("Paste clipboard")       );
	m_middleAction->insertItem( i18n("Insert text note")      );
	m_middleAction->insertItem( i18n("Insert rich text note") );
	m_middleAction->insertItem( i18n("Insert image note")     );
	m_middleAction->insertItem( i18n("Insert link note")      );
	m_middleAction->insertItem( i18n("Insert launcher note")  );
	m_middleAction->insertItem( i18n("Insert color note")     );
	m_middleAction->setCurrentItem(Settings::middleAction());
	QLabel *labelM = new QLabel(m_middleAction, i18n("&Shift+middle-click:"), page6);
	ga->addWidget(labelM,                                         0, 0);
	ga->addWidget(m_middleAction,                                 0, 1);
	ga->addWidget(new QLabel(i18n("at cursor position"), page6),  0, 2);

	QGroupBox *gbSys = new QGroupBox(3, Qt::Vertical, i18n("System Tray Icon"), page6);
	layout6->addWidget(gbSys);

	m_systray = new QWidget(gbSys);
	QVBoxLayout *sysLay = new QVBoxLayout(m_systray, /*margin=*/0, KDialogBase::spacingHint());

	m_showIconInSystray = new QCheckBox(i18n("Show c&urrent basket icon in system tray icon"), m_systray);
	sysLay->addWidget(m_showIconInSystray);

	QGridLayout *gs = new QGridLayout(sysLay, /*nRows=*/2, /*nCols=*/3);
	gs->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 2);

	m_hideOnMouseOut = new QCheckBox(i18n("H&ide main window when mouse goes out of it for"), m_systray);
	m_timeToHideOnMouseOut = new KIntNumInput(Settings::timeToHideOnMouseOut(), m_systray);
	m_timeToHideOnMouseOut->setRange(0, 600, 1, false);
	m_timeToHideOnMouseOut->setSuffix(i18n(" tenths of seconds"));
	gs->addWidget(m_hideOnMouseOut,       0, 0);
	gs->addWidget(m_timeToHideOnMouseOut, 0, 1);

	m_showOnMouseIn  = new QCheckBox(i18n("Show &main window when mouse hovers over the system tray icon for"), m_systray);
	m_timeToShowOnMouseIn = new KIntNumInput(Settings::timeToShowOnMouseIn(), m_systray);
	m_timeToShowOnMouseIn->setRange(0, 600, 1, false);
	m_timeToShowOnMouseIn->setSuffix(i18n(" tenths of seconds"));
	gs->addWidget(m_showOnMouseIn,       1, 0);
	gs->addWidget(m_timeToShowOnMouseIn, 1, 1);

	layout6->insertStretch(-1);

	connect( m_useSystray,     SIGNAL(toggled(bool)), gbSys,                  SLOT(setEnabled(bool)) );
	connect( m_hideOnMouseOut, SIGNAL(toggled(bool)), m_timeToHideOnMouseOut, SLOT(setEnabled(bool)) );
	connect( m_showOnMouseIn,  SIGNAL(toggled(bool)), m_timeToShowOnMouseIn,  SLOT(setEnabled(bool)) );

	m_showIconInSystray->setChecked(    Settings::showIconInSystray()  );
	m_hideOnMouseOut->setChecked(       Settings::hideOnMouseOut()     );
	m_showOnMouseIn->setChecked(        Settings::showOnMouseIn()      );

	m_systray->setEnabled(              Settings::useSystray()         );
	m_timeToHideOnMouseOut->setEnabled( Settings::hideOnMouseOut()     );
	m_timeToShowOnMouseIn->setEnabled(  Settings::showOnMouseIn()      );
}

SettingsDialog::~SettingsDialog()
{
}

/*
What is the Selection? Help:
	QString help = "<p>" + i18n(
		"When you select a text in any application, it is copied in a special clipboard named "
		"'Selection' that you can paste in any other application by simply pressing the middle mouse button.") + "</p><p>";
		help += i18n("%1 let you to copy a note to the selection by just "
		             "double-clicking its handle, allowing quick pastes with the mouse.").arg(kapp->aboutData()->programName());
	help += " " + i18n("If you aren't used to the selection, you can copy the note to the normal clipboard.") + "</p>";
*/

void SettingsDialog::visualize()
{
	ViewSizeDialog size(this, m_imgSizeX->value(), m_imgSizeY->value());
	size.exec();
	m_imgSizeX->setValue(size.width());
	m_imgSizeY->setValue(size.height());
}

void SettingsDialog::slotDefault() // TODO!
{
	m_imgSizeX->setValue(300);
	m_imgSizeY->setValue(200);

	KDialogBase::slotDefault();
}

void SettingsDialog::slotApply()
{
	Settings::setTreeOnLeft(           ! m_treeOnLeft->currentItem()       );
	Settings::setFilterOnTop(          ! m_filterOnTop->currentItem()      );
	Settings::setPlayAnimations(       m_playAnimations->isChecked()       );
	Settings::setShowNotesToolTip(     m_showNotesToolTip->isChecked()     );
	Settings::setBigNotes(             m_bigNotes->isChecked()             );
	Settings::setExportTextTags(       m_exportTextTags->isChecked()       );
	Settings::setUseGnuPGAgent(        m_useGnuPGAgent->isChecked()        );
	Settings::setUsePassivePopup(      m_usePassivePopup->isChecked()      );
	Settings::setMiddleAction(         m_middleAction->currentItem()       );
	Settings::setGroupOnInsertionLine( m_groupOnInsertionLine->isChecked() );

	Settings::setNewNotesPlace(        m_newNotesPlace->currentItem()      );
	Settings::setViewTextFileContent(  m_viewTextFileContent->isChecked()  );
	Settings::setViewHtmlFileContent(  m_viewHtmlFileContent->isChecked()  );
	Settings::setViewImageFileContent( m_viewImageFileContent->isChecked() );
	Settings::setViewSoundFileContent( m_viewSoundFileContent->isChecked() );

	Settings::setUseSystray(           m_useSystray->isChecked()           );
	Settings::setShowIconInSystray(    m_showIconInSystray->isChecked()    );
	Settings::setHideOnMouseOut(       m_hideOnMouseOut->isChecked()       );
	Settings::setTimeToHideOnMouseOut( m_timeToHideOnMouseOut->value()     );
	Settings::setShowOnMouseIn(        m_showOnMouseIn->isChecked()        );
	Settings::setTimeToShowOnMouseIn(  m_timeToShowOnMouseIn->value()      );

	Settings::setDefImageX(            m_imgSizeX->value()                 );
	Settings::setDefImageY(            m_imgSizeY->value()                 );

	Settings::setIsTextUseProg(        m_textUseProg->isChecked()          );
	Settings::setIsHtmlUseProg(        m_htmlUseProg->isChecked()          );
	Settings::setIsImageUseProg(       m_imageUseProg->isChecked()         );
	Settings::setIsAnimationUseProg(   m_animationUseProg->isChecked()     );
	Settings::setIsSoundUseProg(       m_soundUseProg->isChecked()         );
	Settings::setTextProg(             m_textProg->runCommand()            );
	Settings::setHtmlProg(             m_htmlProg->runCommand()            );
	Settings::setImageProg(            m_imageProg->runCommand()           );
	Settings::setAnimationProg(        m_animationProg->runCommand()       );
	Settings::setSoundProg(            m_soundProg->runCommand()           );

	m_soundLook->saveChanges();
	m_fileLook->saveChanges();
	m_localLinkLook->saveChanges();
	m_networkLinkLook->saveChanges();
	m_launcherLook->saveChanges();
	Global::basketTree->linkLookChanged();

	Settings::saveConfig();

	KDialogBase::slotApply();
}

void SettingsDialog::slotOk()
{
	slotApply();
	KDialogBase::slotOk();
}

#include "settings.moc"
