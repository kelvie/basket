
#include "basket_part.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <bnpview.h>
#include <aboutdata.h>
#include <kparts/genericfactory.h>
#include <kparts/statusbarextension.h>
#include "basketstatusbar.h"

typedef KParts::GenericFactory< BasketPart > BasketFactory;
K_EXPORT_COMPONENT_FACTORY( libbasketpart, BasketFactory )

BasketPart::BasketPart( QWidget *parentWidget, const char *,
						QObject *parent, const char *name, const QStringList & )
	: KParts::ReadWritePart(parent, name)
{
  // we need an instance
	setInstance( BasketFactory::instance() );

	BasketStatusBar* bar = new BasketStatusBar(new KParts::StatusBarExtension(this));
  // this should be your custom internal widget
	m_view = new BNPView(parentWidget, "BNPViewPart", this, actionCollection(), bar);
	connect(m_view, SIGNAL(setWindowCaption(const QString &)), this, SLOT(setCaption(const QString &)));
	connect(m_view, SIGNAL(showPart()), this, SIGNAL(showPart()));
	m_view->setFocusPolicy(QWidget::ClickFocus);

  // notify the part that this is our internal widget
	setWidget(m_view);

  // set our XML-UI resource file
	setXMLFile("basket_part.rc");

  // we are read-write by default
	setReadWrite(true);

  // we are not modified since we haven't done anything yet
	setModified(false);
}

BasketPart::~BasketPart()
{}

void BasketPart::setReadWrite(bool rw)
{
  // TODO: notify your internal widget of the read-write state
	ReadWritePart::setReadWrite(rw);
}

void BasketPart::setModified(bool modified)
{
  // in any event, we want our parent to do it's thing
	ReadWritePart::setModified(modified);
}

bool BasketPart::openFile()
{
  // TODO
	return false;
}

bool BasketPart::saveFile()
{
  //TODO
	return false;
}

KAboutData *BasketPart::createAboutData()
{
	return new AboutData();
}

void BasketPart::setCaption(const QString &caption)
{
	emit setWindowCaption(caption);
}

#include "basket_part.moc"
