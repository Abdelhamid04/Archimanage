#include "employe.h"

#include <QSqlQuery>
#include <QSqlRecord>

namespace {
QString sortColumnForCriteria(employe::SortCriteria critere)
{
    switch (critere) {
    case employe::ID: return "IDE";
    case employe::Nom: return "NOM";
    case employe::Prenom: return "PRENOM";
    case employe::Salaire: return "SALAIRE";
    case employe::DateEmbauche: return "DATE_DEMBAUCHE";
    }
    return "IDE";
}
}

employe::employe()
    : m_ide(0)
{
}

employe::employe(int ide,
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
                 const QString &imagePath)
    : m_ide(ide)
    , m_nom(nom)
    , m_prenom(prenom)
    , m_email(email)
    , m_adresse(adresse)
    , m_poste(poste)
    , m_salaire(salaire)
    , m_statue(statue)
    , m_telephone(telephone)
    , m_dateDemabauche(dateDemabauche)
    , m_password(password)
    , m_question(question)
    , m_reponse(reponse)
    , m_imagePath(imagePath)
{
}

bool employe::ajouter()
{
    QSqlQuery query;

    if (m_ide > 0) {
        query.prepare(
            "INSERT INTO employe "
            "(IDE, NOM, PRENOM, EMAIL, ADRESSE, POSTE, SALAIRE, STATUE, TELEPHONE, DATE_DEMBAUCHE, PASSWORD, QUESTION, REPONSE, IMAGE_PATH) "
            "VALUES (:ide, :nom, :prenom, :email, :adresse, :poste, :salaire, :statue, :telephone, :date_dembauche, :password, :question, :reponse, :image_path)");
        query.bindValue(":ide", m_ide);
    } else {
        query.prepare(
            "INSERT INTO employe "
            "(IDE, NOM, PRENOM, EMAIL, ADRESSE, POSTE, SALAIRE, STATUE, TELEPHONE, DATE_DEMBAUCHE, PASSWORD, QUESTION, REPONSE, IMAGE_PATH) "
            "VALUES (EMPLOYE_SEQ.NEXTVAL, :nom, :prenom, :email, :adresse, :poste, :salaire, :statue, :telephone, :date_dembauche, :password, :question, :reponse, :image_path)");
    }

    query.bindValue(":nom", m_nom);
    query.bindValue(":prenom", m_prenom);
    query.bindValue(":email", m_email);
    query.bindValue(":adresse", m_adresse);
    query.bindValue(":poste", m_poste);
    query.bindValue(":salaire", m_salaire);
    query.bindValue(":statue", m_statue);
    query.bindValue(":telephone", m_telephone);
    query.bindValue(":date_dembauche", m_dateDemabauche);
    query.bindValue(":password", m_password);
    query.bindValue(":question", m_question);
    query.bindValue(":reponse", m_reponse);
    query.bindValue(":image_path", m_imagePath);

    if (query.exec()) {
        return true;
    }

    // Fallback when no Oracle sequence is configured.
    m_lastError = query.lastError();
    if (m_ide == 0) {
        QSqlQuery fallback;
        fallback.prepare(
            "INSERT INTO employe "
            "(NOM, PRENOM, EMAIL, ADRESSE, POSTE, SALAIRE, STATUE, TELEPHONE, DATE_DEMBAUCHE, PASSWORD, QUESTION, REPONSE, IMAGE_PATH) "
            "VALUES (:nom, :prenom, :email, :adresse, :poste, :salaire, :statue, :telephone, :date_dembauche, :password, :question, :reponse, :image_path)");

        fallback.bindValue(":nom", m_nom);
        fallback.bindValue(":prenom", m_prenom);
        fallback.bindValue(":email", m_email);
        fallback.bindValue(":adresse", m_adresse);
        fallback.bindValue(":poste", m_poste);
        fallback.bindValue(":salaire", m_salaire);
        fallback.bindValue(":statue", m_statue);
        fallback.bindValue(":telephone", m_telephone);
        fallback.bindValue(":date_dembauche", m_dateDemabauche);
        fallback.bindValue(":password", m_password);
        fallback.bindValue(":question", m_question);
        fallback.bindValue(":reponse", m_reponse);
        fallback.bindValue(":image_path", m_imagePath);

        if (fallback.exec()) {
            return true;
        }
        m_lastError = fallback.lastError();
    }

    return false;
}

bool employe::modifier(int ide)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE employe SET "
        "NOM = :nom, PRENOM = :prenom, EMAIL = :email, ADRESSE = :adresse, "
        "POSTE = :poste, SALAIRE = :salaire, STATUE = :statue, TELEPHONE = :telephone, "
        "DATE_DEMBAUCHE = :date_dembauche, PASSWORD = :password, QUESTION = :question, "
        "REPONSE = :reponse, IMAGE_PATH = :image_path "
        "WHERE IDE = :ide");

    query.bindValue(":nom", m_nom);
    query.bindValue(":prenom", m_prenom);
    query.bindValue(":email", m_email);
    query.bindValue(":adresse", m_adresse);
    query.bindValue(":poste", m_poste);
    query.bindValue(":salaire", m_salaire);
    query.bindValue(":statue", m_statue);
    query.bindValue(":telephone", m_telephone);
    query.bindValue(":date_dembauche", m_dateDemabauche);
    query.bindValue(":password", m_password);
    query.bindValue(":question", m_question);
    query.bindValue(":reponse", m_reponse);
    query.bindValue(":image_path", m_imagePath);
    query.bindValue(":ide", ide);

    const bool ok = query.exec();
    if (!ok) {
        m_lastError = query.lastError();
    }
    return ok;
}

bool employe::supprimer(int ide)
{
    QSqlQuery query;
    query.prepare("DELETE FROM employe WHERE IDE = :ide");
    query.bindValue(":ide", ide);
    const bool ok = query.exec();
    if (!ok) {
        m_lastError = query.lastError();
    }
    return ok;
}

QSqlQueryModel *employe::afficher() const
{
    auto *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT IDE, NOM, PRENOM, EMAIL, ADRESSE, POSTE, SALAIRE, STATUE, TELEPHONE, DATE_DEMBAUCHE, QUESTION, REPONSE, IMAGE_PATH "
        "FROM employe");
    return model;
}

QSqlQueryModel *employe::trier(SortCriteria critere, Qt::SortOrder ordre) const
{
    auto *model = new QSqlQueryModel();
    const QString order = (ordre == Qt::AscendingOrder) ? "ASC" : "DESC";
    const QString sql = QString(
                            "SELECT IDE, NOM, PRENOM, EMAIL, ADRESSE, POSTE, SALAIRE, STATUE, TELEPHONE, DATE_DEMBAUCHE, QUESTION, REPONSE, IMAGE_PATH "
                            "FROM employe ORDER BY %1 %2")
                            .arg(sortColumnForCriteria(critere), order);
    model->setQuery(sql);
    return model;
}

QSqlQueryModel *employe::filtrer(const QString &poste, const QString &statut) const
{
    auto *model = new QSqlQueryModel();
    QSqlQuery query;

    QString sql =
        "SELECT IDE, NOM, PRENOM, EMAIL, ADRESSE, POSTE, SALAIRE, STATUE, TELEPHONE, DATE_DEMBAUCHE, QUESTION, REPONSE, IMAGE_PATH "
        "FROM employe WHERE 1=1";

    if (!poste.isEmpty() && poste != "Tous") {
        sql += " AND POSTE = :poste";
    }
    if (!statut.isEmpty() && statut != "Tous") {
        sql += " AND STATUE = :statut";
    }

    query.prepare(sql);
    if (!poste.isEmpty() && poste != "Tous") {
        query.bindValue(":poste", poste);
    }
    if (!statut.isEmpty() && statut != "Tous") {
        query.bindValue(":statut", statut);
    }

    query.exec();
    model->setQuery(std::move(query));
    return model;
}

QSqlQueryModel *employe::rechercherMultiChamps(const QString &texte) const
{
    auto *model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare(
        "SELECT IDE, NOM, PRENOM, EMAIL, ADRESSE, POSTE, SALAIRE, STATUE, TELEPHONE, DATE_DEMBAUCHE, QUESTION, REPONSE, IMAGE_PATH "
        "FROM employe "
        "WHERE LOWER(NOM) LIKE :txt "
        "OR LOWER(PRENOM) LIKE :txt "
        "OR LOWER(EMAIL) LIKE :txt "
        "OR LOWER(POSTE) LIKE :txt "
        "OR LOWER(STATUE) LIKE :txt");
    query.bindValue(":txt", "%" + texte.toLower() + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

int employe::getIde() const { return m_ide; }
QString employe::getNom() const { return m_nom; }
QString employe::getPrenom() const { return m_prenom; }
QString employe::getEmail() const { return m_email; }
QString employe::getAdresse() const { return m_adresse; }
QString employe::getPoste() const { return m_poste; }
QString employe::getSalaire() const { return m_salaire; }
QString employe::getStatue() const { return m_statue; }
QString employe::getTelephone() const { return m_telephone; }
QDate employe::getDateDemabauche() const { return m_dateDemabauche; }
QString employe::getPassword() const { return m_password; }
QString employe::getQuestion() const { return m_question; }
QString employe::getReponse() const { return m_reponse; }
QString employe::getImagePath() const { return m_imagePath; }

QSqlError employe::getLastError() const
{
    return m_lastError;
}

int employe::getTotalEmployes()
{
    QSqlQuery query("SELECT COUNT(*) FROM employe");
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QMap<QString, int> employe::getStatutCounts()
{
    QMap<QString, int> out;
    QSqlQuery query("SELECT STATUE, COUNT(*) FROM employe GROUP BY STATUE");
    while (query.next()) {
        out.insert(query.value(0).toString(), query.value(1).toInt());
    }
    return out;
}

QMap<QString, double> employe::getSalaryStats()
{
    QMap<QString, double> out;
    out.insert("min", 0.0);
    out.insert("max", 0.0);
    out.insert("avg", 0.0);

    QSqlQuery query("SELECT MIN(SALAIRE), MAX(SALAIRE), AVG(SALAIRE) FROM employe");
    if (query.next()) {
        out["min"] = query.value(0).toDouble();
        out["max"] = query.value(1).toDouble();
        out["avg"] = query.value(2).toDouble();
    }
    return out;
}

QList<QPair<QString, QVariant>> employe::getDataForChart(int chartType)
{
    QList<QPair<QString, QVariant>> data;

    QString queryStr;
    if (chartType == 0) {
        queryStr = "SELECT STATUE, COUNT(*) FROM employe GROUP BY STATUE";
    } else {
        queryStr = "SELECT POSTE, COUNT(*) FROM employe GROUP BY POSTE";
    }

    QSqlQuery query(queryStr);
    while (query.next()) {
        data.append({query.value(0).toString(), query.value(1)});
    }
    return data;
}
