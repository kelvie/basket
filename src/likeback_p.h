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

#ifndef LIKEBACK_PRIVATE_H
#define LIKEBACK_PRIVATE_H

#include <KDialog>
#include <qtimer.h>

#include "likeback.h"

class QToolButton;
class QTextEdit;
class QCheckBox;
class Q3ButtonGroup;
class Kaction;

class LikeBackPrivate
{
  public:
	LikeBackPrivate();
	~LikeBackPrivate();
	LikeBackBar             *bar;
	KConfig                 *config;
	const KAboutData        *aboutData;
	LikeBack::Button         buttons;
	QString                  hostName;
	QString                  remotePath;
	quint16                 hostPort;
	QStringList              acceptedLocales;
	QString                  acceptedLanguagesMessage;
	LikeBack::WindowListing  windowListing;
	bool                     showBarByDefault;
	bool                     showBar;
	int                      disabledCount;
	QString                  fetchedEmail;
	KAction                 *action;
};

class LikeBackBar : public QWidget
{
  Q_OBJECT
  public:
	LikeBackBar(LikeBack *likeBack);
	~LikeBackBar();
  public slots:
	void startTimer();
	void stopTimer();
  private slots:
	void autoMove();
	void clickedLike();
	void clickedDislike();
	void clickedBug();
	void clickedFeature();
  private:
	LikeBack    *m_likeBack;
	QTimer       m_timer;
	QToolButton *m_likeButton;
	QToolButton *m_dislikeButton;
	QToolButton *m_bugButton;
	QToolButton *m_featureButton;
};

class LikeBackDialog : public KDialog
{
  Q_OBJECT
  public:
	LikeBackDialog(LikeBack::Button reason, const QString &initialComment, const QString &windowPath, const QString &context, LikeBack *likeBack);
	~LikeBackDialog();
  private:
	LikeBack     *m_likeBack;
	QString       m_windowPath;
	QString       m_context;
	Q3ButtonGroup *m_group;
	QTextEdit    *m_comment;
	QCheckBox    *m_showButtons;
	QString introductionText();
  private slots:
	void polish();
	void slotDefault();
	void slotOk();
	void changeButtonBarVisible();
	void commentChanged();
	void send();
	void requestFinished(int id, bool error);
};

#endif // LIKEBACK_PRIVATE_H
