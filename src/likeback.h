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

#ifndef LIKEBACK_H
#define LIKEBACK_H

#include <kdialog.h>
#include <qtimer.h>

class QTextEdit;
class QToolButton;

/**
  * @author Sébastien Laoût <slaout@linux62.org>
  */
class LikeBack : public QWidget
{
  Q_OBJECT
  public:
	enum Button { ILike = 0x01, IDoNotLike = 0x02, IFoundABug = 0x04, Configure = 0x10,
	              AllButtons = ILike | IDoNotLike | IFoundABug | Configure };
	enum WindowListing { NoListing, WarnUnnamedWindows, AllWindows };
	LikeBack(Button buttons = AllButtons, WindowListing windowListing = AllWindows, const QString &customLanguageMessage = "");
	~LikeBack();
	static void showInformationMessage();
	static LikeBack* instance();
	static QString customLanguageMessage();
	static QString  hostName();
	static QString  remotePath();
	static Q_UINT16 hostPort();
	static void setServer(QString hostName, QString remotePath, Q_UINT16 hostPort = 80);
	static bool enabled();
	static void disable();
	static void enable();
	static bool userWantToParticipate();
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
  private:
	QTimer         m_timer;
	Button         m_buttons;
	QToolButton   *m_configureButton;
	WindowListing  m_windowListing;
	bool           m_canShow;
	static QString   s_hostName;
	static QString   s_remotePath;
	static Q_UINT16  s_hostPort;
	static QString   s_customLanguageMessage;
	static LikeBack *s_instance;
	static int       s_disabledCount;
};

class LikeBackDialog : public KDialog
{
  Q_OBJECT
  public:
	LikeBackDialog(LikeBack::Button reason, QString windowName, QString context);
	~LikeBackDialog();
  private:
	LikeBack::Button  m_reason;
	QTextEdit        *m_message;
	QString           m_windowName;
	QString           m_context;
  private slots:
	void send();
	void requestFinished(int id, bool error);
};

#endif // LIKEBACK_H
