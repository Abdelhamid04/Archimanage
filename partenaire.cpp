#include "partenaire.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Partenaire::Partenaire() {}

Partenaire::Partenaire(int id, QString nom, QString prenom, QString email, QString telephone,
                       QString adresse, QString type, QString entreprise, QDate dateAjout, int ide)
    : id(id), nom(nom), prenom(prenom), email(email), telephone(telephone), adresse(adresse), type(type), entreprise(entreprise), dateAjout(dateAjout), ide(ide)
{
}
int Partenaire::getLastInsertedId() const
{
    QSqlQuery query("SELECT partenaire_seq.CURRVAL FROM DUAL");
    if (query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}
bool Partenaire::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO PARTENAIRE (ID_PARTENAIRE, NOM, PRENOM, EMAIL, TELEPHONE, "
                  "ADRESSE, TYPE, ENTREPRISE, DATE_AJOUT, IDE) "
                  "VALUES (PARTENAIRE_SEQ.NEXTVAL, :nom, :prenom, :email, :telephone, :adresse, :type, :entreprise, :dateAjout, :ide)");

    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":type", type);
    query.bindValue(":entreprise", entreprise);
    query.bindValue(":dateAjout", dateAjout);
    query.bindValue(":ide", ide);

    if (!query.exec()) {
        lastError_ = query.lastError();
        qDebug() << "Erreur d'insertion :" << lastError_.text();
        return false;
    }
    return true;
}

QSqlQueryModel* Partenaire::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_PARTENAIRE, NOM, PRENOM, EMAIL, TELEPHONE, ADRESSE, TYPE, ENTREPRISE, DATE_AJOUT, IDE FROM PARTENAIRE");
    return model;
}

bool Partenaire::modifier(int id)
{
    QSqlQuery query;
    query.prepare("UPDATE PARTENAIRE SET NOM=:nom, PRENOM=:prenom, EMAIL=:email, TELEPHONE=:telephone, "
                  "ADRESSE=:adresse, TYPE=:type, ENTREPRISE=:entreprise, DATE_AJOUT=:dateAjout, IDE=:ide WHERE ID_PARTENAIRE=:id");

    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":type", type);
    query.bindValue(":entreprise", entreprise);
    query.bindValue(":dateAjout", dateAjout);
    query.bindValue(":ide", ide);

    if (!query.exec()) {
        lastError_ = query.lastError();
        qDebug() << "Erreur de modification :" << lastError_.text();
        return false;
    }
    return true;
}

bool Partenaire::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM PARTENAIRE WHERE ID_PARTENAIRE = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        lastError_ = query.lastError();
        qDebug() << "Erreur de suppression :" << lastError_.text();
        return false;
    }
    return true;
}
