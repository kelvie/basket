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

#ifndef LIKEBACK_H
#define LIKEBACK_H

#include <kdialog.h>
#include <qtimer.h>

class QTextEdit;
class QToolButton;
class QPushButton;
class KProcess;

/**
  * @author Sebastien Laout <slaout@linux62.org>
  */
class LikeBack : public QWidget
{
  Q_OBJECT
  public:
	enum Button { ILike = 0x01, IDoNotLike = 0x02, IFoundABug = 0x04, IWishAFeature = 0x10, Configure = 0x20,
	              AllButtons = ILike | IDoNotLike | IFoundABug | IWishAFeature | Configure,
	              DefaultButtons = ILike | IDoNotLike | Configure };
	enum WindowListing { NoListing, WarnUnnamedWindows, AllWindows };
	LikeBack(Button buttons = DefaultButtons);
	~LikeBack();
	static void showInformationMessage();
	static LikeBack* instance();
	static QString customLanguageMessage();
	static QString  hostName();
	static QString  remotePath();
	static Q_UINT16 hostPort();
	static void setServer(const QString &hostName, const QString &remotePath, Q_UINT16 hostPort = 80);
	static void setWindowNamesListing(WindowListing windowListing);
	static void setCustomLanguageMessage(const QString &message);
	static bool enabled();
	static void disable();
	static void enable();
	static bool userWantToParticipate(); /// << @Returns true if the user have not disabled LikeBack for this version
	static bool emailAddressAlreadyProvided();
	static QString emailAddress(); /// << @Returns the email user address, or ask it to the user if he haven't provided or ignored it
	static void setEmailAddress(const QString &address); /// << Calling emailAddress() will ask it to the user the first time
	static bool isDevelopmentVersion(const QString &version = QString::null); /// << @Returns true if version is an alpha/beta/rc/svn/cvs version. Use kapp->aboutData()->version is @p version is empty
	static void init(Button buttons = DefaultButtons); /// << Initialize the LikeBack system: enable it if the application version is a development one.
	static void init(bool isDevelopmentVersion, Button buttons = DefaultButtons);  /// << Initialize the LikeBack system: enable it if @p isDevelopmentVersion is true.
	static void init(KConfig* config, KAboutData* about, Button buttons = DefaultButtons);
	static QString activeWindowPath();
	static KAboutData* about();
	Button shownButtons();
  private slots:
	void autoMove();
	void iLike();
	void iDoNotLike();
	void iFoundABug();
	void iWishAFeature();
	void configure();
	void showDialog(Button button);
	void openConfigurePopup();
	void doNotHelpAnymore();
	void showWhatsThisMessage();
	void askEMail();
//	void beginFetchingEmail();
	void endFetchingEmailFrom(); // static QString fetchingEmail();
	void setButtonVisibility(Button buttons);
  private:
	QTimer       m_timer;
	Button       m_buttons;
	QToolButton *m_likeButton;
	QToolButton *m_dislikeButton;
	QToolButton *m_bugButton;
	QToolButton *m_wishButton;
	QToolButton *m_configureButton;
	QString      m_fetchedEmail;
	KProcess    *m_process;
	static QString        s_hostName;
	static QString        s_remotePath;
	static Q_UINT16       s_hostPort;
	static QString        s_customLanguageMessage;
	static WindowListing  s_windowListing;
	static LikeBack      *s_instance;
	static int            s_disabledCount;
	static KConfig       *s_config;
	static KAboutData    *s_about;
};

class LikeBackDialog : public KDialog
{
  Q_OBJECT
  public:
	LikeBackDialog(LikeBack::Button reason, const QString &windowName, const QString &context);
	~LikeBackDialog();
  private:
	LikeBack::Button  m_reason;
	QTextEdit        *m_comment;
	QPushButton      *m_sendButton;
	QString           m_windowName;
	QString           m_context;
  private slots:
	void send();
	void requestFinished(int id, bool error);
	void commentChanged();
};

#endif // LIKEBACK_H
