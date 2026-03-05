#ifndef DESIGNS_H
#define DESIGNS_H

#include <QString>
#include <QDate>
#include <QByteArray>
#include <QSqlQueryModel>

class Design
{
public:
    Design(const QString &nom, const QString &type, const QDate &dateCreation, int ide, const QString &description, const QByteArray &image);
    bool ajouter();
    bool modifier(int id);
    bool supprimer(int id);
    static QSqlQueryModel* afficher();
    static QList<QPair<QString, QByteArray>> getAllImagesWithNames(); // Nouvelle méthode
private:
    QString nom;
    QString type;
    QDate dateCreation;
    int ide;
    QString description;
    QByteArray image;
};

#endif // DESIGNS_H
