#include "client.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QRegularExpression>
#include <QSqlError> // Ajoutez cette ligne pour inclure la définition de QSqlError
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QByteArray>

// Définition du constructeur par défaut
Client::Client() {}

Client::Client(const QString &nom, const QString &prenom, const QString &email,
               const QString &telephone, const QString &adresse, const QString &typeClient,
               const QDate &dateCreation)
    : nom(nom), prenom(prenom), email(email), telephone(telephone),
    adresse(adresse), typeClient(typeClient), dateCreation(dateCreation),
    pointsFidelite(0), niveauFidelite("Standard") // Initialisation des nouveaux champs
{
}


bool Client::ajouter()
{
    // Expressions régulières pour la validation
    QRegularExpression nameRegex("^[A-Za-zÀ-ÿ][A-Za-zÀ-ÿ' -]{1,11}$"); // Nom et prénom : pas de chiffre au début, max 12 caractères
    QRegularExpression emailRegex(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)"); // Email valide
    QRegularExpression phoneRegex("^\\d{8}$"); // Numéro de téléphone : 8 chiffres (Tunisie)
    QRegularExpression addressRegex("^[A-Za-z0-9À-ÿ][A-Za-z0-9À-ÿ' ,.-]{4,99}$"); // Adresse : min 5 caractères

    // Vérifications des champs
    if (!nameRegex.match(nom).hasMatch()) {
        qDebug() << "Erreur: Nom invalide (12 caractères max, pas de chiffres au début).";
        return false;
    }
    if (!nameRegex.match(prenom).hasMatch()) {
        qDebug() << "Erreur: Prénom invalide (12 caractères max, pas de chiffres au début).";
        return false;
    }
    if (!emailRegex.match(email).hasMatch()) {
        qDebug() << "Erreur: Email invalide. Forme d'email incorrecte.";
        return false;
    }
    if (!phoneRegex.match(telephone).hasMatch()) {
        qDebug() << "Erreur: Numéro de téléphone invalide (exactement 8 chiffres).";
        return false;
    }
    if (!addressRegex.match(adresse).hasMatch()) {
        qDebug() << "Erreur: Adresse invalide (5-100 caractères, pas de chiffre au début).";
        return false;
    }
    if (typeClient.isEmpty()) {
        qDebug() << "Erreur: Type de client obligatoire.";
        return false;
    }

    // Requête SQL pour ajouter un client
    QSqlQuery query;
    query.prepare("INSERT INTO clients (nom, prenom, email, telephone, adresse, type_client, date_creation) "
                  "VALUES (:nom, :prenom, :email, :telephone, :adresse, :type_client, :date_creation)");

    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":type_client", typeClient);
    query.bindValue(":date_creation", dateCreation);
    query.bindValue(":points_fidelite", 0);  // Initialisation à 0
    if (!query.exec()) {
        qDebug() << "Erreur lors de l'ajout du client :" << query.lastError().text();
        return false;
    }
    int newId = query.lastInsertId().toInt();
    qDebug() << "Nouveau client ajouté avec ID:" << newId;

    // Initialisation des points de fidélité
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE clients SET points_fidelite = 0 WHERE id_client = ?");
    updateQuery.addBindValue(newId);
    updateQuery.exec();

    return true;
}

// Définition de la méthode modifier
bool Client::modifier(int id, const QString &nom, const QString &prenom, const QString &email,
                      const QString &telephone, const QString &adresse, const QString &typeClient)
{
    QSqlQuery query;
    query.prepare("UPDATE clients SET nom = :nom, prenom = :prenom, email = :email, telephone = :telephone, "
                  "adresse = :adresse, type_client = :type_client WHERE id_client = :id");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":type_client", typeClient);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur lors de la modification du client :" << query.lastError().text();
        return false;
    }
    return true;
}

bool Client::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM clients WHERE id_client = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur lors de la suppression du client :" << query.lastError().text();
        return false;
    }
    return true;
}

QSqlQueryModel* Client::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery("SELECT id_client, nom, prenom, email, telephone, adresse, type_client, date_creation FROM clients");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur lors de l'affichage des clients :" << model->lastError().text();
    }

    return model;
}
Client::ClientStats Client::calculerStats() {
    ClientStats stats;
    QSqlQuery query;

    // Total clients
    query.exec("SELECT COUNT(*) FROM clients");
    if(query.next()) stats.totalClients = query.value(0).toInt();

    // Nombre de particuliers et entreprises
    query.exec("SELECT type_client, COUNT(*) FROM clients GROUP BY type_client");
    while(query.next()) {
        QString type = query.value(0).toString();
        if(type.toLower() == "particulier") stats.particuliers = query.value(1).toInt();
        else if(type.toLower() == "entreprise") stats.entreprises = query.value(1).toInt();
    }

    return stats;
}

QSqlQueryModel* Client::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare("SELECT id_client, nom, prenom, email, telephone, adresse, type_client, date_creation "
                  "FROM clients "
                  "WHERE nom LIKE :critere OR "
                  "prenom LIKE :critere OR "
                  "email LIKE :critere OR "
                  "telephone LIKE :critere OR "
                  "adresse LIKE :critere OR "
                  "type_client LIKE :critere");
    query.bindValue(":critere", "%" + critere + "%");

    if (!query.exec()) {
        qDebug() << "Erreur lors de la recherche :" << query.lastError().text();
    }

    model->setQuery(std::move(query));

    return model;
}

QSqlQueryModel* Client::trierParNom(bool ascendant) {
    QSqlQueryModel *model = new QSqlQueryModel();
    QString queryStr = "SELECT id_client, nom, prenom, email, telephone, adresse, type_client, date_creation "
                       "FROM clients ORDER BY nom ";
    queryStr += ascendant ? "ASC" : "DESC";

    QSqlQuery query;
    query.prepare(queryStr);  // Utilisez prepare() pour une meilleure gestion
    if (!query.exec()) {
        qDebug() << "Erreur lors du tri par nom :" << query.lastError().text();
        delete model;  // Nettoyage en cas d'erreur
        return nullptr;
    }

    model->setQuery(std::move(query));  // Déplacement de la requête
    return model;
}

QSqlQueryModel* Client::trierParDate(bool ascendant) {
    QSqlQueryModel *model = new QSqlQueryModel();
    QString queryStr = "SELECT id_client, nom, prenom, email, telephone, adresse, type_client, date_creation "
                       "FROM clients ORDER BY date_creation ";
    queryStr += ascendant ? "ASC" : "DESC";

    QSqlQuery query;
    query.prepare(queryStr);
    if (!query.exec()) {
        qDebug() << "Erreur lors du tri par date :" << query.lastError().text();
        delete model;
        return nullptr;
    }

    model->setQuery(std::move(query));  // Déplacement de la requête
    return model;
}

QSqlQueryModel* Client::afficherPointsFidelite()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT id_client, points_fidelite FROM clients");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur lors de l'affichage des points de fidélité :" << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID Client"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Points Fidélité"));

    return model;
}

void Client::calculerPointsFidelite(int idClient)
{
    int nbProjets = 0;
    int anciennete = 0;
    int totalPoints = 0;

    QSqlQuery query1;
    query1.prepare("SELECT DATE_CREATION FROM CLIENTS WHERE ID_CLIENT = :id");
    query1.bindValue(":id", idClient);

    if (query1.exec() && query1.next()) {
        QDate dateCreationClient = query1.value(0).toDate();
        anciennete = dateCreationClient.daysTo(QDate::currentDate()) / 365;
    } else {
        qWarning() << "Erreur : impossible de récupérer la date de création du client";
        return;
    }

    // 2. Compter les projets associés
    QSqlQuery query2;
    query2.prepare("SELECT COUNT(*) FROM PROJET WHERE ID_CLIENT = :id");
    query2.bindValue(":id", idClient);

    if (query2.exec() && query2.next()) {
        nbProjets = query2.value(0).toInt();
    } else {
        qWarning() << "Erreur : impossible de compter les projets du client";
        return;
    }

    // 3. Calcul des points
    totalPoints = (anciennete * 10) + (nbProjets * 20);

    // 4. Mise à jour des points fidélité dans la BDD
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE CLIENTS SET POINTS_FIDELITE = :points WHERE ID_CLIENT = :id");
    updateQuery.bindValue(":points", totalPoints);
    updateQuery.bindValue(":id", idClient);

    if (!updateQuery.exec()) {
        qWarning() << "Erreur : mise à jour des points de fidélité échouée";
    } else {
        qDebug() << "Points fidélité mis à jour :" << totalPoints;
    }
}


void Client::calculerPointsTousLesClients()
{
    QSqlQuery query("SELECT ID_CLIENT, DATE_CREATION FROM CLIENTS");

    while (query.next()) {
        int idClient = query.value("ID_CLIENT").toInt();
        QDate dateCreationClient = query.value("DATE_CREATION").toDate();

        int anciennete = dateCreationClient.daysTo(QDate::currentDate()) / 365;

        // Compter les projets
        QSqlQuery queryProjets;
        queryProjets.prepare("SELECT COUNT(*) FROM PROJET WHERE ID_CLIENT = :id");
        queryProjets.bindValue(":id", idClient);

        int nbProjets = 0;
        if (queryProjets.exec() && queryProjets.next()) {
            nbProjets = queryProjets.value(0).toInt();
        } else {
            qWarning() << "Erreur : impossible de compter les projets du client ID" << idClient;
            continue;
        }

        // Calcul des points
        int totalPoints = (anciennete * 10) + (nbProjets * 20);

        // Mise à jour dans la base
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE CLIENTS SET POINTS_FIDELITE = :points WHERE ID_CLIENT = :id");
        updateQuery.bindValue(":points", totalPoints);
        updateQuery.bindValue(":id", idClient);

        if (!updateQuery.exec()) {
            qWarning() << "Erreur : mise à jour échouée pour le client ID" << idClient;
        } else {
            qDebug() << "Client ID" << idClient << ": Points fidélité =" << totalPoints;
        }
    }
}


void Client::verifierEtEnvoyerSMS(const QString &nom, const QString &telephone, int pointsFidelite) {
    if (pointsFidelite >= 250) {
        QString message = QString("Félicitations %1 ! Vous êtes désormais un client fidèle avec %2 points. Merci pour votre confiance !")
                              .arg(nom)
                              .arg(pointsFidelite);
        Client::envoyerSMS(telephone, message);
    }
}
void Client::envoyerSMS(const QString &numero, const QString &message) {
            QNetworkAccessManager *manager = new QNetworkAccessManager();
            const QString accountSid = QString::fromUtf8(qgetenv("TWILIO_ACCOUNT_SID"));
            const QString authToken = QString::fromUtf8(qgetenv("TWILIO_AUTH_TOKEN"));
            const QString from = QString::fromUtf8(qgetenv("TWILIO_FROM_NUMBER"));
            if (accountSid.isEmpty() || authToken.isEmpty() || from.isEmpty()) {
                qDebug() << "Twilio configuration missing (TWILIO_ACCOUNT_SID/TWILIO_AUTH_TOKEN/TWILIO_FROM_NUMBER).";
                manager->deleteLater();
                return;
            }

            QUrl url(QString("https://api.twilio.com/2010-04-01/Accounts/%1/Messages.json").arg(accountSid));

            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

            QString to = numero;

            // ✅ Formatage du numéro en international si nécessaire
            if (!to.startsWith("+")) {
                to = "+216" + to;  // Par exemple pour la Tunisie
            }

            QUrlQuery postData;
            postData.addQueryItem("From", from);
            postData.addQueryItem("To", to);
            postData.addQueryItem("Body", message);

            // ✅ Authentification Basic : SID:Token
            QByteArray credentials = QString("%1:%2").arg(accountSid, authToken).toUtf8();
            request.setRawHeader("Authorization", "Basic " + credentials.toBase64());

    // ✅ Réponse
    QObject::connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "✅ SMS envoyé avec succès à" << to;
        } else {
            qDebug() << "❌ Erreur d'envoi SMS:" << reply->errorString();
            qDebug() << "➡ Réponse Twilio:" << reply->readAll();
        }
        reply->deleteLater();
        manager->deleteLater();  // Nettoyage
    });

    manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}


void Client::notifierClientsFideles()
{
    QSqlQuery query;
    query.prepare("SELECT TELEPHONE, NOM, POINTS_FIDELITE FROM CLIENTS WHERE POINTS_FIDELITE > 20");

    if (query.exec()) {
        while (query.next()) {
            QString numero = query.value("TELEPHONE").toString();
            QString nom = query.value("NOM").toString();
            int points = query.value("POINTS_FIDELITE").toInt();

            QString message = QString("[Archimanage]\n"
                                      "Cher(e) %1,\n\n"
                                      "Nous tenons à vous remercier pour votre fidélité et votre confiance.\n"
                                      "Votre solde actuel est de %2 points de fidélité.\n\n"
                                      "Ces points vous permettent de bénéficier d'avantages exclusifs.\n"
                                      "N'hésitez pas à nous contacter pour plus d'informations.\n\n"
                                      "Cordialement,\n"
                                      "L'équipe Archimanage")
                                  .arg(nom)
                                  .arg(points);
            envoyerSMS(numero, message);
        }
    } else {
        qDebug() << "Erreur lors de la récupération des clients fidèles:" << query.lastError().text();
    }
}
