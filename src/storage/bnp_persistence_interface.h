#ifndef BNP_PERSISTENCE_INTERFACE_H
#define BNP_PERSISTENCE_INTERFACE_H
// Include the basics

#include <QtCore/QObject>
#include <QVariantList>
#include <QSqlQueryModel>
/**
  * This is the plugin class. There will be only one instance of this class.
  */
class BNPPersistenceInterface
                        : public QObject
{
        Q_OBJECT
public:
        // Constructor
        explicit BNPPersistenceInterface ( QObject *parent,
                                           const QVariantList &args );
        // Destructor
        virtual ~BNPPersistenceInterface();
        /*return  a list of basketnote */
        QSqlQueryModel* getBasketList(void);
        /* return id and uuid for the new BasketNote
        */
        QStringList newBasketNote(QString name=tr("new basket note (change the name to find it)"), bool crypted=false);

private:
		/**
		  * TODO implement it on another plugin 
		  */
		void metaDataConnect(void);

};


#endif // BNP_PERSISTENCE_INTERFACE_H
