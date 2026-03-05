#ifndef CLIENT_H
#define CLIENT_H
#include <QMap>
#include <QVector>
#include <QString>
#include <QDate>
#include <QSqlQueryModel>

class Client
{
public:
    Client();
    Client(const QString &nom, const QString &prenom, const QString &email,
           const QString &telephone, const QString &adresse, const QString &typeClient,
           const QDate &dateCreation);

    bool ajouter();
    bool modifier(int id, const QString &nom, const QString &prenom, const QString &email,
                  const QString &telephone, const QString &adresse, const QString &typeClient);
    bool supprimer(int id);
    static QSqlQueryModel* afficher();
    static QSqlQueryModel* afficherPointsFidelite();
    static void verifierEtEnvoyerSMS(const QString &nom, const QString &telephone, int pointsFidelite); // ✅ Ajouté
    static void envoyerSMS(const QString &numero, const QString &message); // ✅ Ajouté
    static void calculerPointsTousLesClients();
    static void notifierClientsFideles();

    struct ClientStats {
        int totalClients;
        int particuliers;
        int entreprises;
        QMap<QString, int> clientsParMois;
    };

    static QSqlQueryModel* rechercher(const QString &critere);
    static QSqlQueryModel* trierParNom(bool ascendant = true);
    static QSqlQueryModel* trierParDate(bool ascendant = true);
    static ClientStats calculerStats();
    void calculerPointsFidelite(int idClient);



private:
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString adresse;
    QString typeClient;
    QDate dateCreation;
    int pointsFidelite;  // Nouveau champ
    QString niveauFidelite;  // "Standard", "Fidèle", etc.
};
#endif // CLIENT_H
