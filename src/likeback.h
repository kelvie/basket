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

#include <qobject.h>

class KConfig;
class KAboutData;
class KAction;
class KActionCollection;

class LikeBackPrivate;
class LikeBackBar;
class LikeBackDialog;

/**
 * TODO
 * The LikeBack system has 5 components:
 * @li In the application: The comment dialog, where the user write a comment, select a type of comment, etc...
 * @li In the application: The KAction to plug in the Help menu. This action displays the comment dialog.
 * @li In the application: The button-bar, which floats bellow titlebar of every windows of the application, and let the user to quickly show the comment dialog.
 * @li On the server: A PHP script that collects every comments that users send. The LikeBack object should be configured to contact that server.
 * @li On the server: The developer interface. It lists every comments that were sent, let you sort them, add remarks to them, and mark them as fixed or another status.
 * @see http://basket.kde.org/likeback.php for more information, screenshots, tutorial, hints, return of experiences...
 * @author Sebastien Laout <slaout@linux62.org>
 */
class LikeBack : public QObject
{
  Q_OBJECT
  public:
	/**
	 * Ids of every LikeBack buttons the button-bar can have.
	 * The four first values are each individual buttons you can enable or not.
	 * The next ones are combinations: all buttons at once, and the default set of buttons (Like, Dislike).
	 * Those values are used ...... TODO
	 */
	enum Button { Like = 0x01, Dislike = 0x02, Bug = 0x04, Feature = 0x10, // TODO: enum CommentType ?
	              AllButtons = Like | Dislike | Bug | Feature,
	              DefaultButtons = Like | Dislike };

	/**
	 * @See the LikeBack constructor to know how to use those values.
	 */
	enum WindowListing {
		NoListing = 0,          /// << Do not print out any window name. For release use.
		WarnUnnamedWindows = 1, /// << Each time the user option a window, print out a message if the window is unnamed. For development needs, to check windows.
		AllWindows = 2          /// << Print out the window hierarchy of each opened windows during execution. For development needs, to check every names.
	};

	/**
	 * You only need to call the constructor once, typically in main.cpp.
	 * Even if you do not show the button-bar by default, you should instanciate LikeBack,
	 * to include its action in the Help menu of your application, to let the users send comments or activate the bar.
	 * @param buttons              The types of comments you want to get. Determine which radio-buttons are shown in the comment dialog,
	 *                             and which ones are displayed in the button-bar.
	 * @param showButtonsByDefault TODO. Advise: to avoid getting too much noise, enable it only if it is a small application or a development release.
	 * @param config               TODO. Used for TODO. By default (null), the KApplication configuration object is used.
	 * @param aboutData            TODO. Used to get the application name and version. By default (null), the KApplication about data object is used.
	 *                             The application name is only used in the first-use information message.
	 *                             The version is used to store the button-bar per version (can be shown in a development version but not a final one...)
	 *                             and to send with the comment, so you can filter per version and know if a comment refers the last version of the application or not.
	 */
	LikeBack(Button buttons = DefaultButtons, bool showBarByDefault = false, KConfig *config = 0, KAboutData *aboutData = 0);

	/**
	 * Destructor.
	 * Also hide the button-bar, if it was shown.
	 */
	~LikeBack();

	/**
	 * TODO
	 */
	void setWindowNamesListing(WindowListing windowListing);

	/**
	 * TODO
	 */
	WindowListing windowNamesListing();

	/**
	 * By default, only English comments are accepted. The user is informed she must write in this language by a sentence placed in the comment dialog.
	 * If you have people talking other languages in your development team, it can be interesting to call this method to define the accepted locales (languages),
	 * and provide a message to inform users. The developer interface on the server let developers view comments in theire locale.
	 * Note that no verification is done to check if the user used the right language, it would be impossible.
	 * The list of locales is there to make it possible to NOT show the message for users of the accepted languages.
	 * For instance, if you accept only English and French, and that the application run in a French environment,
	 * it's likely the user is French and will write comments using French. Telling him he should write in French is unnecessary and redundant.
	 * Passing an empty list and an empty string to the method will make LikeBack display the default message telling the user only English is accepted.
	 * Note: during tests, if you do not see the sentence, it's because you are running the application with an "accepted language": do not be surprised ;-)
	 * Example of call you can quickly copy, paste and adapt:
	 * @code
	 *     likeBack->setAcceptedLanguages(QStringList::split(";", "en;fr"), i18n("Please write in English or French."));
	 * @endcode
	 * @param locales TODO The list of locales where the message does not need to be shown.
	 * @param message TODO
	 */
	void setAcceptedLanguages(const QStringList &locales, const QString &message);

	/**
	 * @Returns The list of accepted locales for the user to write comments.
	 * @see setAcceptedLanguages().
	 */
	QStringList acceptedLocales();

	/**
	 * @Returns The message displayed to users who are not running the application in an accepted locale.
	 * @see setAcceptedLanguages().
	 */
	QString acceptedLanguagesMessage();

	/**
	 * Set the path where LikeBack should send every comments.
	 * It's composed of the server host name, the path to the PHP script used to send comments, and optionnaly a port number if it is not 80.
	 * This call is mandatory for LikeBack to work.
	 * @param hostName   The server host name to contact when sending comments. For instance "myapp.kde.org".
	 * @param remotePath The path to the send script on the server. For instance, "/likeback/send.php".
	 * @param hostPort   Optionnal port used to contact the server using the HTTP protocol. By default, it's port 80.
	 */
	void setServer(const QString &hostName, const QString &remotePath, Q_UINT16 hostPort = 80);

	/**
	 * @Returns The server host name to contact when sending comments.
	 * @see setServer().
	 */
	QString hostName();

	/**
	 * @Returns The path to the send script on the server.
	 * @see setServer().
	 */
	QString remotePath();

	/**
	 * @Returns The port used to contact the server using the HTTP protocol.
	 * @see setServer().
	 */
	Q_UINT16 hostPort();

	/**
	 * Get the KAction letting user to show the comment dialog.
	 * You should plug it in your Help menu, just bellow the "Report a Bug" action, or replace it.
	 * Adding the action below "Report a Bug" or replacing "Report a Bug" depends on your application and if you have a Bugzilla account.
	 * If you do not have a Bugzilla account, LikeBack is a good way for your small application to get bug reports: remove "Report a Bug".
	 * For more information about how to configure LikeBack depending on your application size and settings, see the constructor documentation.
	 * @Note The action is named "likeback_send_a_comment". So you should add the following XML in the *ui.rc file of your application:
	 *       <Action name="likeback_send_a_comment" />
	 */
	KAction* sendACommentAction(KActionCollection *parent = 0);

	/**
	 * @Returns true if the button-bar is currently enabled. Ie, if it has been re-enabled as many times as it has been disabled.
	 * @see disableBar() for more information on how enabling/disabling works.
	 */
	bool enabledBar();

  public slots:

	/**
	 * TODO
	 * Note: Calls to enableBar() and disableBar() are ref-counted.
	 * This means that the number of times disableBar() is called is memorized,
	 * and enableBar() will only have effect after it has been called as many times as disableBar() was called before.
	 * So, make sure to always call enableBar() the same number of times ou called disableBar().
	 * And please make sure to ALWAYS call disableBar() BEFORE enableBar().
	 * In the counter-case, another code could call disableBar() and EXCPECT the bar to be disabled. But it will not, because its call only canceled yours.
	 * Sometiems, you will absolutely need to call enableBar() before disableBar().
	 * For instance, MyWindow::show() calls enableBar() and MyWindow.hide() calls disableBar().
	 * This is the trick used to show the LikeBack button-bar of a Kontact plugin only when the main widget of that plugin is active.
	 * In this case, call disableBar() at the begin of your program, so the disable count will never be negative.
	 * @Note If the bar is enabled, it doesn't mean the bar is shown. For that, the developer (using showBarByDefault in the construcor)
	 *       or the user (by checking the checkbox in the comment dialog) have to explicitely show the bar.
	 */
	void disableBar(); // FIXME: rename to show/hide Bar ?

	/**
	 * Re-enable the button-bar one time.
	 * @see disableBar() for more information on how enabling/disabling works.
	 */
	void enableBar();

	/**
	 * Popup the comment dialog.
	 * With no parameter, it popups in the default configuration: the first type selected, empty message, current window path, and empty context.
	 * You can use the following parameters to customize how it should appears:
	 * @param type           Which radiobutton should be checked when poping up. AllButton, the default value, means the first available type will be checked.
	 * @param initialComment The text to put in the comment text area. Allows you to popup the dialog in some special circumstances,
	 *                       like to let the user report an internal error by populating the comment area with technical details useful for you to debug.
	 * @param windowPath     The window path to send with the comment. If empty (the default), the current window path is took.
	 *                       Separate window names with "~~". For instance "MainWindow~~NewFile~~FileOpen".
	 *                       If you popup the dialog after an error occurred, you can put the error name in that field (if the window path has no sense in that context).
	 *                       When the dialog is popuped up from the sendACommentAction() KAction, this value is "HelpMenu", because there is no way to know if the user
	 *                       is commenting a thing he found/thinked about in a sub-dialog.
	 * @param context        Not used for the moment. Will allow more fine-grained application status report.
	 */
	void execCommentDialog(Button type = AllButtons, const QString &initialComment = "", const QString &windowPath = "", const QString &context = "");

  private:
	LikeBackPrivate *d;
  private slots:
	void execCommentDialogFromHelp();

  protected:
  public:
	QString activeWindowPath();

	/**
	 * @Returns TODO
	 */
	Button buttons();
	/**
	 * @Returns true if the user has enabled the LikeBack bar for this version.
	 */
	bool barShown();
	void setUserWantsToShowBar(bool showBar);
  protected:
  public:
	KAboutData* aboutData();
	KConfig* config();


	void showInformationMessage();





	bool emailAddressAlreadyProvided();
	QString emailAddress(); /// << @Returns the email user address, or ask it to the user if he haven't provided or ignored it
	void setEmailAddress(const QString &address/*, bool userProvided = false*/); /// << Calling emailAddress() will ask it to the user the first time
	static bool isDevelopmentVersion(const QString &version = QString::null); /// << @Returns true if version is an alpha/beta/rc/svn/cvs version. Use kapp->aboutData()->version is @p version is empty
  private slots:
  public slots:
//	void showWhatsThisMessage();
	void askEmailAddress();
  private slots:
//	void beginFetchingEmail();
	void endFetchingEmailFrom(); // static QString fetchingEmail();
};

#endif // LIKEBACK_H
