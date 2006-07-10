
#ifndef _BASKETPART_H_
#define _BASKETPART_H_

#include <kparts/part.h>
#include <kparts/factory.h>

class QWidget;
class QPainter;
class KURL;
class QMultiLineEdit;
class BNPView;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Petri Damsten <petri.damsten@iki.fi>
 * @version 0.1
 */
class BasketPart : public KParts::ReadWritePart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    BasketPart(QWidget *parentWidget, const char *widgetName,
			   QObject *parent, const char *name, const QStringList &);

    /**
     * Destructor
     */
    virtual ~BasketPart();

    /**
     * This is a virtual function inherited from KParts::ReadWritePart.
     * A shell will use this to inform this Part if it should act
     * read-only
     */
    virtual void setReadWrite(bool rw);

    /**
     * Reimplemented to disable and enable Save action
     */
    virtual void setModified(bool modified);

	static KAboutData *createAboutData();

  protected:
    /**
	 * This must be implemented by each part
	 */
	virtual bool openFile();

    /**
		 * This must be implemented by each read-write part
	 */
	virtual bool saveFile();

  protected slots:
	  void setCaption(const QString &caption);

  private:
    BNPView *m_view;
};

#endif // _BASKETPART_H_
