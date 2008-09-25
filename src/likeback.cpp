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
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QVBoxLayout>
#include <kpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <kguiitem.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <KDialog>
#include <qhttp.h>
#include <kurl.h>
#include <kinputdialog.h>
#include <qvalidator.h>
#include <qaction.h>
#include <kdebug.h>
#include <QButtonGroup>

#include <pwd.h>

#include <kglobal.h>

#include "likeback.h"
#include "likebackbar.h"
#include "likebackdialog.h"
#include "likeback_p.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <kuser.h>
#include <QDesktopWidget>


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
//   Q3MimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_like",    likeIcon);
//   Q3MimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_dislike", dislikeIcon);
//   Q3MimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_bug",     bugIcon);
//   Q3MimeSourceFactory::defaultFactory()->setPixmap("likeback_icon_feature", featureIcon);

  // Show a message reflecting the allowed types of comment:
  Button buttons = d->buttons;
  int nbButtons = (buttons & Like    ? 1 : 0) +
                  (buttons & Dislike ? 1 : 0) +
                  (buttons & Bug     ? 1 : 0) +
                  (buttons & Feature ? 1 : 0);
  KMessageBox::information(0,
    "<p><b>" + (isDevelopmentVersion(d->aboutData->version()) ?
      i18n("Welcome to this testing version of %1.", d->aboutData->programName()) :
      i18n("Welcome to %1.", d->aboutData->programName())
    ) + "</b></p>"
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
/*  Q3MimeSourceFactory::defaultFactory()->setData("likeback_icon_like",    0L);
  Q3MimeSourceFactory::defaultFactory()->setData("likeback_icon_dislike", 0L);
  Q3MimeSourceFactory::defaultFactory()->setData("likeback_icon_bug",     0L);
  Q3MimeSourceFactory::defaultFactory()->setData("likeback_icon_feature", 0L);*/
}

QString LikeBack::activeWindowPath()
{
  // Compute the window hierarchy (from the latest to the oldest):
  QStringList windowNames;
  QWidget *window = kapp->activeWindow();
  while (window) {
    QString name = window->objectName();
    // Append the class name to the window name if it is unnamed:
    if (name == "unnamed")
      name += QString(":") + window->metaObject()->className();
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
  return version.indexOf( "alpha", 0, Qt::CaseInsensitive ) != -1 ||
         version.indexOf( "beta",  0, Qt::CaseInsensitive ) != -1 ||
         version.indexOf( "rc",    0, Qt::CaseInsensitive ) != -1 ||
         version.indexOf( "svn",   0, Qt::CaseInsensitive ) != -1 ||
         version.indexOf( "cvs",   0, Qt::CaseInsensitive ) != -1;
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
//  m_configureEmail->setEnabled(false);
}*/

/**
 * Code from KBugReport::slotSetFrom() in kdeui/kbugreport.cpp:
 */
void LikeBack::fetchUserEmail()
{
//  delete m_process;
//  m_process = 0;
//  m_configureEmail->setEnabled(true);

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
//  m_from->setText( fromaddr );
}

