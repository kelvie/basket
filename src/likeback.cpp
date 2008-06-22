/***************************************************************************
 *   Copyright (C) 2006 by Sebastien Laout                                 *
 *   slaout@linux62.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.         *
 ***************************************************************************/

#include <kapplication.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kaction.h>
#include <kicon.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <QPixmap>
#include <Q3VBoxLayout>
#include <kpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <q3vgroupbox.h>
#include <kguiitem.h>
#include <q3popupmenu.h>
#include <q3textedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <KDialog>
#include <q3http.h>
#include <kurl.h>
#include <kinputdialog.h>
#include <qvalidator.h>
#include <qaction.h>
#include <kdebug.h>

#include <pwd.h>

#include <kglobal.h>

#include "likeback.h"
#include "likeback_p.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <kuser.h>
#include <QDesktopWidget>
/****************************************/
/********** class LikeBackBar: **********/
/****************************************/

LikeBackBar::LikeBackBar(LikeBack *likeBack)
 : QWidget(0, "LikeBackBar", Qt::WX11BypassWM | Qt::WStyle_NoBorder | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop | Qt::WStyle_NoBorder | Qt::WGroupLeader)
 , m_likeBack(likeBack)
{
	Q3HBoxLayout *layout = new Q3HBoxLayout(this);

	QIcon likeIconSet    = KIcon("likeback_like");
	QIcon dislikeIconSet = KIcon("likeback_dislike");
	QIcon bugIconSet     = KIcon("likeback_bug");
	QIcon featureIconSet = KIcon("likeback_feature");

	m_likeButton = new QToolButton(this, "likeback_like");
	m_likeButton->setIconSet(likeIconSet);
	m_likeButton->setTextLabel("<p>" + i18n("Send application developers a comment about something you like"));
	m_likeButton->setAutoRaise(true);
	connect( m_likeButton, SIGNAL(clicked()), this, SLOT(clickedLike()) );
	layout->add(m_likeButton);

	m_dislikeButton = new QToolButton(this, "likeback_dislike");
	m_dislikeButton->setIconSet(dislikeIconSet);
	m_dislikeButton->setTextLabel("<p>" + i18n("Send application developers a comment about something you dislike"));
	m_dislikeButton->setAutoRaise(true);
	connect( m_dislikeButton, SIGNAL(clicked()), this, SLOT(clickedDislike()) );
	layout->add(m_dislikeButton);

	m_bugButton = new QToolButton(this, "likeback_bug");
	m_bugButton->setIconSet(bugIconSet);
	m_bugButton->setTextLabel("<p>" + i18n("Send application developers a comment about an improper behavior of the application"));
	m_bugButton->setAutoRaise(true);
	connect( m_bugButton, SIGNAL(clicked()), this, SLOT(clickedBug()) );
	layout->add(m_bugButton);

	m_featureButton = new QToolButton(this, "likeback_feature");
	m_featureButton->setIconSet(featureIconSet);
	m_featureButton->setTextLabel("<p>" + i18n("Send application developers a comment about a new feature you desire"));
	m_featureButton->setAutoRaise(true);
	connect( m_featureButton, SIGNAL(clicked()), this, SLOT(clickedFeature()) );
	layout->add(m_featureButton);

	connect( &m_timer, SIGNAL(timeout()), this, SLOT(autoMove()) );

	LikeBack::Button buttons = likeBack->buttons();
	m_likeButton->setShown(    buttons & LikeBack::Like    );
	m_dislikeButton->setShown( buttons & LikeBack::Dislike );
	m_bugButton->setShown(     buttons & LikeBack::Bug     );
	m_featureButton->setShown( buttons & LikeBack::Feature );
}

LikeBackBar::~LikeBackBar()
{
}

void LikeBackBar::startTimer()
{
	m_timer.start(10);
}

void LikeBackBar::stopTimer()
{
	m_timer.stop();
}

void LikeBackBar::autoMove()
{
	static QWidget *lastWindow = 0;

	QWidget *window = kapp->activeWindow();
	// When a Kicker applet has the focus, like the Commandline QLineEdit,
	// the systemtray icon indicates to be the current window and the LikeBack is shown next to the system tray icon.
	// It's obviously bad ;-) :
	bool shouldShow = (m_likeBack->userWantsToShowBar() && m_likeBack->enabledBar() && window && !window->inherits("KSystemTray"));
	if (shouldShow) {
		//move(window->x() + window->width() - 100 - width(), window->y());
		//move(window->x() + window->width() - 100 - width(), window->mapToGlobal(QPoint(0, 0)).y() - height());
		move(window->mapToGlobal(QPoint(0, 0)).x() + window->width() - width(), window->mapToGlobal(QPoint(0, 0)).y() + 1);

		if (window != lastWindow && m_likeBack->windowNamesListing() != LikeBack::NoListing) {
			if (qstricmp(window->name(), "") == 0 || qstricmp(window->name(), "unnamed") == 0) {
				kDebug() << "===== LikeBack ===== UNNAMED ACTIVE WINDOW OF TYPE " << window->className() << " ======" << LikeBack::activeWindowPath();
			} else if (m_likeBack->windowNamesListing() == LikeBack::AllWindows) {
				kDebug() << "LikeBack: Active Window: " << LikeBack::activeWindowPath();
			}
		}
		lastWindow = window;
	}

	// Show or hide the bar accordingly:
	if (shouldShow && !isShown()) {
		show();
	} else if (!shouldShow && isShown()) {
		hide();
	}
}

void LikeBackBar::clickedLike()
{
	m_likeBack->execCommentDialog(LikeBack::Like);
}

void LikeBackBar::clickedDislike()
{
	m_likeBack->execCommentDialog(LikeBack::Dislike);
}

void LikeBackBar::clickedBug()
{
	m_likeBack->execCommentDialog(LikeBack::Bug);
}

void LikeBackBar::clickedFeature()
{
	m_likeBack->execCommentDialog(LikeBack::Feature);
}

/********************************************/
/********** class LikeBackPrivate: **********/
/********************************************/

LikeBackPrivate::LikeBackPrivate()
 : bar(0)
 , config(0)
 , aboutData(0)
 , buttons(LikeBack::DefaultButtons)
 , hostName()
 , remotePath()
 , hostPort(80)
 , acceptedLocales()
 , acceptedLanguagesMessage()
 , windowListing(LikeBack::NoListing)
 , showBar(false)
 , disabledCount(0)
 , fetchedEmail()
 , action(0)
{
}

LikeBackPrivate::~LikeBackPrivate()
{
	delete bar;
	delete action;

	config = 0;
	aboutData = 0;
}

/*************************************/
/********** class LikeBack: **********/
/*************************************/

LikeBack::LikeBack(Button buttons, bool showBarByDefault, KConfig *config, const KAboutData *aboutData)
 : QObject()
{
	// Initialize properties (1/2):
	d = new LikeBackPrivate();
	d->buttons          = buttons;
	d->config           = config;
	d->aboutData        = aboutData;
	d->showBarByDefault = showBarByDefault;

	// Use default KApplication config and aboutData if not provided:
	if (d->config == 0)
		d->config = KGlobal::config().data();
	if (d->aboutData == 0)
		d->aboutData = KGlobal::mainComponent().aboutData();

	// Initialize properties (2/2) [Needs aboutData to be set]:
	d->showBar          = userWantsToShowBar();

	// Fetch the KControl user email address as a default one:
	if (!emailAddressAlreadyProvided())
		fetchUserEmail();

	// Initialize the button-bar:
	d->bar = new LikeBackBar(this);
	d->bar->resize(d->bar->sizeHint());

	// Show the information message if it is the first time, and if the button-bar is shown:
	static const char *messageShown = "LikeBack_starting_information";
	if (d->showBar && KMessageBox::shouldBeShownContinue(messageShown)) {
		showInformationMessage();
		KMessageBox::saveDontShowAgainContinue(messageShown);
	}

	// Show the bar if that's wanted by the developer or the user:
	if (d->showBar)
		QTimer::singleShot( 0, d->bar, SLOT(startTimer()) );

#if 0
	disableBar();
	// Alex: Oh, it drove me nuts
	d->buttons = (Button) (                             0); showInformationMessage();
	d->buttons = (Button) (                       Feature); showInformationMessage();
	d->buttons = (Button) (                 Bug          ); showInformationMessage();
	d->buttons = (Button) (                 Bug | Feature); showInformationMessage();
	d->buttons = (Button) (       Dislike                ); showInformationMessage();
	d->buttons = (Button) (       Dislike       | Feature); showInformationMessage();
	d->buttons = (Button) (       Dislike | Bug          ); showInformationMessage();
	d->buttons = (Button) (       Dislike | Bug | Feature); showInformationMessage();
	d->buttons = (Button) (Like                          ); showInformationMessage();
	d->buttons = (Button) (Like                 | Feature); showInformationMessage();
	d->buttons = (Button) (Like           | Bug          ); showInformationMessage();
	d->buttons = (Button) (Like           | Bug | Feature); showInformationMessage();
	d->buttons = (Button) (Like | Dislike                ); showInformationMessage();
	d->buttons = (Button) (Like | Dislike       | Feature); showInformationMessage();
	d->buttons = (Button) (Like | Dislike | Bug          ); showInformationMessage();
	d->buttons = (Button) (Like | Dislike | Bug | Feature); showInformationMessage();
	enableBar();
#endif
}

LikeBack::~LikeBack()
{
	delete d;
}

void LikeBack::setWindowNamesListing(WindowListing windowListing)
{
	d->windowListing = windowListing;
}

LikeBack::WindowListing LikeBack::windowNamesListing()
{
	return d->windowListing;
}

void LikeBack::setAcceptedLanguages(const QStringList &locales, const QString &message)
{
	d->acceptedLocales          = locales;
	d->acceptedLanguagesMessage = message;
}

QStringList LikeBack::acceptedLocales()
{
	return d->acceptedLocales;
}

QString LikeBack::acceptedLanguagesMessage()
{
	return d->acceptedLanguagesMessage;
}

void LikeBack::setServer(const QString &hostName, const QString &remotePath, quint16 hostPort)
{
	d->hostName   = hostName;
	d->remotePath = remotePath;
	d->hostPort   = hostPort;
}

QString LikeBack::hostName()
{
	return d->hostName;
}

QString LikeBack::remotePath()
{
	return d->remotePath;
}

quint16 LikeBack::hostPort()
{
	return d->hostPort;
}

void LikeBack::disableBar()
{
	d->disabledCount++;
	if (d->bar && d->disabledCount > 0) {
		d->bar->hide();
		d->bar->stopTimer();
	}
}

void LikeBack::enableBar()
{
	d->disabledCount--;
	if (d->disabledCount < 0)
		kDebug() << "===== LikeBack ===== Enabled more times than it was disabled. Please refer to the disableBar() documentation for more information and hints.";
	if (d->bar && d->disabledCount <= 0) {
		d->bar->startTimer();
	}
}

bool LikeBack::enabledBar()
{
	return d->disabledCount <= 0;
}

void LikeBack::execCommentDialog(Button type, const QString &initialComment, const QString &windowPath, const QString &context)
{
	disableBar();
	LikeBackDialog dialog(type, initialComment, windowPath, context, this);
	dialog.exec();
	enableBar();
}

void LikeBack::execCommentDialogFromHelp()
{
	execCommentDialog(AllButtons, /*initialComment=*/"", /*windowPath=*/"HelpMenuAction");
}

LikeBack::Button LikeBack::buttons()
{
	return d->buttons;
}

const KAboutData* LikeBack::aboutData()
{
	return d->aboutData;
}

KConfig* LikeBack::config()
{
	return d->config;
}

KAction* LikeBack::sendACommentAction(KActionCollection *parent)
{
    if (d->action == 0)
    {
	d->action = parent->addAction("likeback_send_a_comment", this,
				      SLOT(execCommentDialog()));
	d->action->setText(i18n("&Send a Comment to Developers"));
	d->action->setIcon(KIcon("mail_new"));
    }

    return d->action;
}

bool LikeBack::userWantsToShowBar()
{
	// Store the button-bar per version, so it can be disabled by the developer for the final version:
	KConfigGroup configGroup = KGlobal::config()->group("LikeBack");
	return configGroup.readEntry("userWantToShowBarForVersion_" + d->aboutData->version(), d->showBarByDefault);
}

void LikeBack::setUserWantsToShowBar(bool showBar)
{
	if (showBar == d->showBar)
		return;

	d->showBar = showBar;

	// Store the button-bar per version, so it can be disabled by the developer for the final version:
	KConfigGroup configGroup = KGlobal::config()->group("LikeBack");
	configGroup.writeEntry("userWantToShowBarForVersion_" + d->aboutData->version(), showBar);
	configGroup.sync(); // Make sure the option is saved, even if the application crashes after that.

	if (showBar)
		d->bar->startTimer();
}

void LikeBack::showInformationMessage()
{
	// Load and register the images needed by the message:
	QPixmap likeIcon    = KIcon("likeback_like").pixmap(32,32);
	QPixmap dislikeIcon = KIcon("likeback_dislike").pixmap(32,32);
	QPixmap bugIcon     = KIcon("likeback_bug").pixmap(32,32);
	QPixmap featureIcon = KIcon("likeback_feature").pixmap(32,32);
	Q3MimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_like",    likeIcon);
	Q3MimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_dislike", dislikeIcon);
	Q3MimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_bug",     bugIcon);
	Q3MimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_feature", featureIcon);

	// Show a message reflecting the allowed types of comment:
	Button buttons = d->buttons;
	int nbButtons = (buttons & Like    ? 1 : 0) +
	                (buttons & Dislike ? 1 : 0) +
	                (buttons & Bug     ? 1 : 0) +
	                (buttons & Feature ? 1 : 0);
	KMessageBox::information(0,
		"<p><b>" + (isDevelopmentVersion(d->aboutData->version()) ?
			i18n("Welcome to this testing version of %1.") :
			i18n("Welcome to %1.")
		).arg(d->aboutData->programName()) + "</b></p>"
		"<p>" + i18n("To help us improve it, your comments are important.") + "</p>"
		"<p>" +
			((buttons & LikeBack::Like) && (buttons & LikeBack::Dislike) ?
				i18n("Each time you have a great or frustrating experience, "
				     "please click the appropriate face below the window title-bar, "
				     "briefly describe what you like or dislike and click Send.")
			: (buttons & LikeBack::Like ?
				i18n("Each time you have a great experience, "
				     "please click the smiling face below the window title-bar, "
				     "briefly describe what you like and click Send.")
			: (buttons & LikeBack::Dislike ?
				i18n("Each time you have a frustrating experience, "
				     "please click the frowning face below the window title-bar, "
				     "briefly describe what you dislike and click Send.")
			:
				QString()
			))) + "</p>" +
		(buttons & LikeBack::Bug ?
			"<p>" +
				(buttons & (LikeBack::Like | LikeBack::Dislike) ?
					i18n("Follow the same principle to quickly report a bug: "
						"just click the broken-object icon in the top-right corner of the window, describe it and click Send.")
				:
					i18n("Each time you discover a bug in the application, "
						"please click the broken-object icon below the window title-bar, "
						"briefly describe the mis-behaviour and click Send.")
				) + "</p>"
			: "") +
		"<p>" + i18np("Example:", "Examples:", nbButtons) + "</p>" +
		(buttons & LikeBack::Like ?
			"<p><img source=\"likeback_icon_like\"> &nbsp;" +
				i18n("<b>I like</b> the new artwork. Very refreshing.") + "</p>"
			: "") +
		(buttons & LikeBack::Dislike ?
			"<p><img source=\"likeback_icon_dislike\"> &nbsp;" +
				i18n("<b>I dislike</b> the welcome page of that assistant. Too time consuming.") + "</p>"
			: "") +
		(buttons & LikeBack::Bug ?
			"<p><img source=\"likeback_icon_bug\"> &nbsp;" +
				i18n("<b>The application has an improper behaviour</b> when clicking the Add button. Nothing happens.") + "</p>"
			: "") +
		(buttons & LikeBack::Feature ?
			"<p><img source=\"likeback_icon_feature\"> &nbsp;" +
				i18n("<b>I desire a new feature</b> allowing me to send my work by email.") + "</p>"
			: "") +
		"</tr></table>",
		i18n("Help Improve the Application"));

	// Reset the images from the factory:
	Q3MimeSourceFactory::defaultFactory()->setData("likeback_icon_like",    0L);
	Q3MimeSourceFactory::defaultFactory()->setData("likeback_icon_dislike", 0L);
	Q3MimeSourceFactory::defaultFactory()->setData("likeback_icon_bug",     0L);
	Q3MimeSourceFactory::defaultFactory()->setData("likeback_icon_feature", 0L);
}

QString LikeBack::activeWindowPath()
{
	// Compute the window hierarchy (from the latest to the oldest):
	QStringList windowNames;
	QWidget *window = kapp->activeWindow();
	while (window) {
		QString name = window->name();
		// Append the class name to the window name if it is unnamed:
		if (name == "unnamed")
			name += QString(":") + window->className();
		windowNames.append(name);
		window = dynamic_cast<QWidget*>(window->parent());
	}

	// Create the string of windows starting by the end (from the oldest to the latest):
	QString windowPath;
	for (int i = ((int)windowNames.count()) - 1; i >= 0; i--) {
		if (windowPath.isEmpty())
			windowPath = windowNames[i];
		else
			windowPath += QString("~~") + windowNames[i];
	}

	// Finally return the computed path:
	return windowPath;
}

bool LikeBack::emailAddressAlreadyProvided()
{
	KConfigGroup configGroup = KGlobal::config()->group("LikeBack");
	return configGroup.readEntry("emailAlreadyAsked", false);
}

QString LikeBack::emailAddress()
{
	if (!emailAddressAlreadyProvided())
		askEmailAddress();

	KConfigGroup configGroup = KGlobal::config()->group("LikeBack");
	return configGroup.readEntry("emailAddress", "");
}

void LikeBack::setEmailAddress(const QString &address, bool userProvided)
{
	KConfigGroup configGroup = KGlobal::config()->group("LikeBack");
	configGroup.writeEntry("emailAddress",      address);
	configGroup.writeEntry("emailAlreadyAsked", userProvided || emailAddressAlreadyProvided());
	configGroup.sync(); // Make sure the option is saved, even if the application crashes after that.
}

void LikeBack::askEmailAddress()
{
	KConfigGroup configGroup = KGlobal::config()->group("LikeBack");

	QString currentEmailAddress = configGroup.readEntry("emailAddress", "");
	if (!emailAddressAlreadyProvided() && !d->fetchedEmail.isEmpty())
		currentEmailAddress = d->fetchedEmail;

	bool ok;

	QString emailExpString = "[\\w-\\.]+@[\\w-\\.]+\\.[\\w]+";
	//QString namedEmailExpString = "[.]*[ \\t]+<" + emailExpString + '>';
	//QRegExp emailExp("^(|" + emailExpString + '|' + namedEmailExpString + ")$");
	QRegExp emailExp("^(|" + emailExpString + ")$");
	QRegExpValidator emailValidator(emailExp, this);

	disableBar();
	QString email = KInputDialog::getText(
		i18n("Email Address"),
		"<p><b>" + i18n("Please provide your email address.") + "</b></p>" +
		"<p>" + i18n("It will only be used to contact you back if more information is needed about your comments, ask you how to reproduce the bugs you report, send bug corrections for you to test, etc.") + "</p>" +
		"<p>" + i18n("The email address is optional. If you do not provide any, your comments will be sent anonymously.") + "</p>",
		currentEmailAddress, &ok, kapp->activeWindow(), &emailValidator);
	enableBar();

	if (ok)
		setEmailAddress(email);
}

// FIXME: Should be moved to KAboutData? Cigogne will also need it.
bool LikeBack::isDevelopmentVersion(const QString &version)
{
	return version.find("alpha", /*index=*/0, /*caseSensitive=*/false) != -1 ||
	       version.find("beta",  /*index=*/0, /*caseSensitive=*/false) != -1 ||
	       version.find("rc",    /*index=*/0, /*caseSensitive=*/false) != -1 ||
	       version.find("svn",   /*index=*/0, /*caseSensitive=*/false) != -1 ||
	       version.find("cvs",   /*index=*/0, /*caseSensitive=*/false) != -1;
}

/**
 * Code from KBugReport::slotConfigureEmail() in kdeui/kbugreport.cpp:
 */
/*void LikeBack::beginFetchingEmail()
{
	if (m_process)
		return;
	m_process = new K3Process();
	*m_process << QString::fromLatin1("kcmshell") << QString::fromLatin1("kcm_useraccount");
	connect( m_process, SIGNAL(processExited(K3Process*)), SLOT(fetchUserEmail()) );
	if (!m_process->start()) {
		kDebug() << "Couldn't start kcmshell.." << endl;
		delete m_process;
		m_process = 0;
		return;
	}
//	m_configureEmail->setEnabled(false);
}*/

/**
 * Code from KBugReport::slotSetFrom() in kdeui/kbugreport.cpp:
 */
void LikeBack::fetchUserEmail()
{
//	delete m_process;
//	m_process = 0;
//	m_configureEmail->setEnabled(true);

	// ### KDE4: why oh why is KEmailSettings in kio?
	KConfig emailConf( QString::fromLatin1("emaildefaults") );

	// find out the default profile
	KConfigGroup configGroup = KConfigGroup(&emailConf,QString::fromLatin1("Defaults"));
	QString profile = QString::fromLatin1("PROFILE_");
	profile += configGroup.readEntry(QString::fromLatin1("Profile"), QString::fromLatin1("Default"));

	configGroup = KConfigGroup(&emailConf,profile);
	QString fromaddr = configGroup.readEntry(QString::fromLatin1("EmailAddress"));
	if (fromaddr.isEmpty()) {
		KUser userInfo;
		d->fetchedEmail = userInfo.property(KUser::FullName).toString();
	} else {
		QString name = configGroup.readEntry(QString::fromLatin1("FullName"));
		if (!name.isEmpty())
			d->fetchedEmail = /*name + QString::fromLatin1(" <") +*/ fromaddr /*+ QString::fromLatin1(">")*/;
	}
//	m_from->setText( fromaddr );
}

/*******************************************/
/********** class LikeBackDialog: **********/
/*******************************************/

LikeBackDialog::LikeBackDialog(LikeBack::Button reason, const QString &initialComment, const QString &windowPath, const QString &context, LikeBack *likeBack)
 : KDialog(kapp->activeWindow())
 , m_likeBack(likeBack)
 , m_windowPath(windowPath)
 , m_context(context)
{
	// KDialog Options
	setCaption(i18n("Send a Comment to Developers"));
	setButtons(Ok | Cancel | Default);
	setDefaultButton(Ok);
	setParent(kapp->activeWindow());
	setObjectName("_likeback_feedback_window_");
	setModal(true);
	showButtonSeparator(true);
	connect(this, SIGNAL(okClicked()), SLOT(slotOk()));
	connect(this, SIGNAL(defaultClicked()), SLOT(slotDefault()));

	// If no specific "reason" is provided, choose the first one:
	if (reason == LikeBack::AllButtons) {
		LikeBack::Button buttons = m_likeBack->buttons();
		int firstButton = 0;
		if (firstButton == 0 && (buttons & LikeBack::Like))    firstButton = LikeBack::Like;
		if (firstButton == 0 && (buttons & LikeBack::Dislike)) firstButton = LikeBack::Dislike;
		if (firstButton == 0 && (buttons & LikeBack::Bug))     firstButton = LikeBack::Bug;
		if (firstButton == 0 && (buttons & LikeBack::Feature)) firstButton = LikeBack::Feature;
		reason = (LikeBack::Button) firstButton;
	}

	// If no window path is provided, get the current active window path:
	if (m_windowPath.isEmpty())
		m_windowPath = LikeBack::activeWindowPath();

	QWidget *page = new QWidget(this);
	Q3VBoxLayout *pageLayout = new Q3VBoxLayout(page, /*margin=*/0, spacingHint());

	// The introduction message:
	QLabel *introduction = new QLabel(introductionText(), page);
	pageLayout->addWidget(introduction);

	// The comment group:
	m_group = new Q3ButtonGroup(0);//i18n("Send Application Developers a Comment About:"), page);
	Q3VGroupBox *box = new Q3VGroupBox(i18n("Send Application Developers a Comment About:"), page);
	pageLayout->addWidget(box);

	// The radio buttons:
	QWidget *buttons = new QWidget(box);
	Q3GridLayout *buttonsGrid = new Q3GridLayout(buttons, /*nbRows=*/4, /*nbColumns=*/2, /*margin=*/0, spacingHint());
	if (m_likeBack->buttons() & LikeBack::Like) {
		QPixmap likePixmap = KIconLoader::global()->loadIcon(
            "likeback_like", KIconLoader::NoGroup, 16,
            KIconLoader::DefaultState, QStringList(), 0L, true
            );
		QLabel *likeIcon = new QLabel(buttons);
		likeIcon->setPixmap(likePixmap);
		likeIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		QRadioButton *likeButton = new QRadioButton(i18n("Something you &like"), buttons);
		buttonsGrid->addWidget(likeIcon,   /*row=*/0, /*column=*/0);
		buttonsGrid->addWidget(likeButton, /*row=*/0, /*column=*/1);
		m_group->insert(likeButton, LikeBack::Like);
	}
	if (m_likeBack->buttons() & LikeBack::Dislike) {
		QPixmap dislikePixmap = KIconLoader::global()->loadIcon(
            "likeback_dislike", KIconLoader::NoGroup, 16,
            KIconLoader::DefaultState, QStringList(), 0L, true
            );
		QLabel *dislikeIcon = new QLabel(buttons);
		dislikeIcon->setPixmap(dislikePixmap);
		dislikeIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		QRadioButton *dislikeButton = new QRadioButton(i18n("Something you &dislike"), buttons);
		buttonsGrid->addWidget(dislikeIcon,   /*row=*/1, /*column=*/0);
		buttonsGrid->addWidget(dislikeButton, /*row=*/1, /*column=*/1);
		m_group->insert(dislikeButton, LikeBack::Dislike);
	}
	if (m_likeBack->buttons() & LikeBack::Bug) {
		QPixmap bugPixmap = KIconLoader::global()->loadIcon(
            "likeback_bug", KIconLoader::NoGroup, 16, KIconLoader::DefaultState,
            QStringList(), 0L, true
            );
		QLabel *bugIcon = new QLabel(buttons);
		bugIcon->setPixmap(bugPixmap);
		bugIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		QRadioButton *bugButton = new QRadioButton(i18n("An improper &behavior of this application"), buttons);
		buttonsGrid->addWidget(bugIcon,   /*row=*/2, /*column=*/0);
		buttonsGrid->addWidget(bugButton, /*row=*/2, /*column=*/1);
		m_group->insert(bugButton, LikeBack::Bug);
	}
	if (m_likeBack->buttons() & LikeBack::Feature) {
		QPixmap featurePixmap = KIconLoader::global()->loadIcon(
            "likeback_feature", KIconLoader::NoGroup, 16,
            KIconLoader::DefaultState, QStringList(), 0L, true);
		QLabel *featureIcon = new QLabel(buttons);
		featureIcon->setPixmap(featurePixmap);
		featureIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		QRadioButton *featureButton = new QRadioButton(i18n("A new &feature you desire"), buttons);
		buttonsGrid->addWidget(featureIcon,   /*row=*/3, /*column=*/0);
		buttonsGrid->addWidget(featureButton, /*row=*/3, /*column=*/1);
		m_group->insert(featureButton, LikeBack::Feature);
	}
	m_group->setButton(reason);

	// The comment text box:
	m_comment = new Q3TextEdit(box);
	m_comment->setTabChangesFocus(true);
	m_comment->setTextFormat(Qt::PlainText);
	m_comment->setText(initialComment);

	m_showButtons = new QCheckBox(i18n("Show comment buttons below &window titlebars"), page);
	m_showButtons->setChecked(m_likeBack->userWantsToShowBar());
	pageLayout->addWidget(m_showButtons);
	connect( m_showButtons, SIGNAL(stateChanged(int)), this, SLOT(changeButtonBarVisible()) );

	setButtonGuiItem(Ok,KGuiItem(i18n("&Send Comment")));
	enableButtonOk(false);
	connect( m_comment, SIGNAL(textChanged()), this, SLOT(commentChanged()) );

	setButtonGuiItem(Default, KGuiItem(i18n("&Email Address..."), "mail"));

	resize(QSize(kapp->desktop()->width() * 1 / 2, kapp->desktop()->height() * 3 / 5).expandedTo(sizeHint()));

	QAction *sendShortcut = new QAction(this);
	sendShortcut->setAccel(QString("Ctrl+Return"));
	connect( sendShortcut, SIGNAL(activated()), button(Ok), SLOT(animateClick()) );

	setMainWidget(page);
}

LikeBackDialog::~LikeBackDialog()
{
}

QString LikeBackDialog::introductionText()
{
	QString text = "<p>" + i18n("Please provide a brief description of your opinion of %1.").arg(m_likeBack->aboutData()->programName()) + " ";

	QString languagesMessage = "";
	if (!m_likeBack->acceptedLocales().isEmpty() && !m_likeBack->acceptedLanguagesMessage().isEmpty()) {
		languagesMessage = m_likeBack->acceptedLanguagesMessage();
		QStringList locales = m_likeBack->acceptedLocales();
		for (QStringList::Iterator it = locales.begin(); it != locales.end(); ++it) {
			QString locale = *it;
			if (KGlobal::locale()->language().startsWith(locale))
				languagesMessage = "";
		}
	} else {
		if (!KGlobal::locale()->language().startsWith("en"))
			languagesMessage = i18n("Please write in English.");
	}

	if (!languagesMessage.isEmpty())
		// TODO: Replace the URL with a localized one:
		text += languagesMessage + " " +
			i18n("You may be able to use an <a href=\"%1\">online translation tool</a>.")
			.arg("http://www.google.com/language_tools?hl=" + KGlobal::locale()->language())
			+ " ";

	// If both "I Like" and "I Dislike" buttons are shown and one is clicked:
	if ((m_likeBack->buttons() & LikeBack::Like) && (m_likeBack->buttons() & LikeBack::Dislike))
		text += i18n("To make the comments you send more useful in improving this application, try to send the same amount of positive and negative comments.") + " ";

	if (!(m_likeBack->buttons() & LikeBack::Feature))
		text += i18n("Do <b>not</b> ask for new features: your requests will be ignored.");

	return text;
}

void LikeBackDialog::polish()
{
	KDialog::polish();
	m_comment->setFocus();
}

void LikeBackDialog::slotDefault()
{
	m_likeBack->askEmailAddress();
}

void LikeBackDialog::slotOk()
{
	send();
}

void LikeBackDialog::changeButtonBarVisible()
{
	m_likeBack->setUserWantsToShowBar(m_showButtons->isChecked());
}

void LikeBackDialog::commentChanged()
{
	QPushButton *sendButton = button(Ok);
	sendButton->setEnabled(!m_comment->text().isEmpty());
}

void LikeBackDialog::send()
{
	QString emailAddress = m_likeBack->emailAddress();

	int reason = m_group->selectedId();
	QString type = (reason == LikeBack::Like ? "Like" : (reason == LikeBack::Dislike ? "Dislike" : (reason == LikeBack::Bug ? "Bug" : "Feature")));
	QString data =
		"protocol=" + QUrl::toPercentEncoding("1.0")                              + '&' +
		"type="     + QUrl::toPercentEncoding(type)                               + '&' +
		"version="  + QUrl::toPercentEncoding(m_likeBack->aboutData()->version()) + '&' +
		"locale="   + QUrl::toPercentEncoding(KGlobal::locale()->language())      + '&' +
		"window="   + QUrl::toPercentEncoding(m_windowPath)                       + '&' +
		"context="  + QUrl::toPercentEncoding(m_context)                          + '&' +
		"comment="  + QUrl::toPercentEncoding(m_comment->text())                  + '&' +
		"email="    + QUrl::toPercentEncoding(emailAddress);
	Q3Http *http = new Q3Http(m_likeBack->hostName(), m_likeBack->hostPort());

	kDebug() << "http://" << m_likeBack->hostName() << ":" << m_likeBack->hostPort() << m_likeBack->remotePath();
	kDebug() << data;
	connect( http, SIGNAL(requestFinished(int, bool)), this, SLOT(requestFinished(int, bool)) );

	Q3HttpRequestHeader header("POST", m_likeBack->remotePath());
	header.setValue("Host", m_likeBack->hostName());
	header.setValue("Content-Type", "application/x-www-form-urlencoded");
	http->setHost(m_likeBack->hostName());
	http->request(header, data.utf8());

	m_comment->setEnabled(false);
}

void LikeBackDialog::requestFinished(int /*id*/, bool error)
{
	// TODO: Save to file if error (connection not present at the moment)
	m_comment->setEnabled(true);
	m_likeBack->disableBar();
	if (error) {
		KMessageBox::error(this, i18n("<p>Error while trying to send the report.</p><p>Please retry later.</p>"), i18n("Transfer Error"));
	} else {
		KMessageBox::information(
			this,
			i18n("<p>Your comment has been sent successfully. It will help improve the application.</p><p>Thanks for your time.</p>"),
			i18n("Comment Sent")
		);
		close();
	}
	m_likeBack->enableBar();

	KDialog::accept();
}
