/***************************************************************************
 *   Copyright (C) 2006 by S�astien Laot                                 *
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

#ifndef LIKEBACK_H
#define LIKEBACK_H

#include <kdialog.h>
#include <qtimer.h>

class QTextEdit;
class QToolButton;
class QPushButton;

/**
  * @author S�astien Laot <slaout@linux62.org>
  */
class LikeBack : public QWidget
{
  Q_OBJECT
  public:
	enum Button { ILike = 0x01, IDoNotLike = 0x02, IFoundABug = 0x04, Configure = 0x10,
	              AllButtons = ILike | IDoNotLike | IFoundABug | Configure };
	enum WindowListing { NoListing, WarnUnnamedWindows, AllWindows };
	LikeBack(Button buttons = AllButtons);
	~LikeBack();
	static void showInformationMessage();
	static LikeBack* instance();
	static QString customLanguageMessage();
	static QString  hostName();
	static QString  remotePath();
	static Q_UINT16 hostPort();
	static void setServer(QString hostName, QString remotePath, Q_UINT16 hostPort = 80);
	static void setWindowNamesListing(WindowListing windowListing);
	static void setCustomLanguageMessage(const QString &message);
	static bool enabled();
	static void disable();
	static void enable();
	static bool userWantToParticipate(); /// << @Returns true if the user have not disabled LikeBack for this version
	static QString emailAddress(); /// << @Returns the email user address, or ask it to the user if he haven't provided or ignored it
	static void setEmailAddress(const QString &address); /// << Calling emailAddress() will ask it to the user the first time
	static bool isDevelopmentVersion(const QString &version = QString::null); /// << @Returns true if version is an alpha/beta/rc/svn/cvs version. Use kapp->aboutData()->version is @p version is empty
	static void init(Button buttons = AllButtons); /// << Initialize the LikeBack system: enable it if the application version is a development one.
	static void init(bool isDevelopmentVersion, Button buttons = AllButtons);  /// << Initialize the LikeBack system: enable it if @p isDevelopmentVersion is true.
	static QString activeWindowPath();
  private slots:
	void autoMove();
	void iLike();
	void iDoNotLike();
	void iFoundABug();
	void configure();
	void showDialog(Button button);
	void openConfigurePopup();
	void doNotHelpAnymore();
	void showWhatsThisMessage();
	void askEMail();
  private:
	QTimer       m_timer;
	Button       m_buttons;
	QToolButton *m_configureButton;
	static QString        s_hostName;
	static QString        s_remotePath;
	static Q_UINT16       s_hostPort;
	static QString        s_customLanguageMessage;
	static WindowListing  s_windowListing;
	static LikeBack      *s_instance;
	static int            s_disabledCount;
};

class LikeBackDialog : public KDialog
{
  Q_OBJECT
  public:
	LikeBackDialog(LikeBack::Button reason, QString windowName, QString context);
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
