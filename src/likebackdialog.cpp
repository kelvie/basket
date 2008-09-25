
#include <QHttp>
#include <QHttpRequestHeader>

#include <KAboutData>
#include <KApplication>
#include <KDebug>
#include <KMessageBox>
#include <KPushButton>

#include "likebackdialog.h"

LikeBackDialog::LikeBackDialog(LikeBack::Button reason, const QString &initialComment, const QString &windowPath, const QString &context, LikeBack *likeBack)
: KDialog(kapp->activeWindow())
, Ui::LikeBackDialog()
, m_likeBack(likeBack)
, m_windowPath(windowPath)
, m_context(context)
{
  // KDialog Options
  setCaption( i18n( "Send a Comment to Developers" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  setObjectName( "_likeback_feedback_window_" );
  showButtonSeparator( true );

  // Set up the user interface
  QWidget *mainWidget = new QWidget( this );
  setupUi( mainWidget );
  setMainWidget( mainWidget );

  // The introduction message is long and will require a new minimum dialog size
  m_informationLabel->setText( introductionText() );
  setMinimumSize( minimumSizeHint() );


  m_typeGroup_ = new QButtonGroup( this );
  m_typeGroup_->addButton( likeRadio_,    LikeBack::Like    );
  m_typeGroup_->addButton( dislikeRadio_, LikeBack::Dislike );
  m_typeGroup_->addButton( bugRadio_,     LikeBack::Bug     );
  m_typeGroup_->addButton( featureRadio_, LikeBack::Feature );

  LikeBack::Button buttons = m_likeBack->buttons();

  // If no specific "reason" is provided, choose the first one:
  if( reason == LikeBack::AllButtons )
  {
    int firstButton = 0;
    if(                     (buttons & LikeBack::Like)    ) firstButton = LikeBack::Like;
    if( firstButton == 0 && (buttons & LikeBack::Dislike) ) firstButton = LikeBack::Dislike;
    if( firstButton == 0 && (buttons & LikeBack::Bug)     ) firstButton = LikeBack::Bug;
    if( firstButton == 0 && (buttons & LikeBack::Feature) ) firstButton = LikeBack::Feature;
    reason = (LikeBack::Button) firstButton;

    switch( firstButton )
    {
      case LikeBack::Like:    likeRadio_   ->setChecked( true ); break;
      case LikeBack::Dislike: dislikeRadio_->setChecked( true ); break;
      case LikeBack::Bug:     bugRadio_    ->setChecked( true ); break;
      case LikeBack::Feature: featureRadio_->setChecked( true ); break;
    }
  }

  // If no window path is provided, get the current active window path:
  if( m_windowPath.isEmpty() )
  {
    m_windowPath = LikeBack::activeWindowPath();
  }

  likeRadio_   ->setVisible( buttons & LikeBack::Like    );
  dislikeRadio_->setVisible( buttons & LikeBack::Dislike );
  bugRadio_    ->setVisible( buttons & LikeBack::Bug     );
  featureRadio_->setVisible( buttons & LikeBack::Feature );

  m_comment->setPlainText( initialComment );
  m_comment->setFocus();
}

LikeBackDialog::~LikeBackDialog()
{

}

QString LikeBackDialog::introductionText()
{
  QString text = "<p>" + i18n("Please provide a brief description of your opinion of %1.", m_likeBack->aboutData()->programName()) + " ";

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
      i18n("You may be able to use an <a href=\"%1\">online translation tool</a>."
                , "http://www.google.com/language_tools?hl=" + KGlobal::locale()->language()) + " ";

  // If both "I Like" and "I Dislike" buttons are shown and one is clicked:
  if ((m_likeBack->buttons() & LikeBack::Like) && (m_likeBack->buttons() & LikeBack::Dislike))
    text += i18n("To make the comments you send more useful in improving this application, try to send the same amount of positive and negative comments.") + " ";

  if (!(m_likeBack->buttons() & LikeBack::Feature))
    text += i18n("Do <b>not</b> ask for new features: your requests will be ignored.");

  return text;
}

void LikeBackDialog::slotDefault()
{
  m_likeBack->askEmailAddress();
}

void LikeBackDialog::slotOk()
{
  send();
}

void LikeBackDialog::commentChanged()
{
  KPushButton *sendButton = button(Ok);
  sendButton->setEnabled( ! m_comment->document()->isEmpty() );
}

void LikeBackDialog::send()
{
  QString type;
  QString emailAddress = m_likeBack->emailAddress();

  switch( m_typeGroup_->checkedId() )
  {
    case LikeBack::Like:    type = "Like";    break;
    case LikeBack::Dislike: type = "Dislike"; break;
    case LikeBack::Bug:     type = "Bug";     break;
    case LikeBack::Feature: type = "Feature"; break;
  }

  QString data = "protocol=" + QUrl::toPercentEncoding( "1.0" )                              + '&' +
                 "type="     + QUrl::toPercentEncoding( type )                               + '&' +
                 "version="  + QUrl::toPercentEncoding( m_likeBack->aboutData()->version() ) + '&' +
                 "locale="   + QUrl::toPercentEncoding( KGlobal::locale()->language() )      + '&' +
                 "window="   + QUrl::toPercentEncoding( m_windowPath )                       + '&' +
                 "context="  + QUrl::toPercentEncoding( m_context )                          + '&' +
                 "comment="  + QUrl::toPercentEncoding( m_comment->toPlainText() )           + '&' +
                 "email="    + QUrl::toPercentEncoding( emailAddress );
  QHttp *http = new QHttp(m_likeBack->hostName(), m_likeBack->hostPort());

  kDebug() << "http://" << m_likeBack->hostName() << ":" << m_likeBack->hostPort() << m_likeBack->remotePath();
  kDebug() << data;
  connect( http, SIGNAL(requestFinished(int, bool)), this, SLOT(requestFinished(int, bool)) );

  QHttpRequestHeader header("POST", m_likeBack->remotePath());
  header.setValue("Host", m_likeBack->hostName());
  header.setValue("Content-Type", "application/x-www-form-urlencoded");
  http->setHost(m_likeBack->hostName());
  http->request(header, data.toUtf8());

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
