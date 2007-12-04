// Own includes
#include "bnp_persistence_interface.h"

#include <KPluginFactory>
#include <KPluginLoader>

/*
 * sql include part 
 */
#include <KAboutData>
#include <KStandardDirs>

class BNPPersistenceInterface;
// This macro defines a KPluginFactory subclass named BNPPersistenceInterfaceFactory. The second
// argument to the macro is code that is inserted into the constructor of the class.
// I our case all we need to do is register one plugin. If you want to have more
// than one plugin in the same library then you can register multiple plugin classes
// here. The registerPlugin function takes an optional QString parameter which is a
// keyword to uniquely identify the plugin then (it maps to X-KDE-PluginKeyword in the
// .desktop file).
K_PLUGIN_FACTORY(BNPPersistenceInterfaceFactory, 
 registerPlugin<BNPPersistenceInterface>();
)

// With the next macro call, the library exports version information about the
// Qt and KDE libraries being used and (most important) the entry symbol to get at
// the factory we defined above.
// The argument this macro takes is the constructor call of the above factory which
// provides two constructors. One which takes a KAboutData* and another one
// that takes two (optional) const char* parameters (Same as for KComponentData
// constructors).
// We put there the X-KDE-LibraryName.
// Is important to provide as last parameter "ktexteditor_plugins".
 static KAboutData createAboutData()
 {
     KAboutData aboutData("BNPPersistenceInterface", "BNPPersistenceInterface", ki18n("BNPPersistenceInterface"), "0.1",
             ki18n("a description of the plugin"), KAboutData::License_LGPL,
             ki18n("Copyright (C) 2007 PAtrice Broustal aka Perihelion"));
     aboutData.addAuthor(ki18n("Your Name"));
     return aboutData;
 }
 K_EXPORT_PLUGIN(BNPPersistenceInterfaceFactory(createAboutData()))

//K_EXPORT_PLUGIN(BNPPersistenceInterfaceFactory("bnp_persistence_interface", "bnp_plugins"))

// Constructor
BNPPersistenceInterface::BNPPersistenceInterface(QObject *parent, const QVariantList &args)
{
        // Avoid warning on compile time because of unused argument
        Q_UNUSED(args);
        
        metaDataConnect();
}


// Destructor
BNPPersistenceInterface::~BNPPersistenceInterface()
{
}

void BNPPersistenceInterface::metaDataConnect()
{
    QString myDB = KStandardDirs::locate("data", "basket/basketnotepads.db3");
    if(myDB.isNull())
    	myDB = KStandardDirs::locateLocal("data", "basket",true)+"/basketnotepads.db3";
    	
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(myDB);
}

QSqlQueryModel* BNPPersistenceInterface::getBasketList(void){
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery("SELECT * FROM basketnote");
    while (model->canFetchMore())
                model->fetchMore();
    return  model;	
}

QStringList BNPPersistenceInterface::newBasketNote(QString name, bool crypted){
return QStringList();	
}


#include "bnp_persistence_interface.moc"
/*
#include <KPluginFactory>
#include <KPluginLoader>
#include <plugininterface.h>

class MyPlugin;

K_PLUGIN_FACTORY(MyPluginFactory,
                  registerPlugin<MyPlugin>();
                 )
K_EXPORT_PLUGIN(MyPluginFactory("componentName", "catalogName"))

 // or:
static KAboutData createAboutData()
 {
     KAboutData aboutData("myplugin", "myplugin", ki18n("MyPlugin"), "0.1",
             ki18n("a description of the plugin"), KAboutData::License_LGPL,
             ki18n("Copyright (C) 2007 Your Name"));
     aboutData.addAuthor(ki18n("Your Name"), ...);
     return aboutData;
 }
 K_EXPORT_PLUGIN(MyPluginFactory(createAboutData()))

 class MyPlugin : public PluginInterface
 {
     ...
     KComponentData kcd = MyPluginFactory::componentData();
     ...
 };*/
