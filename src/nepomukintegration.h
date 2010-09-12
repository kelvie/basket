#ifndef NEPOMUKINTEGRATION_H
#define NEPOMUKINTEGRATION_H

#include <QString>
#include <QDomDocument>

class nepomukintegration
{
public:
    nepomukintegration(){};
    static bool updateMetadata(const QString &fullPath, const QDomDocument &document);
};

#endif // NEPOMUKINTEGRATION_H
