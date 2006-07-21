/***************************************************************************
 *   Copyright (C) 2006 by Sébastien Laoût                                 *
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

#include <kapplication.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <kdialogbase.h>
#include <qhttp.h>
#include <kurl.h>
#include <kinputdialog.h>
#include <qvalidator.h>

#include <iostream>

#include "likeback.h"

LikeBack::LikeBack(Button buttons)
 : QWidget( 0, "LikeBack", Qt::WX11BypassWM | Qt::WStyle_NoBorder | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop | Qt::WStyle_NoBorder | Qt::Qt::WGroupLeader)
 , m_buttons(buttons)
{
	QHBoxLayout *layout = new QHBoxLayout(this);

	QIconSet likeIconSet      = kapp->iconLoader()->loadIconSet("likeback_like",      KIcon::Small);
	QIconSet dislikeIconSet   = kapp->iconLoader()->loadIconSet("likeback_dislike",   KIcon::Small);
	QIconSet bugIconSet       = kapp->iconLoader()->loadIconSet("bug",                KIcon::Small);
	QIconSet configureIconSet = kapp->iconLoader()->loadIconSet("likeback_configure", KIcon::Small);

	QToolButton *m_likeButton = new QToolButton(this, "ilike");
	m_likeButton->setIconSet(likeIconSet);
	m_likeButton->setTextLabel(i18n("I Like..."));
	m_likeButton->setAutoRaise(true);
	connect( m_likeButton, SIGNAL(clicked()), this, SLOT(iLike()) );
	layout->add(m_likeButton);

	QToolButton *m_dislikeButton = new QToolButton(this, "idonotlike");
	m_dislikeButton->setIconSet(dislikeIconSet);
	m_dislikeButton->setTextLabel(i18n("I Do not Like..."));
	m_dislikeButton->setAutoRaise(true);
	connect( m_dislikeButton, SIGNAL(clicked()), this, SLOT(iDoNotLike()) );
	layout->add(m_dislikeButton);

	QToolButton *m_bugButton = new QToolButton(this, "ifoundabug");
	m_bugButton->setIconSet(bugIconSet);
	m_bugButton->setTextLabel(i18n("I Found a Bug..."));
	m_bugButton->setAutoRaise(true);
	connect( m_bugButton, SIGNAL(clicked()), this, SLOT(iFoundABug()) );
	layout->add(m_bugButton);

	m_configureButton = new QToolButton(this, "configure");
	m_configureButton->setIconSet(configureIconSet);
	m_configureButton->setTextLabel(i18n("Configure..."));
	m_configureButton->setAutoRaise(true);
	connect( m_likeButton, SIGNAL(clicked()), this, SLOT(configure()) );
	layout->add(m_configureButton);

	QPopupMenu *configureMenu = new QPopupMenu(this);
	configureMenu->insertItem( i18n("What's &This?"), this , SLOT(showWhatsThisMessage()) );
	QIconSet changeEmailIconSet = kapp->iconLoader()->loadIconSet("mail_generic", KIcon::Small);
	configureMenu->insertItem( changeEmailIconSet, i18n("&Change or Remove Email Address..."), this , SLOT(askEMail()) );
	QIconSet dontHelpIconSet = kapp->iconLoader()->loadIconSet("stop", KIcon::Small);
	configureMenu->insertItem( dontHelpIconSet, i18n("&Do not Help Anymore"), this , SLOT(doNotHelpAnymore()) );
	m_configureButton->setPopup(configureMenu);
	connect( m_configureButton, SIGNAL(pressed()), this, SLOT(openConfigurePopup()) );

	static const char *messageShown = "LikeBack_starting_information";
	if (KMessageBox::shouldBeShownContinue(messageShown)) {
		showInformationMessage();
		KMessageBox::saveDontShowAgainContinue(messageShown);
	}

	resize(sizeHint());

	connect( &m_timer, SIGNAL(timeout()), this, SLOT(autoMove()) );
	m_timer.start(10);

	s_instance = this;
}

LikeBack::~LikeBack()
{
}

void LikeBack::openConfigurePopup()
{
	m_configureButton->openPopup();
}

void LikeBack::doNotHelpAnymore()
{
	disable();
	int result = KMessageBox::questionYesNo(
			kapp->activeWindow(),
			i18n("Are you sure you do not want to participate anymore in the application enhancing program?"),
			i18n("Do not Help Anymore"));
	if (result == KMessageBox::No) {
		enable();
		return;
	}

	KConfig *config = KGlobal::config();
	config->setGroup("LikeBack");
	config->writeEntry("userWantToParticipateForVersion_" + kapp->aboutData()->version(), false);
	deleteLater();
}

void LikeBack::showWhatsThisMessage()
{
	disable();
	showInformationMessage();
	enable();
}

bool LikeBack::userWantToParticipate()
{
	if (!kapp)
		return true;

	KConfig *config = KGlobal::config();
	config->setGroup("LikeBack");
	return config->readBoolEntry("userWantToParticipateForVersion_" + kapp->aboutData()->version(), true);
}

// TODO: Only show relevant buttons!

void LikeBack::showInformationMessage()
{
	QPixmap likeIcon    = kapp->iconLoader()->loadIcon("likeback_like",    KIcon::Small);
	QPixmap dislikeIcon = kapp->iconLoader()->loadIcon("likeback_dislike", KIcon::Small);
	QPixmap bugIcon     = kapp->iconLoader()->loadIcon("bug", KIcon::Small);
	QMimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_like",    likeIcon);
	QMimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_dislike", dislikeIcon);
	QMimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_bug",     bugIcon);
	KMessageBox::information(0,
		"<p><b>" + i18n("Welcome to this testing version of %1.").arg(kapp->aboutData()->programName()) + "</b></p>"
		"<p>" + i18n("To help us improve it, your comments are important.") + "</p>"
		"<p>" + i18n("Each time you have a great or frustrating experience, "
		             "please click the appropriate hand below the window title-bar, "
		             "briefly describe what you like or dislike and click Send.") + "</p>"
		"<p>" + i18n("Follow the same principle to quickly report a bug: "
		             "just click the bug icon in the top-right corner of the window, describe it and click Send.") + "</p>"
		"<p>" + i18n("Examples:") + "</p>"
		"<table><tr>"
		"<td><nobr><b><img source=\"likeback_icon_like\"> " + i18n("I like...") + "</b></nobr><br>" +
		i18n("I like...", "the nice new artwork.") + "<br>" +
		i18n("Very refreshing.") + "</td>"
		"<td><nobr><b><img source=\"likeback_icon_dislike\"> " + i18n("I do not like...") + "</b></nobr><br>" +
		i18n("I do not like...", "the welcome page of that wizard.") + "<br>" +
		i18n("Too time consuming.") + "</td>"
		"<td><nobr><b><img source=\"likeback_icon_bug\"> " + i18n("I found a bug...") + "</b></nobr><br>" +
		i18n("I found a bug...", "when clicking the Add button, nothing happens.") + "</td>"
		"</tr></table>",
		i18n("Help Improve the Application"));
	QMimeSourceFactory::defaultFactory()->setData("likeback_icon_like", 0L);
	QMimeSourceFactory::defaultFactory()->setData("likeback_icon_dislike", 0L);
}

QString                 LikeBack::s_customLanguageMessage = QString();
LikeBack::WindowListing LikeBack::s_windowListing         = LikeBack::NoListing;
QString                 LikeBack::s_hostName              = QString();
QString                 LikeBack::s_remotePath            = QString();
Q_UINT16                LikeBack::s_hostPort              = 16;
int                     LikeBack::s_disabledCount         = 0;
LikeBack*               LikeBack::s_instance              = 0;

LikeBack* LikeBack::instance()
{
	return s_instance;
}

QString LikeBack::customLanguageMessage()
{
	return s_customLanguageMessage;
}

QString LikeBack::hostName()
{
	return s_hostName;
}

QString LikeBack::remotePath()
{
	return s_remotePath;
}

Q_UINT16 LikeBack::hostPort()
{
	return s_hostPort;
}

void LikeBack::disable()
{
	s_disabledCount++;
}

void LikeBack::enable()
{
	s_disabledCount--;
	if (s_disabledCount < 0)
		std::cerr << "===== LikeBack ===== Enabled too many times (less than how many times it was disabled)" << std::endl;
}

bool LikeBack::enabled()
{
	return s_disabledCount == 0;
}

void LikeBack::setServer(QString hostName, QString remotePath, Q_UINT16 hostPort)
{
	s_hostName   = hostName;
	s_remotePath = remotePath;
	s_hostPort   = hostPort;
}

void LikeBack::setWindowNamesListing(WindowListing windowListing)
{
	s_windowListing = windowListing;
}

void LikeBack::setCustomLanguageMessage(const QString &message)
{
	s_customLanguageMessage = message;
}

void LikeBack::autoMove()
{
	static QWidget *lastWindow = 0;

	QWidget *window = kapp->activeWindow();
	bool shouldShow = (enabled() && window);
	if (shouldShow) {
		//move(window->x() + window->width() - 100 - width(), window->y());
		//move(window->x() + window->width() - 100 - width(), window->mapToGlobal(QPoint(0, 0)).y() - height());
		move(window->mapToGlobal(QPoint(0, 0)).x() + window->width() - width(), window->mapToGlobal(QPoint(0, 0)).y() + 1);

		if (window != lastWindow && s_windowListing != NoListing)
			if (qstricmp(window->name(), "") == 0 || qstricmp(window->name(), "unnamed") == 0)
				std::cout << "===== LikeBack ===== UNNAMED ACTIVE WINDOW OF TYPE " << window->className() << " ======" << activeWindowPath() << std::endl;
			else if (s_windowListing == AllWindows)
				std::cout << "LikeBack: Active Window: " << activeWindowPath() << std::endl;
		lastWindow = window;
	}

	if (shouldShow && !isShown()) {
		show();
	} else if (!shouldShow && isShown())
		hide();
}

void LikeBack::iLike()
{
	showDialog(ILike);
}

void LikeBack::iDoNotLike()
{
	showDialog(IDoNotLike);
}

void LikeBack::iFoundABug()
{
	showDialog(IFoundABug);
}

void LikeBack::configure()
{
}

QString LikeBack::activeWindowPath()
{
	QStringList windowNames;
	QWidget *window = kapp->activeWindow();
	while (window) {
		QString name = window->name();
		if (name == "unnamed")
			name += QString(":") + window->className();
		windowNames.append(name);
		window = dynamic_cast<QWidget*>(window->parent());
	}

	QString windowName;
	for (int i = ((int)windowNames.count()) - 1; i >= 0; i--) {
		if (windowName.isEmpty())
			windowName = windowNames[i];
		else
			windowName += QString("~~") + windowNames[i];
	}

	return windowName;
}

void LikeBack::showDialog(Button button)
{
	LikeBackDialog dialog(button, activeWindowPath(), "");
	disable();
	hide();
	kapp->processEvents();
	dialog.exec();
	enable();
}

QString LikeBack::emailAddress()
{
	KConfig *config = KGlobal::config();
	config->setGroup("LikeBack");

	if (config->readBoolEntry("emailAlreadyAsked", false) == false)
		instance()->askEMail();

	return config->readEntry("emailAddress", "");
}

void LikeBack::setEmailAddress(const QString &address)
{
	KConfig *config = KGlobal::config();
	config->setGroup("LikeBack");
	config->writeEntry("emailAddress",      address);
	config->writeEntry("emailAlreadyAsked", true);
}

void LikeBack::askEMail()
{
	KConfig *config = KGlobal::config();
	config->setGroup("LikeBack");
	QString currentEMailAddress = config->readEntry("emailAddress", "");

	bool ok;

	QString mailExpString = "[\\w-\\.]+@[\\w-\\.]+\\.[\\w]+";
	QRegExp mailExp("^(|"+mailExpString+")$");
	QRegExpValidator emailValidator(mailExp, this);

	disable();
	QString email = KInputDialog::getText(
			i18n("Set Email Address"),
			"<p><b>" + i18n("Please provide your email address.") + "</b></p>" +
			"<p>" + i18n("It will only be used to contact you back if more information are needed about your comments, how to reproduce the bugs you report, send bug corrections for you to test...") + "</p>" +
			"<p>" + i18n("The email address is optionnal. If you do not provide any, your comments will be sent annonymously. Just click OK in that case.") + "</p>" +
			"<p>" + i18n("You can change or remove your email address whenever you want. For that, use the little arrow icon at the top-right corner of a window.") + "</p>" +
			"<p>" + i18n("Your email address (keep empty to post comments annonymously):"),
			currentEMailAddress, &ok, kapp->activeWindow(), /*name=*/(const char*)0, &emailValidator);
	enable();

	if (ok)
		setEmailAddress(email);
}

// FIXME: Should be moved to KAboutData? Cigogne will also need it.
bool LikeBack::isDevelopmentVersion(const QString &version)
{
	QString theVersion = (version.isEmpty() ? kapp->aboutData()->version() : version);

	return theVersion.find("alpha", /*index=*/0, /*caseSensitive=*/false) != -1 ||
	       theVersion.find("beta",  /*index=*/0, /*caseSensitive=*/false) != -1 ||
	       theVersion.find("rc",    /*index=*/0, /*caseSensitive=*/false) != -1 ||
	       theVersion.find("svn",   /*index=*/0, /*caseSensitive=*/false) != -1 ||
	       theVersion.find("cvs",   /*index=*/0, /*caseSensitive=*/false) != -1;
}

void LikeBack::init(Button buttons)
{
	init(isDevelopmentVersion(), buttons);
}

void LikeBack::init(bool isDevelopmentVersion, Button buttons)
{
	if (LikeBack::userWantToParticipate() && isDevelopmentVersion)
		new LikeBack(buttons);
}

/** class LikeBackDialog: */

LikeBackDialog::LikeBackDialog(LikeBack::Button reason, QString windowName, QString context)
 : KDialog(kapp->activeWindow(), "_likeback_feedback_window_")
 , m_reason(reason)
 , m_windowName(windowName)
 , m_context(context)
{
	setModal(true);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	QWidget *coloredWidget = new QWidget(this);
	QLabel  *explainings = new QLabel(this);
	QHBoxLayout *explainingLayout = new QHBoxLayout((QWidget*)0, KDialogBase::marginHint());
	explainingLayout->addWidget(explainings);
	mainLayout->addWidget(coloredWidget);

	QColor  color;
	QColor  lineColor;
	QPixmap icon;
	QString title;
	QString please;
	switch (reason) {
		case LikeBack::ILike:
			color     = QColor("#DFFFDF");
			lineColor = Qt::green;
			icon      = kapp->iconLoader()->loadIcon("likeback_like",    KIcon::Small);
			title     = i18n("I like...");
			please    = i18n("Please briefly describe what you like.");
			break;
		case LikeBack::IDoNotLike:
			color     = QColor("#FFDFDF");
			lineColor = Qt::red;
			icon      = kapp->iconLoader()->loadIcon("likeback_dislike", KIcon::Small);
			title     = i18n("I do not like...");
			please    = i18n("Please briefly describe what you do not like.");
			break;
		case LikeBack::IFoundABug:
			color     = QColor("#C0C0C0");
			lineColor = Qt::black;
			icon      = kapp->iconLoader()->loadIcon("bug",              KIcon::Small);
			title     = i18n("I found a bug...");
			please    = i18n("Please briefly describe the bug you encountered.");
			break;
		case LikeBack::Configure:
		case LikeBack::AllButtons:
			return;
	}

	QWidget *line = new QWidget(this);
	line->setPaletteBackgroundColor(lineColor);
	line->setFixedHeight(1);
	mainLayout->addWidget(line);
	mainLayout->addLayout(explainingLayout);

	QHBoxLayout *titleLayout = new QHBoxLayout(0);
	coloredWidget->setPaletteBackgroundColor(color);
	QLabel *iconLabel = new QLabel(coloredWidget);
	iconLabel->setPixmap(icon);
	QLabel *titleLabel = new QLabel(title, coloredWidget);
	QFont font = titleLabel->font();
	font.setBold(true);
	titleLabel->setFont(font);
	titleLabel->setPaletteForegroundColor(Qt::black);
	titleLayout->addWidget(iconLabel);
	titleLayout->addSpacing(4);
	titleLayout->addWidget(titleLabel);
	titleLayout->addStretch();

	QVBoxLayout *coloredWidgetLayout = new QVBoxLayout(coloredWidget);
	coloredWidgetLayout->setMargin(KDialogBase::marginHint());
	coloredWidgetLayout->setSpacing(KDialogBase::spacingHint());
	coloredWidgetLayout->addLayout(titleLayout);

	QHBoxLayout *commentLayout = new QHBoxLayout((QWidget*)0);
	commentLayout->setMargin(0);
	commentLayout->setSpacing(KDialogBase::spacingHint());
	m_comment = new QTextEdit(coloredWidget);
	QIconSet sendIconSet = kapp->iconLoader()->loadIconSet("mail_send", KIcon::Toolbar);
	m_sendButton = new QPushButton(sendIconSet, i18n("Send"), coloredWidget);
	m_sendButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_sendButton->setEnabled(false);
	connect( m_sendButton, SIGNAL(clicked()),     this, SLOT(send())           );
	connect( m_comment,    SIGNAL(textChanged()), this, SLOT(commentChanged()) );
	commentLayout->addWidget(m_comment);
	commentLayout->addWidget(m_sendButton);
	coloredWidgetLayout->addLayout(commentLayout);

	explainings->setText(
			"<p>" + please + " " +
			(LikeBack::customLanguageMessage().isEmpty() ?
				i18n("Only english language is accepted.") :
				LikeBack::customLanguageMessage()
			)  + " " +
			(reason == LikeBack::ILike || reason == LikeBack::IDoNotLike ?
				i18n("Note that to improve this application, it's important to tell us the things you like as much as the things you dislike.") + " " :
				""
			) +
			i18n("Do not ask for features: your wishes will be ignored.") + "</p>"
	);

	resize(kapp->desktop()->width() / 2, kapp->desktop()->height() / 3);

	setCaption(kapp->makeStdCaption(i18n("Send a Comment")));
	//	setMinimumSize(mainLayout->sizeHint()); // FIXME: Doesn't work!
}

LikeBackDialog::~LikeBackDialog()
{
}

QHttp *http ;

void LikeBackDialog::send()
{
	QString emailAddress = LikeBack::instance()->emailAddress();

	QString type = (m_reason == LikeBack::ILike ? "Like" : (m_reason == LikeBack::IDoNotLike ? "Dislike" : "Bug"));
	QString data =
			"protocol=" + KURL::encode_string("1.0")                         + "&" +
			"type="     + KURL::encode_string(type)                          + "&" +
			"version="  + KURL::encode_string(kapp->aboutData()->version())  + "&" +
			"locale="   + KURL::encode_string(KGlobal::locale()->language()) + "&" +
			"window="   + KURL::encode_string(m_windowName)                  + "&" +
			"context="  + KURL::encode_string(m_context)                     + "&" +
			"comment="  + KURL::encode_string(m_comment->text())             + "&" +
			"email="    + KURL::encode_string(emailAddress);
	//QByteArray *data = new QByteArray();
	/*QHttp **/http = new QHttp(LikeBack::hostName(), LikeBack::hostPort());

	std::cout << "http://" << LikeBack::hostName() << ":" << LikeBack::hostPort() << LikeBack::remotePath() << std::endl;
	std::cout << data << std::endl;
	connect( http, SIGNAL(requestFinished(int, bool)), this, SLOT(requestFinished(int, bool)) );
//	http->post(LikeBack::remotePath(), data.utf8());

	QHttpRequestHeader header("POST", LikeBack::remotePath());
	header.setValue("Host", LikeBack::hostName());
	header.setValue("Content-Type", "application/x-www-form-urlencoded");
	http->setHost(LikeBack::hostName());
	http->request(header, data.utf8());


	m_comment->setEnabled(false);
}

void LikeBackDialog::requestFinished(int /*id*/, bool error)
{
	// TODO: Save to file if error (connection not present at the moment)
	m_comment->setEnabled(true);
	LikeBack::disable();
	if (error) {
		KMessageBox::error(this, i18n("<p>Error while trying to send the report.</p><p>Please retry later.</p>"), i18n("Transfer Error"));
	} else {
		KMessageBox::information(this, i18n("<p>Your comment has been sent successfully.</p><p>Thanks for your time. It will help improve the application.</p>") /*+ QString(http->readAll())*/, i18n("Comment Sent"));
		close();
	}
	LikeBack::enable();
}

void LikeBackDialog::commentChanged()
{
	m_sendButton->setEnabled(!m_comment->text().isEmpty());
}

#include "likeback.moc"
