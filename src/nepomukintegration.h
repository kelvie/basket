#ifndef NEPOMUKINTEGRATION_H
#define NEPOMUKINTEGRATION_H

#include <QString>
#include <QDomDocument>

class nepomukintegration
{
public:
    nepomukintegration(){};
    static bool updateMetadata(const QString &fullPath, const QDomDocument &document);
    static bool deleteMetadata(const QString &fullPath);
};

#endif // NEPOMUKINTEGRATION_H
