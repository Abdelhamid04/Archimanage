#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QDate>
#include <QList>
#include <QMap>
#include <QPair>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QString>
#include <QVariant>
#include <Qt>

class employe
{
public:
    enum SortCriteria {
        ID = 0,
        Nom = 1,
        Prenom = 2,
        Salaire = 6,
        DateEmbauche = 9
    };

    employe();
    employe(int ide,
            const QString &nom,
            const QString &prenom,
            const QString &email,
            const QString &adresse,
            const QString &poste,
            const QString &salaire,
            const QString &statue,
            const QString &telephone,
            const QDate &dateDemabauche,
            const QString &password,
            const QString &question,
            const QString &reponse,
            const QString &imagePath = QString());

    bool ajouter();
    bool modifier(int ide);
    bool supprimer(int ide);

    QSqlQueryModel *afficher() const;
    QSqlQueryModel *trier(SortCriteria critere, Qt::SortOrder ordre) const;
    QSqlQueryModel *filtrer(const QString &poste, const QString &statut) const;
    QSqlQueryModel *rechercherMultiChamps(const QString &texte) const;

    int getIde() const;
    QString getNom() const;
    QString getPrenom() const;
    QString getEmail() const;
    QString getAdresse() const;
    QString getPoste() const;
    QString getSalaire() const;
    QString getStatue() const;
    QString getTelephone() const;
    QDate getDateDemabauche() const;
    QString getPassword() const;
    QString getQuestion() const;
    QString getReponse() const;
    QString getImagePath() const;

    QSqlError getLastError() const;

    static int getTotalEmployes();
    static QMap<QString, int> getStatutCounts();
    static QMap<QString, double> getSalaryStats();
    static QList<QPair<QString, QVariant>> getDataForChart(int chartType);

private:
    int m_ide;
    QString m_nom;
    QString m_prenom;
    QString m_email;
    QString m_adresse;
    QString m_poste;
    QString m_salaire;
    QString m_statue;
    QString m_telephone;
    QDate m_dateDemabauche;
    QString m_password;
    QString m_question;
    QString m_reponse;
    QString m_imagePath;
    QSqlError m_lastError;
};

#endif // EMPLOYE_H
