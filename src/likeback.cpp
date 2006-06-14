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
#include <kiconloader.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <kdialogbase.h>
#include <qhttp.h>
#include <kurl.h>

#include <iostream>

#include "likeback.h"

static const char *LIKEBACK_WINDOW = "_likeback_feedback_window_";

LikeBack::LikeBack(Button buttons, bool warnUnnamedWindow, const QString &customLanguageMessage)
 : QWidget( 0, "LikeBack", Qt::WX11BypassWM | Qt::WStyle_NoBorder | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop | Qt::WStyle_NoBorder | Qt::Qt::WGroupLeader)
 , m_buttons(buttons)
 , m_warnUnnamedWindow(warnUnnamedWindow)
 , m_canShow(true)
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
	m_dislikeButton->setTextLabel(i18n("I Dislike..."));
	m_dislikeButton->setAutoRaise(true);
	connect( m_dislikeButton, SIGNAL(clicked()), this, SLOT(iDoNotLike()) );
	layout->add(m_dislikeButton);

	QToolButton *m_bugButton = new QToolButton(this, "ifoundabug");
	m_bugButton->setIconSet(bugIconSet);
	m_bugButton->setTextLabel(i18n("I Found a Bug..."));
	m_bugButton->setAutoRaise(true);
	connect( m_bugButton, SIGNAL(clicked()), this, SLOT(iFoundABug()) );
	layout->add(m_bugButton);

	QToolButton *m_configureButton = new QToolButton(this, "configure");
	m_configureButton->setIconSet(configureIconSet);
	m_configureButton->setTextLabel(i18n("Configure..."));
	m_configureButton->setAutoRaise(true);
	connect( m_likeButton, SIGNAL(clicked()), this, SLOT(configure()) );
	layout->add(m_configureButton);

	static const char *messageShown = "LikeBack_starting_information";
	if (KMessageBox::shouldBeShownContinue(messageShown)) {
		showInformationMessage();
		KMessageBox::saveDontShowAgainContinue(messageShown);
	}

	resize(sizeHint());

	connect( &m_timer, SIGNAL(timeout()), this, SLOT(autoMove()) );
	m_timer.start(10);

	s_instance = this;
	s_customLanguageMessage = customLanguageMessage;
}

LikeBack::~LikeBack()
{
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
		i18n(
			"<p><b>Welcome to this testing version of %1.</b></p>"
			"<p>To help us improve it, your comments are important.</p>"
			"<p>Each time you are surprised with something you like, or something you do not like, "
			"please click the appropriate hand below the window title-bar, "
			"briefly describe the object of your sentiment and click Send.</p>"
			"<p>Follow the same principle to quickly report a bug: "
			"just click the bug icon in the top-right corner of the window, describe it and click Send.</p>"
			"<p>Examples:</p>"
			"<table><tr>"
			"<td><nobr><b><img source=\"likeback_icon_like\"> I like...</b></nobr><br>"
			"the nice new artwork.<br>"
			"Very refreshing.</td>"
			"<td><nobr><b><img source=\"likeback_icon_dislike\"> I do not like...</b></nobr><br>"
			"the welcome page of that wizard.<br>"
			"Too time consuming.</td>"
			"<td><nobr><b><img source=\"likeback_icon_bug\"> I found a bug...</b></nobr><br>"
			"when clicking the Add button, nothing happens.</td>"
			"</tr></table>").arg(kapp->aboutData()->programName()),
		i18n("Help Improve the Application"));
	QMimeSourceFactory::defaultFactory()->setData("likeback_icon_like", 0L);
	QMimeSourceFactory::defaultFactory()->setData("likeback_icon_dislike", 0L);
}

QString   LikeBack::s_customLanguageMessage = QString();
QString   LikeBack::s_hostName              = QString();
QString   LikeBack::s_remotePath            = QString();
Q_UINT16  LikeBack::s_hostPort              = 16;
int       LikeBack::s_disabledCount         = 0;
LikeBack* LikeBack::s_instance = 0;

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

void LikeBack::autoMove()
{
	static QWidget *lastWindow = 0;

	QWidget *window = kapp->activeWindow();
	bool shouldShow = (m_canShow && enabled() && window && qstricmp(window->name(), LIKEBACK_WINDOW) != 0);
	if (shouldShow) {
		//move(window->x() + window->width() - 100 - width(), window->y());
		//move(window->x() + window->width() - 100 - width(), window->mapToGlobal(QPoint(0, 0)).y() - height());
		move(window->mapToGlobal(QPoint(0, 0)).x() + window->width() - width(), window->mapToGlobal(QPoint(0, 0)).y() + 1);

		if (window != lastWindow)
			if (m_warnUnnamedWindow && (qstricmp(window->name(), "") == 0 || qstricmp(window->name(), "unnamed") == 0)) {
				std::cout << "===== LikeBack ===== UNNAMED ACTIVE WINDOW ======" << std::endl;
			} else
				std::cout << "LikeBack: Active Window: " << window->name() << std::endl;
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

void LikeBack::showDialog(Button button)
{
	QString windowName = (kapp->activeWindow() ? kapp->activeWindow()->name() : "");
	LikeBackDialog dialog(button, windowName, "");
	hide();
	m_canShow = false;
	kapp->processEvents();
	dialog.exec();
	m_canShow = true;
}

/** class LikeBackDialog: */

LikeBackDialog::LikeBackDialog(LikeBack::Button reason, QString windowName, QString context)
 : KDialog(kapp->activeWindow(), LIKEBACK_WINDOW)
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
		case LikeBack::All:
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

	QHBoxLayout *messageLayout = new QHBoxLayout((QWidget*)0);
	messageLayout->setMargin(0);
	messageLayout->setSpacing(KDialogBase::spacingHint());
	m_message = new QTextEdit(coloredWidget);
	QIconSet sendIconSet = kapp->iconLoader()->loadIconSet("mail_send", KIcon::Toolbar);
	QToolButton *sendButton = new QToolButton(coloredWidget);
	sendButton->setIconSet(sendIconSet);
	sendButton->setIconText(i18n("Send"));
// 	sendButton->setText(i18n("Send"));
	sendButton->setTextLabel(i18n("Send"));
	sendButton->setUsesTextLabel(true);
	sendButton->setTextPosition(QToolButton::BesideIcon);
	connect( sendButton, SIGNAL(clicked()), this, SLOT(send()) );
	messageLayout->addWidget(m_message);
	messageLayout->addWidget(sendButton);
	coloredWidgetLayout->addLayout(messageLayout);

	explainings->setText(
			"<p>" + please + " " +
			(LikeBack::customLanguageMessage().isEmpty() ?
				i18n("Only english and french languages are accepted.") : // TODO: Remove french!
				LikeBack::customLanguageMessage()
			)  + "</p><p>" +
			(reason == LikeBack::ILike || reason == LikeBack::IDoNotLike ?
				i18n("Note that to improve this application, it's important to tell us the things you like as much as the things you dislike.") + " " :
				""
			) +
			i18n("Do not ask for features: your wish will be ignored.") + "</p><p>" +
			i18n("The comment will be submitted annonymously."));

	resize(kapp->desktop()->width() / 2, kapp->desktop()->height() / 3);

	// TODO: message should never be empty!
}

LikeBackDialog::~LikeBackDialog()
{
}

QHttp *http ;

void LikeBackDialog::send()
{
	QString type = (m_reason == LikeBack::ILike ? "Like" : (m_reason == LikeBack::IDoNotLike ? "Dislike" : "Bug"));
	QString data =
			"version=" + KURL::encode_string(kapp->aboutData()->version()) + "&" +
			"window="  + KURL::encode_string(m_windowName)                 + "&" +
			"context=" + KURL::encode_string(m_context)                    + "&" +
			"type="    + KURL::encode_string(type)                         + "&" +
			"comment=" + KURL::encode_string(m_message->text());
	//QByteArray *data = new QByteArray();
	/*QHttp **/http = new QHttp(LikeBack::hostName(), LikeBack::hostPort());

	std::cout << LikeBack::hostName() << LikeBack::hostPort() << LikeBack::remotePath() << std::endl;
	std::cout << data << std::endl;
	connect( http, SIGNAL(requestFinished(int, bool)), this, SLOT(requestFinished(int, bool)) );
//	http->post(LikeBack::remotePath(), data.utf8());

	QHttpRequestHeader header("POST", LikeBack::remotePath());
	header.setValue("Host", LikeBack::hostName());
	header.setValue("Content-Type", "application/x-www-form-urlencoded");
	http->setHost(LikeBack::hostName());
	http->request(header, data.utf8());


	m_message->setEnabled(false);
}

// TODO: Send a hierarchy of windows!!        ( "MainWindow -> Properties -> error" instead of "error")

void LikeBackDialog::requestFinished(int /*id*/, bool error)
{
	// TODO: Save to file if error (connection not present at the moment)
	m_message->setEnabled(true);
	LikeBack::disable();
	if (error) {
		KMessageBox::error(this, i18n("Error while trying to send the report."), i18n("Transfert Error"));
	} else {
		KMessageBox::information(this, i18n("<p>Your comment has been sent successfully.</p><p>Thanks for your time. It will help improve the application.") /*+ QString(http->readAll())*/, i18n("Message Sent"));
		close();
	}
	LikeBack::enable();
}

#include "likeback.moc"
