#include "designs.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QSqlError>

Design::Design(const QString &nom, const QString &type, const QDate &dateCreation, int ide, const QString &description, const QByteArray &image)
    : nom(nom), type(type), dateCreation(dateCreation), ide(ide), description(description), image(image)
{
}

bool Design::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO designs (nom_design, type_design, date_creation, ide, description, image) "
                  "VALUES (:nom, :type, :date_creation, :ide, :description, :image)");
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":date_creation", dateCreation);
    query.bindValue(":ide", ide);
    query.bindValue(":description", description);
    query.bindValue(":image", image);

    if (!query.exec()) {
        qDebug() << "Erreur lors de l'ajout du design :" << query.lastError().text();
        return false;
    }
    return true;
}

bool Design::modifier(int id)
{
    QSqlQuery query;
    query.prepare("UPDATE designs SET nom_design = :nom, type_design = :type, date_creation = :date_creation, ide = :ide, description = :description, image = :image "
                  "WHERE id_design = :id");
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":date_creation", dateCreation);
    query.bindValue(":ide", ide);
    query.bindValue(":description", description);
    query.bindValue(":image", image);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur lors de l'ajout du design :" << query.lastError().text();
        return false;
    }
    return true;;
}

bool Design::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM designs WHERE id_design = :id");
    query.bindValue(":id", id);

    return query.exec();
}

QSqlQueryModel* Design::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery("SELECT id_design, nom_design, type_design, date_creation, ide, description, image FROM designs");
    return model;
}
QList<QPair<QString, QByteArray>> Design::getAllImagesWithNames()
{
    QList<QPair<QString, QByteArray>> images;
    QSqlQuery query;
    query.prepare("SELECT nom_design, image FROM DESIGNS");

    if (query.exec()) {
        while (query.next()) {
            QString name = query.value(0).toString();
            QByteArray imageData = query.value(1).toByteArray();
            images.append(qMakePair(name, imageData));
        }
    } else {
        qDebug() << "Erreur lors de la récupération des images:" << query.lastError();
    }

    return images;
}
