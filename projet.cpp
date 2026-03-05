#include "projet.h"
#include <QSqlQuery>
#include <QVariant>
#include <Qbarset>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QPieSeries>


Projet::Projet() {}

Projet::Projet(int id, const QString &nom, const QString &description, int idClient,
               double budget, const QDate &dateDebut, const QDate &dateFin,
               const QString &statut, int id_architect)
    : id_projet(id), nom_projet(nom), description(description), id_client(idClient),
    budget_estime(budget), date_debut(dateDebut), date_fin(dateFin),
    statut_projet(statut), id_architecte(id_architect){}


//getters
int Projet::getId() const {
    return id_projet;
}

QString Projet::getNom() const {
    return nom_projet;
}

QString Projet::getDescription() const {
    return description;
}

int Projet::getIdClient() const {
    return id_client;
}

double Projet::getBudget() const {
    return budget_estime;
}

QDate Projet::getDateDebut() const {
    return date_debut;
}

QDate Projet::getDateFin() const {
    return date_fin;
}

QString Projet::getStatut() const {
    return statut_projet;
}

int Projet::getIdArchitecte() const {
    return id_architecte;
}
//setters
void Projet::setId(int id) {
    id_projet = id;
}

void Projet::setNom(const QString &nom) {
    nom_projet = nom;
}

void Projet::setDescription(const QString &description) {
    this->description = description;
}

void Projet::setIdClient(int idClient) {
    id_client = idClient;
}

void Projet::setBudget(double budget) {
    budget_estime = budget;
}

void Projet::setDateDebut(const QDate &dateDebut) {
    date_debut = dateDebut;
}

void Projet::setDateFin(const QDate &dateFin) {
    date_fin = dateFin;
}

void Projet::setStatut(const QString &statut) {
    statut_projet = statut;
}

void Projet::setIdArchitecte(int idArchitecte) {
    id_architecte = idArchitecte;
}
//CRUD

bool Projet::ajouter()
{

    QSqlQuery query;
    query.prepare("INSERT INTO projet ( Nom_proj, Description, ID_Client, Budget, date_Deb, Date_fin, Statut, ID_architect) "
                  "VALUES (:nom, :description, :idClient, :budget, :dateDebut, :dateFin, :statut, :id_architect)");

    query.bindValue(":nom", nom_projet);
    query.bindValue(":description", description);
    query.bindValue(":idClient", id_client);
    query.bindValue(":budget", budget_estime);
    query.bindValue(":dateDebut", date_debut);
    query.bindValue(":dateFin", date_fin);
    query.bindValue(":statut", statut_projet);
    query.bindValue(":id_architect", id_architecte);

    return query.exec();
}

QSqlQueryModel* Projet::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM projet");
    return model;
}

bool Projet::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM projet WHERE ID_Proj = :id");
    query.bindValue(":id", id);
    return query.exec();
}

bool Projet::modifier()
{

    QSqlQuery query;
    query.prepare("UPDATE projet SET Nom_proj = :nom, Description = :description, ID_Client = :idClient, "
                  "Budget = :budget, Date_deb = :dateDebut, Date_fin = :dateFin, "
                  "Statut = :statut, ID_architect = :idarchitect "
                  "WHERE ID_Proj = :id");
    query.bindValue(":id", id_projet);
    query.bindValue(":nom", nom_projet);
    query.bindValue(":description", description);
    query.bindValue(":idClient", id_client);
    query.bindValue(":budget", budget_estime);
    query.bindValue(":dateDebut", date_debut);
    query.bindValue(":dateFin", date_fin);
    query.bindValue(":statut", statut_projet);
    query.bindValue(":idarchitect", id_architecte);

    return query.exec();
}
//tri
QSqlQueryModel * Projet::trier(int test)
{
    QSqlQueryModel *model=new QSqlQueryModel() ;
    QSqlQuery query ;

    if(test==1)
    {
        query.prepare("SELECT *  FROM Projet ORDER BY Nom_proj ASC ") ;
    }
    else if(test==2)
    {
        query.prepare("SELECT *  FROM Projet ORDER BY budget ASC ") ;
    }
    else if(test==3)
    {
        query.prepare("SELECT *  FROM Projet ORDER BY date_deb ASC ") ;
    }
    if (query.exec()&&query.next())
    {
        model->setQuery(query) ;

    }
    return model;
}

//recherhce
QSqlQueryModel * Projet::recherche(QString id)
{
    QSqlQueryModel * model= new QSqlQueryModel();

    QSqlQuery query;
    query.prepare("SELECT * FROM Projet WHERE Upper(Nom_proj) LIKE Upper(:id)");
    query.bindValue(":id", "%" + id + "%");
    query.exec();

    model->setQuery(query);

    return model;
}
//statistique


QChart *Projet::statproj()
{
    QSqlQuery qEnCours, qTermine, qAnnule, qNonCommence;
    qreal cEnCours = 0, cTermine = 0, cAnnule = 0, cNonCommence = 0;

    // Get counts
    qEnCours.prepare("SELECT COUNT(*) FROM projet WHERE statut='en cours'");
    if (qEnCours.exec() && qEnCours.next())
        cEnCours = qEnCours.value(0).toInt();

    qTermine.prepare("SELECT COUNT(*) FROM projet WHERE statut='termine'");
    if (qTermine.exec() && qTermine.next())
        cTermine = qTermine.value(0).toInt();

    qAnnule.prepare("SELECT COUNT(*) FROM projet WHERE statut='annule'");
    if (qAnnule.exec() && qAnnule.next())
        cAnnule = qAnnule.value(0).toInt();

    qNonCommence.prepare("SELECT COUNT(*) FROM projet WHERE statut='non commence'");
    if (qNonCommence.exec() && qNonCommence.next())
        cNonCommence = qNonCommence.value(0).toInt();

    qreal total = cEnCours + cTermine + cAnnule + cNonCommence;
    if (total == 0) total = 1; // prevent division by zero

    // Create donut series
    QPieSeries *series = new QPieSeries();
    QPieSlice *sliceEnCours = series->append("En cours", cEnCours);
    QPieSlice *sliceTermine = series->append("Terminé", cTermine);
    QPieSlice *sliceAnnule = series->append("Annulé", cAnnule);
    QPieSlice *sliceNonCommence = series->append("Non commencé", cNonCommence);

    // Custom colors matching your theme
    sliceEnCours->setBrush(QColor("#FFD4C0"));  // peachy light
    sliceTermine->setBrush(QColor("#F28B82"));  // coral pink
    sliceAnnule->setBrush(QColor("#7D5A50"));   // brownish
    sliceNonCommence->setBrush(QColor("#F5F5DC")); // light blue

    // Set labels with percentages
    for (auto slice : series->slices()) {
        int percent = static_cast<int>((slice->value() / total) * 100);
        slice->setLabel(QString("%1 (%2%)").arg(slice->label()).arg(percent));

        slice->setLabelVisible(true);
        slice->setLabelFont(QFont("Arial",5, QFont::Bold));

    }

    series->setHoleSize(0.35);

    // Create and style chart
    QChart *chart = new QChart();
    chart->addSeries(series);

    chart->setBackgroundVisible(false);
    chart->setContentsMargins(0, 0, 0, 0);
    chart->setMargins(QMargins(0, 0, 0, 0));

    return chart;
}
//Arduino

bool Projet:: existe()
{
    QSqlQuery query;
    query.prepare("SELECT * from Projet where :id_proj=id_proj");
    query.bindValue(":id_proj", id_projet);
    if(query.exec() && query.next())
    {
        id_client= query.value(6).toInt();
        id_architecte = query.value(7).toInt();


        return true;
    }
    else
    {
        return  false;
    }
}
QString Projet:: getClientByID()
{
    QSqlQuery query;
    query.prepare("SELECT * from Clients where :id_client=id_client");
    query.bindValue(":id_client", id_client);
    if(query.exec() && query.next())
    {
        QString nom = query.value(1).toString();

        return nom ;
    }
    else
    {
        return  "";
    }
}
QString Projet:: getArchitectByid()
{
    QSqlQuery query;
    query.prepare("SELECT * from Employe where :IDE=IDE");
    query.bindValue(":IDE", id_architecte);
    if(query.exec() && query.next())
    {
        QString nom = query.value(1).toString();
        qDebug() <<nom;
        return nom ;
    }
    else
    {
        return  "";
    }
}
