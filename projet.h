#ifndef PROJET_H
#define PROJET_H

#include <QString>
#include <QDate>
#include <QSqlQueryModel>
#include <QChart>
#include "arduino.h"

class Projet
{
public:
    Projet();


    Projet(int id, const QString &nom, const QString &description, int idClient,
           double budget, const QDate &dateDebut, const QDate &dateFin,
           const QString &statut,int id_architect );
    // Getters
    int getId() const;
    QString getNom() const;
    QString getDescription() const;
    int getIdClient() const;
    double getBudget() const;
    QDate getDateDebut() const;
    QDate getDateFin() const;
    QString getStatut() const;
    int getIdArchitecte() const;

    // Setters
    void setId(int id);
    void setNom(const QString &nom);
    void setDescription(const QString &description);
    void setIdClient(int idClient);
    void setBudget(double budget);
    void setDateDebut(const QDate &dateDebut);
    void setDateFin(const QDate &dateFin);
    void setStatut(const QString &statut);
    void setIdArchitecte(int idArchitecte);


    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int id);
    bool modifier();
    //tri
    QSqlQueryModel * trier(int test);

    QSqlQueryModel * recherche(QString id);

    //stat
    QChart * statproj();
    //
    //arduino

    bool existe();
    QString getClientByID();
    QString getArchitectByid();

private:
    int id_projet;
    QString nom_projet;
    QString description;
    int id_client;
    double budget_estime;
    QDate date_debut;
    QDate date_fin;
    QString statut_projet;

    int id_architecte;
    QChart *chart = nullptr;

};

#endif // PROJET_H
