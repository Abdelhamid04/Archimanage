#ifndef PARTENAIRE_H
#define PARTENAIRE_H

#include <QString>
#include <QDate>
#include <QSqlQueryModel>
#include <QSqlError>

class Partenaire
{
public:
    Partenaire();
    Partenaire(int id, QString nom, QString prenom, QString email, QString telephone,
               QString adresse, QString type, QString entreprise, QDate dateAjout, int ide);

    bool ajouter();
    bool modifier(int id);
    bool supprimer(int id);
    QSqlQueryModel* afficher();
    int getLastInsertedId() const;
    QSqlError lastError() const { return lastError_; }

private:
    int id;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString adresse;
    QString type;
    QString entreprise;
    QDate dateAjout;
    int ide;
    QSqlError lastError_;
};

#endif // PARTENAIRE_H
