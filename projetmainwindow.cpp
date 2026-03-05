#include "projetmainwindow.h"
#include "ui_projetmainwindow.h"
#include "qsqlquery.h"
#include <QMessageBox>
#include <QDebug>
#include <QDialog>
#include<QPrinter>
#include<QPrintDialog>
#include<QPrintPreviewDialog>
#include<QPainter>
#include<QTextDocument>
#include <QPointF>
#include <QPdfWriter>
#include <QFileDialog>
#include <QChartView>
#include <QListWidget>
#include <QMimeData>
#include <QTimer>
#include "DialogResume.h"
#include "employemainwindow.h"

//ProjetMainWindow::ProjetMainWindow(QWidget *parent)
ProjetMainWindow::ProjetMainWindow(const employe& e, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProjetMainWindow)
    ,  chart(nullptr),
    chartView(nullptr)
{
    ui->setupUi(this);
    ui->affichageproj->setModel(p.afficher()); //affichage de liste de proj
    setFormTitle("Ajouter un projet");
    ui->lpr->setVisible(false);//titre calendrier
    //stat


    chart=p.statproj();
    chartView = new QChartView(chart, ui->Stat);

    //    chartView->resize(281, 171);
    chartView->resize(350, 200);
    chartView->setRenderHint(QPainter::Antialiasing);
    //calendrier

    //colore les date libre de l'anne courante  en beige

    QMap<QString, QColor> projectColors;
    QMap<QDate, QList<QColor>> dayColors;

    QSqlQuery query("SELECT date_deb, date_fin, nom_proj FROM projet");
    int hue = 0;
    while (query.next()) {
        QString name = query.value("nom_proj").toString();
        QDate start = query.value("date_deb").toDate();
        QDate end = query.value("date_fin").toDate();

        if (!projectColors.contains(name)) {
            QColor color;
            color.setHsv(hue % 360, 200, 255);  // Définir la couleur en fonction de la teinte (hue)
            projectColors[name] = color;
            hue += 60;  // Augmenter la teinte pour obtenir des couleurs différentes
        }

        // Ajouter la couleur à la liste de couleurs pour chaque jour du projet
        for (QDate d = start; d <= end; d = d.addDays(1)) {
            dayColors[d].append(projectColors[name]);
        }
    }

    // Définir la plage de dates à vérifier pour l'année entière (1er janvier au 31 décembre de l'année en cours)
    QDate startDate = QDate(QDate::currentDate().year(), 1, 1);  // 1er janvier de l'année en cours
    QDate endDate = QDate(QDate::currentDate().year(), 12, 31);   // 31 décembre de l'année en cours

    // Définir la couleur beige personnalisée
    QColor beige(245, 245, 220);  // Couleur beige (R: 245, G: 245, B: 220)

    for (QDate d = startDate; d <= endDate; d = d.addDays(1)) {
        // Si la date n'est pas dans le calendrier des projets (date libre)
        if (!dayColors.contains(d)) {
            // Appliquer la couleur de fond beige aux dates libres
            QTextCharFormat format;
            format.setBackground(beige);  // Appliquer la couleur beige
            ui->cal->setDateTextFormat(d, format);
        }
    }



    //controle de saisie
    QRegularExpression rx("^[A-Za-z ]{1,10}$");

    QValidator *validator = new QRegularExpressionValidator(rx, this);

    QRegularExpression des("^[A-Za-z ]{1,30}$");

    QValidator *valides = new QRegularExpressionValidator(des, this);

    QRegularExpression rs("^(en cours|termine|annule|non commence)$");
    QValidator *valid = new QRegularExpressionValidator(rs, this);

    ui->bua_->setValidator(new QDoubleValidator(0.0, 99999999.0, 2, this));
    ui->npa_->setValidator(validator);
    ui->desa_->setValidator(valides);
    ui->nsa_->setValidator(valid);

    // QSqlQuery query;
    query.prepare("SELECT Id_client FROM Clients ");

    query.exec();
    while (query.next()){
        ui->ica_->addItem(query.value(0).toString());


    }
    query.prepare("SELECT IDE FROM Employe");

    query.exec();
    while (query.next()){
        ui->iaa_->addItem(query.value(0).toString());


    }

    //pour selection non projet pour calendrier


    query.prepare("SELECT nom_proj FROM projet where statut <> 'annule' ");

    query.exec();
    while (query.next()){
        ui->npr->addItem(query.value(0).toString());


    }
    //calendrier
    // Désactiver temporairement le signal de la combobox
    disconnect(ui->npr, SIGNAL(currentTextChanged(QString)), this, SLOT(on_npr_currentTextChanged(QString)));
    ui->cal->setSelectedDate(QDate::currentDate());
    // Initialiser le label sans affichage de projet
    ui->drr->setText("Aucun projet sélectionné");
    connect(ui->npr, SIGNAL(currentTextChanged(QString)), this, SLOT(on_npr_currentTextChanged(QString)));
    ui->ti_m->setText("Calendrier");
    ui->lpr->setVisible(true);
    ui->calp->setText("Aucun date selectionne");

    //to do list

    loadProjectsToComboBox();
    //
    setupListConnections();
    loadTasksFromDatabase();
    setupDragDrop();
    //Arduino//

    arduino4 = new Arduino(this);
    arduino4->setPortName("COM8");

        int ret= arduino4->connect_arduino(); // lancer la connexion à arduino
    switch(ret){
    case(0):qDebug()<< "arduino is available and connected to : "<< arduino4->getarduino_port_name();
        break;
    case(1):qDebug() << "arduino is available but not connected to :" <<arduino4->getarduino_port_name();
        break;
    case(-1):qDebug() << "arduino is not available";
    }
    QObject::connect(arduino4->getserial(),SIGNAL(readyRead()),this,SLOT(update_label()));
}

void ProjetMainWindow::update_label()
{
    data = arduino4->read_from_arduino();
    if(data == "N")
    {
        id ="" ;
    }
    else if(data!="D" && data != "N")
    {
        id = id + data ;
    }
    else if (data != "N")
    {
        Projet p ;
        p.setId(id.toInt()) ;
        if(p.existe())
        {
            QString nom = p.getArchitectByid();
            qDebug() << nom ;

            for (int i = 0 ; i< nom.length() ; i++)
            {
                QByteArray ch(1, nom[i].toLatin1());
                arduino4->write_to_arduino(ch) ;
            }
            arduino4->write_to_arduino("B") ;
            nom = p.getClientByID();
            qDebug() << nom ;

            for (int i = 0 ; i< nom.length() ; i++)
            {
                QByteArray ch(1, nom[i].toLatin1());
                arduino4->write_to_arduino(ch) ;
            }
            arduino4->write_to_arduino("D") ;
        }
        else
        {
            arduino4->write_to_arduino("N");
        }
        id ="" ;
    }
    qDebug () << id ;
}





ProjetMainWindow::~ProjetMainWindow()
{
    delete ui;
}
void ProjetMainWindow::updateChart() {
    if (chartView) {
        // Vérifier si p.statproj() renvoie un graphique valide
        QChart* newChart = p.statproj();  // Générer un nouveau graphique basé sur les données actuelles

        if (newChart) {
            if (chart) {
                // Comparer les séries pour vérifier si le graphique doit être mis à jour
                bool shouldUpdate = false;
                if (chart->series().size() != newChart->series().size()) {
                    shouldUpdate = true;  // Si le nombre de séries change, on doit mettre à jour
                } else {
                    // Comparer les séries une par une pour voir si des données ont changé
                    for (int i = 0; i < chart->series().size(); ++i) {
                        if (chart->series().at(i) != newChart->series().at(i)) {
                            shouldUpdate = true;
                            break;
                        }
                    }
                }

                if (shouldUpdate) {
                    delete chart;  // Supprimer l'ancien graphique
                    chart = nullptr;  // Réinitialiser le graphique

                    chart = newChart;  // Affecter le nouveau graphique
                    chartView->setChart(chart);  // Mettre à jour le graphique affiché
                }
            } else {
                // Si aucun graphique n'existe encore, créer le premier graphique
                chart = newChart;
                chartView->setChart(chart);
            }

            chartView->update();  // Rafraîchir l'affichage
        } else {
            qDebug() << "Erreur: Le graphique généré par p.statproj() est NULL.";
        }
    } else {
        qDebug() << "Erreur: chartView est NULL.";
    }

    qDebug() << "Graphique mis à jour.";
}



//
void ProjetMainWindow::resetFormFields()
{
    ui->npa_->clear();
    ui->desa_->clear();
    ui->dda_->setDate(QDate::currentDate()); // ou une date par défaut
    ui->dfa_->setDate(QDate::currentDate());
    ui->bua_->clear();
    ui->nsa_->clear();
    ui->ica_->setCurrentIndex(0); // Réinitialise à l'index par défaut
    ui->iaa_->setCurrentIndex(0); // Réinitialise à l'index par défaut
}
//titre de formulaire ajout et modif
void ProjetMainWindow::setFormTitle(const QString &title)
{
    ui->title->setText(title);
}


//double clique sur idproj  dans le table  pour supp et modif
void ProjetMainWindow::on_affichageproj_activated(const QModelIndex &index)
{QString val=ui->affichageproj->model()->data(index).toString();
    QSqlQuery qry;

    qry.prepare("SELECT * FROM Projet WHERE id_proj = :id");
    qry.bindValue(":id", val);
    setFormTitle("Modifier un projet");



    if(qry.exec())
    {
        while(qry.next())
        {
            ui->idsup->setText(qry.value(0).toString());
            ui->npa_->setText(qry.value(1).toString());

            ui->desa_->setText(qry.value(2).toString());
            ui->dda_->setDate(qry.value(3).toDate());
            ui->dfa_->setDate(qry.value(4).toDate());
            ui->bua_->setText(qry.value(5).toString());

            ui->ica_->setCurrentText(qry.value(6).toString());
            ui->iaa_->setCurrentText(qry.value(7).toString());
            ui->nsa_->setText(qry.value(8).toString());
        }

    }

}


void ProjetMainWindow::on_pushButton_60_clicked() //ajout proj  pour formulaire
{

    {
        QString nomproj=ui->npa_->text();
        QString description =ui->desa_->text();
        QDate datedeb=ui->dda_->date();
        QDate datefin=ui->dfa_->date();
        double budget=ui->bua_->text().toDouble();
        QString statut=ui->nsa_->text();
        int idarchitect=ui->iaa_->currentText().toInt();

        int idclient=ui->ica_->currentText().toInt();

        //
        if (datefin < datedeb) {
            QMessageBox::warning(this, "Erreur de saisie", "La date de fin doit être postérieure à la date de début.");
            return; // Ne pas soumettre le formulaire
        }

        Projet pa(1,nomproj,description,idclient,budget,datedeb,datefin,statut,idarchitect);




        bool test=pa.ajouter();
        if(test)
        {
            ui->affichageproj->setModel(pa.afficher());
            //Mis a jour stat
            updateChart();
            //MIS a jour todolist
            refreshProjectsList();
            //Mis a jours calendrier

            on_npr_currentTextChanged(nomproj);

            QSqlQuery  query;
            query.prepare("SELECT nom_proj FROM projet where statut <> 'annule'");
            ui->npr->clear();
            query.exec();
            while (query.next()){
                ui->npr->addItem(query.value(0).toString());
            }
            //
            on_pushButton_55_clicked();
            //


            //QMessageBox::information(nullptr, QObject::tr("ajout effectué"),  QObject::tr("connection successful.\n" "Click Cancel to exit."), QMessageBox::Cancel);
            QMessageBox::information(this, "Succès", "Ajout effectuée avec succès.");

            ui->lstprj->setCurrentIndex(0);
            resetFormFields();
            setFormTitle("Ajouter un projet");

        }

        else
            QMessageBox::critical(nullptr, QObject::tr("ajout n'est pas effectué"),QObject::tr("connection failed.\n"  "Click Cancel to exit."), QMessageBox::Cancel);
    }
}


void ProjetMainWindow::on_pushButton_57_clicked()//suppression
{
    int id=ui->idsup->text().toInt();
    bool test=p.supprimer(id);
    if(test)
    {
        ui->affichageproj->setModel(p.afficher());
        resetFormFields();
        setFormTitle("Ajouter un projet");


        //QMessageBox::information(nullptr, QObject::tr("suppression effectué"),  QObject::tr("connection successful.\n" "Click Cancel to exit."), QMessageBox::Cancel);
        QMessageBox::information(this, "Succès", "Supression effectuée avec succès.");
        //
        updateChart();
        //Misajour to do list
        refreshProjectsList();
        //mis ajour calendrier
        on_npr_currentTextChanged(ui->npa_->text());
        QSqlQuery  query;
        query.prepare("SELECT nom_proj FROM projet where statut <> 'annule' ");
        ui->npr->clear();

        query.exec();
        while (query.next()){
            ui->npr->addItem(query.value(0).toString());
        }
        on_pushButton_55_clicked();

    }

    else
        QMessageBox::critical(nullptr, QObject::tr("suppression n'est pas effuctué"),QObject::tr("connection failed.\n"  "Click Cancel to exit."), QMessageBox::Cancel);
}

//modificationproj
void ProjetMainWindow::on_pushButton_56_clicked()
{
    {
        QString nomproj=ui->npa_->text();
        QString description =ui->desa_->text();
        QDate datedeb=ui->dda_->date();
        QDate datefin=ui->dfa_->date();
        double budget=ui->bua_->text().toDouble();
        QString statut=ui->nsa_->text();
        int idarchitect=ui->iaa_->currentText().toInt();
        int idclient=ui->ica_->currentText().toInt();
        //
        if (datefin < datedeb) {
            QMessageBox::warning(this, "Erreur de saisie", "La date de fin doit être postérieure à la date de début.");
            return; // Ne pas soumettre le formulaire
        }
        //


        int id=ui->idsup->text().toInt(); //recupere id a modifier a partir idsup (id cliqué)
        Projet pa(id,nomproj,description,idclient,budget,datedeb,datefin,statut,idarchitect);

        bool test=pa.modifier();
        if(test)
        {
            //to do list
            if (statut.toLower() == "annulé" || statut.toLower() == "annule") {
                deleteTasksForProject(id);
            }
            //
            ui->affichageproj->setModel(pa.afficher());

            //QMessageBox::information(nullptr, QObject::tr("modification effectué"),  QObject::tr("connection successful.\n" "Click Cancel to exit."), QMessageBox::Cancel);
            QMessageBox::information(this, "Succès", "Modification effectuée avec succès.");
            //
            updateChart();
            refreshProjectsList();
            //Mis ajour cal
            on_npr_currentTextChanged(nomproj);
            QSqlQuery  query;
            query.prepare("SELECT nom_proj FROM projet where statut <> 'annule' ");
            ui->npr->clear();
            query.exec();
            while (query.next()){
                ui->npr->addItem(query.value(0).toString());
            }
            //refresh bpouton calendrier
            on_pushButton_55_clicked();
            resetFormFields();
            setFormTitle("Ajouter un projet");


        }

        else
            QMessageBox::critical(nullptr, QObject::tr("modification n'est pas effuctué"),QObject::tr("connection failed.\n"  "Click Cancel to exit."), QMessageBox::Cancel);
    }
}



//tri
void ProjetMainWindow::on_comboBox_4_activated(int index)
{
    if( index == 0)
        ui->affichageproj->setModel(p.trier(1));
    if( index == 1)
        ui->affichageproj->setModel(p.trier(2));
    if( index == 2)
        ui->affichageproj->setModel(p.trier(3));
}

//recherche
void ProjetMainWindow::on_recherche_4_textChanged(const QString &arg1)
{
    QString r=ui->recherche_4->text();
    // qDebug() <<r;
    ui->affichageproj->setModel(p.recherche(r));
}

//export pdf
void ProjetMainWindow::on_pushButton_51_clicked()
{
    // Récupérer le modèle du QTableView (qui contient les résultats filtrés)
    QAbstractItemModel* model = ui->affichageproj->model();
    if (!model) return;

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", QString(), "*.pdf");
    if (QFileInfo(fileName).suffix().isEmpty()) {
        fileName.append(".pdf");
    }

    QPdfWriter pdf(fileName);
    QPainter painter(&pdf);

    painter.setPen(Qt::red);
    painter.setFont(QFont("Cambria", 30));
    painter.drawText(2300, 1700, "Liste des projets trouvés");

    painter.setPen(Qt::black);
    painter.setFont(QFont("Cambria", 14));
    painter.drawRect(0, 3000, 9600, 500);

    painter.setFont(QFont("Cambria", 11));
    painter.drawText(200, 3300, "id_projet");
    painter.drawText(1300, 3300, "nom_projet");
    painter.drawText(2700, 3300, "description");
    painter.drawText(4000, 3300, "Budget");
    painter.drawText(5300, 3300, "id_client");
    painter.drawText(6600, 3300, "id_architecte");
    painter.drawText(7900, 3300, "statut");

    int i = 4000;

    int rowCount = model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        painter.drawText(200, i, model->data(model->index(row, 0)).toString());
        painter.drawText(1300, i, model->data(model->index(row, 1)).toString());
        painter.drawText(2700, i, model->data(model->index(row, 2)).toString());
        painter.drawText(4000, i, model->data(model->index(row, 5)).toString());
        painter.drawText(5300, i, model->data(model->index(row, 6)).toString());
        painter.drawText(6600, i, model->data(model->index(row, 7)).toString());
        painter.drawText(7900, i, model->data(model->index(row, 8)).toString());
        i += 500;
    }

    painter.end(); // Terminer l'écriture dans le PDF
}



void ProjetMainWindow::on_cal_clicked(const QDate &date)
{
    QSqlQuery query;
    QString result;
    query.prepare("Select date_Deb , date_Fin ,nom_proj from Projet  ");
    query.exec();
    while (query.next()) {

        QDate start = query.value("Date_deb").toDate();
        QDate end = query.value("Date_fin").toDate();
        QString name = query.value("nom_proj").toString();
        //  qDebug()<<start<<end<<date;
        if (date >= start && date <= end) {
            result += name + "\n";
        }
    }

    if (result.isEmpty()){
        result = "Aucun projet trouvé";
        ui->lpr->setVisible(true);}
    else{
        ui->lpr->setVisible(true);
    }

    ui->calp->setText(result);


}

// / calendrier proj selectionne
void ProjetMainWindow::on_npr_currentTextChanged(const QString &arg1)
{
    if (firstLoad) {
        // Première exécution : on ajoute un élément vide si ce n'est pas déjà fait
        if (ui->npr->count() == 0 || ui->npr->itemText(0) != "") {
            ui->npr->insertItem(0, "");  // Ajouter un item vide au début, s'il n'existe pas déjà
        }
        ui->npr->setCurrentIndex(0);  // Sélectionner l'élément vide
        ui->cal->setSelectedDate(QDate::currentDate());  // Fixer la date d'aujourd'hui
        ui->drr->setText("Aucun projet sélectionné");
        firstLoad = false;  // Marquer que le premier chargement est terminé
        return;
    }

    if (arg1.isEmpty()) {
        ui->drr->setText("Aucun projet sélectionné");
        ui->calp->setText("Aucun date selectionne");
        return; }// Ne rien faire si aucun projet n'est sélectionné

    QSqlQuery query;
    QDate today = QDate::currentDate();
    ui->cal->setDateTextFormat(QDate(), QTextCharFormat()); // Clear formatting

    QSet<QDate> usedDates;

    // === Étape 1 : Récupérer toutes les dates de projet ===
    query.prepare("SELECT date_deb, date_fin FROM projet");
    query.exec();

    while (query.next()) {
        QDate start = query.value("date_deb").toDate();
        QDate end = query.value("date_fin").toDate();

        for (QDate d = start; d <= end; d = d.addDays(1)) {
            usedDates.insert(d);
        }
    }

    // === Étape 2 : Colorier les jours libres en beige ===
    QColor beige(245, 245, 220);
    QDate startDate(QDate::currentDate().year(), 1, 1);
    QDate endDate(QDate::currentDate().year(), 12, 31);

    for (QDate d = startDate; d <= endDate; d = d.addDays(1)) {
        if (!usedDates.contains(d)) {
            QTextCharFormat format;
            format.setBackground(beige);
            ui->cal->setDateTextFormat(d, format);
        }
    }

    // === Étape 3 : Récupérer les dates du projet sélectionné ===
    query.prepare("SELECT date_deb, date_fin FROM projet WHERE nom_proj = :projName");
    query.bindValue(":projName", arg1);
    query.exec();

    if (query.next()) {
        QDate start = query.value("date_deb").toDate();
        QDate end = query.value("date_fin").toDate();

        // Déplacer le calendrier vers la date de début du projet seulement si un projet est sélectionné
        ui->cal->setSelectedDate(start);

        for (QDate d = start; d <= end; d = d.addDays(1)) {
            QTextCharFormat format;
            if (d < today) {
                format.setBackground(QColor("#D9534F")); // Rouge (passé)
            } else {
                format.setBackground(QColor("#5CB85C")); // Vert (futur ou présent)
            }
            format.setForeground(Qt::white);
            ui->cal->setDateTextFormat(d, format);
        }

        // Mettre à jour le label dd avec le statut du projet
        if (end < today) {
            ui->drr->setText("Projet terminé");
        } else if (start > today) {
            int daysBeforeStart = today.daysTo(start);
            ui->drr->setText("Début dans " + QString::number(daysBeforeStart) + " jours");
        } else {
            int daysRemaining = today.daysTo(end);
            ui->drr->setText("Termine dans " + QString::number(daysRemaining) + " jours");
        }
    }
}
//refresh calendrier boutton

void ProjetMainWindow::on_pushButton_55_clicked()
{
    QDate today = QDate::currentDate();
    ui->cal->setSelectedDate(today);

    QSqlQuery query;
    // QDate today = QDate::currentDate();

    // Réinitialiser les couleurs des projets sélectionnés sans toucher à la couleur beige
    ui->cal->setDateTextFormat(QDate(), QTextCharFormat());  // Effacer toute mise en forme

    // Garder la couleur beige pour les dates libres
    QSet<QDate> usedDates;
    query.prepare("SELECT date_deb, date_fin FROM projet");
    query.exec();

    while (query.next()) {
        QDate start = query.value("date_deb").toDate();
        QDate end = query.value("date_fin").toDate();

        for (QDate d = start; d <= end; d = d.addDays(1)) {
            usedDates.insert(d);
        }
    }

    // Appliquer la couleur beige pour les dates libres
    QColor beige(245, 245, 220);
    QDate startDate(QDate::currentDate().year(), 1, 1);
    QDate endDate(QDate::currentDate().year(), 12, 31);

    for (QDate d = startDate; d <= endDate; d = d.addDays(1)) {
        if (!usedDates.contains(d)) {
            QTextCharFormat format;
            format.setBackground(beige);
            ui->cal->setDateTextFormat(d, format);
        }
    }

    // Réinitialiser le statut du projet dans le label
    ui->drr->setText("Aucun Projet selectionne");  // Ou mettre un message par défaut comme "Aucun projet sélectionné"

    // Optionnel : Réinitialiser la combobox à un index vide
    ui->npr->setCurrentIndex(-1);
    //
    ui->calp->setText("Aucun date selectionne");
}
void ProjetMainWindow::on_pushButton_59_clicked()
{
    ui->page_ini->setCurrentIndex(1);
    ui->ti_m->setText("Calendrier");

    on_pushButton_55_clicked();
}
void ProjetMainWindow::on_pushButton_54_clicked()
{
    ui->page_ini->setCurrentIndex(2);
    ui->ti_m->setText("To do list");
}
void ProjetMainWindow::updateTaskStatusInDatabase(int taskId, const QString &newStatus) {
    QSqlQuery query;
    query.prepare("UPDATE todo SET statut = :status WHERE id_tache = :id");
    query.bindValue(":status", newStatus);
    query.bindValue(":id", taskId);

    if (!query.exec()) {
        qDebug() << "Erreur mise à jour statut:" ;
    }
}
void ProjetMainWindow::setupDragDrop() {
    auto configureList = [this](QListWidget *list) {
        list->setDragEnabled(true);
        list->setAcceptDrops(true);
        list->setDropIndicatorShown(true);
        list->setDragDropMode(QAbstractItemView::DragDrop);
        list->setDefaultDropAction(Qt::MoveAction);
        list->setSelectionMode(QAbstractItemView::SingleSelection);
    };

    configureList(ui->todo);
    configureList(ui->prgrs);
    configureList(ui->done);

    // Nouveau : Gestion de la sélection unique globale
    auto handleSelectionChange = [this](QListWidget* activeList) {
        // Désélectionner toutes les autres listes
        QList<QListWidget*> allLists = {ui->todo, ui->prgrs, ui->done};
        for (QListWidget* list : allLists) {
            if (list != activeList && !list->selectedItems().isEmpty()) {
                list->clearSelection();
                list->setCurrentItem(nullptr);
            }
        }
    };

    // Connexion des signaux
    connect(ui->todo, &QListWidget::itemChanged, this, [this]() { handleItemMoved(ui->todo); });
    connect(ui->prgrs, &QListWidget::itemChanged, this, [this]() { handleItemMoved(ui->prgrs); });
    connect(ui->done, &QListWidget::itemChanged, this, [this]() { handleItemMoved(ui->done); });
    // Nouveaux signaux pour la sélection unique
    connect(ui->todo, &QListWidget::itemSelectionChanged, this, [=]() {
        if (!ui->todo->selectedItems().isEmpty()) handleSelectionChange(ui->todo);
    });
    connect(ui->prgrs, &QListWidget::itemSelectionChanged, this, [=]() {
        if (!ui->prgrs->selectedItems().isEmpty()) handleSelectionChange(ui->prgrs);
    });
    connect(ui->done, &QListWidget::itemSelectionChanged, this, [=]() {
        if (!ui->done->selectedItems().isEmpty()) handleSelectionChange(ui->done);
    });
}

// Gestion du déplacement
void ProjetMainWindow::handleItemMoved(QListWidget *list) {
    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        int taskId = item->data(Qt::UserRole).toInt();
        QString newStatus = getStatusForList(list);
        updateTaskStatusInDatabase(taskId, newStatus);
    }
}

// Obtention du statut selon la liste
QString ProjetMainWindow::getStatusForList(QListWidget *list) const {
    if (list == ui->todo) return "to do";
    if (list == ui->prgrs) return "In progress";
    if (list == ui->done) return "done";
    return "";
}
//bouton ajout tache
void ProjetMainWindow::on_pushButton_52_clicked(){
    addNewTask();
}




//modif tache
void ProjetMainWindow::setupListConnections() {
    auto setupList = [this](QListWidget* list) {
        list->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        connect(list, &QListWidget::itemChanged, this, [this](QListWidgetItem *item) {
            // Vérifier si le changement vient de l'édition du texte
            if (item->isSelected()) {
                handleItemEdit(item);
            }
        });
    };

    setupList(ui->todo);
    setupList(ui->prgrs);
    setupList(ui->done);
}
void ProjetMainWindow::handleItemEdit(QListWidgetItem *item) {
    if (!item) return;

    // 1. Récupérer les données
    int taskId = item->data(Qt::UserRole).toInt();
    QString newText = item->text().trimmed();

    if (taskId <= 0) {
        qDebug() << "ID de tâche invalide";
        return;
    }

    if (newText.isEmpty()) {
        // Si le texte est vide, restaurer l'ancienne valeur
        QSqlQuery query;
        query.prepare("SELECT name FROM todo WHERE id_tache = ?");
        query.addBindValue(taskId);

        if (query.exec() && query.next()) {
            item->setText(query.value(0).toString());
        }
        QMessageBox::warning(this, "Attention", "Le nom de la tâche ne peut pas être vide");
        return;
    }

    // 2. Vérifier si le texte a réellement changé
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT name FROM todo WHERE id_tache = ?");
    checkQuery.addBindValue(taskId);

    if (checkQuery.exec() && checkQuery.next()) {
        QString currentDbText = checkQuery.value(0).toString();
        if (currentDbText == newText) {
            return; // Pas de changement, rien à faire
        }
    }

    // 3. Mise à jour dans la base
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE todo SET name = ? WHERE id_tache = ?");
    updateQuery.addBindValue(newText);
    updateQuery.addBindValue(taskId);

    if (!updateQuery.exec()) {
        qDebug() << "Échec de la mise à jour du nom de la tâche:" ;

        // Restaurer l'ancien nom depuis la base de données
        QSqlQuery restoreQuery;
        restoreQuery.prepare("SELECT name FROM todo WHERE id_tache = ?");
        restoreQuery.addBindValue(taskId);

        if (restoreQuery.exec() && restoreQuery.next()) {
            item->setText(restoreQuery.value(0).toString());
        }

        QMessageBox::critical(this, "Erreur", "Échec de la mise à jour du nom de la tâche");
    }
}

//supressioon tache

void ProjetMainWindow::deleteSelectedTask() {
    // 1. Trouver la liste active avec sélection
    QListWidget* currentList = nullptr;
    QListWidgetItem* selectedItem = nullptr;

    QList<QListWidget*> allLists = {ui->todo, ui->prgrs, ui->done};

    for (QListWidget* list : allLists) {
        if (list->currentItem()) {
            currentList = list;
            selectedItem = list->currentItem();
            break;
        }
    }

    // 2. Vérification de sélection
    if (!selectedItem) {
        QMessageBox::information(this, "Information", "Aucune tâche sélectionnée");
        return;
    }

    // 3. Récupération de l'ID
    int taskId = selectedItem->data(Qt::UserRole).toInt();
    if (taskId <= 0) {
        qDebug() << "ID invalide - suppression annulée";
        return;
    }

    // 4. Confirmation utilisateur
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmer",
                                  "Supprimer cette tâche ?\n" + selectedItem->text(),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    // 5. Suppression BD avec gestion d'erreur
    QSqlQuery query;
    query.prepare("DELETE FROM todo WHERE id_tache = ?");
    query.addBindValue(taskId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur",
                              "Échec de suppression:\n" );
        return;
    }

    // 6. Suppression UI avec vérification
    int row = currentList->row(selectedItem);
    if (row >= 0) {
        // Désélectionner avant suppression
        currentList->clearSelection();
        currentList->setCurrentItem(nullptr);

        delete currentList->takeItem(row);
        qDebug() << "Tâche supprimée - ID:" << taskId;
    }

    // 7. Nettoyage et rafraîchissement
    currentList->viewport()->update();
    QApplication::processEvents();
}
void ProjetMainWindow::clearAllSelectionsExcept(QListWidget* activeList) {
    QList<QListWidget*> allLists = {ui->todo, ui->prgrs, ui->done};

    for (QListWidget* list : allLists) {
        if (list != activeList) {
            list->clearSelection();
            list->setCurrentItem(nullptr);
        }
    }
}

void ProjetMainWindow::on_supp_task_clicked()
{
    deleteSelectedTask();
}




//to do list par projj


// Remplir le ComboBox des projets
void ProjetMainWindow::loadProjectsToComboBox()
{
    ui->Projet_todo->clear();

    // Option "Tous les projets"
    ui->Projet_todo->addItem("Tous les projets", -1);
    // Préparer et exécuter la requête
    QSqlQuery query;
    //where statut <> 'annule' a eliminer
    query.prepare("SELECT id_proj, nom_proj FROM projet where statut <> 'annule' ORDER BY nom_proj ");

    if (!query.exec()) {
        qDebug() << "Erreur lors de la récupération des projets:" ;
        return;
    }

    // Remplir le ComboBox avec les résultats
    while (query.next()) {
        int id = query.value(0).toInt();       // id_proj
        QString nom = query.value(1).toString(); // nom du projet

        // Ajouter au ComboBox : texte visible = nom, donnée associée = id_proj
        ui->Projet_todo->addItem(nom, id);
    }
}



void ProjetMainWindow::on_Projet_todo_currentTextChanged(const QString &arg1)
{
    int idProjet = ui->Projet_todo->currentData().toInt();

    // Recharger les tâches selon le filtre
    loadTasksFromDatabase(idProjet);
}

void ProjetMainWindow::loadTasksFromDatabase(int idProjet) // -1 = Tous les projets
{
    // Vider les listes actuelles
    ui->todo->clear();
    ui->prgrs->clear();
    ui->done->clear();

    // Construire la requête SQL dynamiquement
    QSqlQuery query;
    QString sql = "SELECT id_tache, name, statut FROM todo";

    if (idProjet != -1) {
        sql += " WHERE id_proj = :idProjet";
        query.prepare(sql);
        query.bindValue(":idProjet", idProjet);
    } else {
        query.prepare(sql);
    }

    // Exécuter et remplir les listes
    if (query.exec()) {
        while (query.next()) {
            int id = query.value(0).toInt();
            QString name = query.value(1).toString();
            QString status = query.value(2).toString();

            QListWidgetItem *item = new QListWidgetItem(name);
            item->setData(Qt::UserRole, id);
            item->setFlags(item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);

            if (status == "to do") ui->todo->addItem(item);
            else if (status == "In progress") ui->prgrs->addItem(item);
            else if (status == "done") ui->done->addItem(item);
        }
    } else {
        qDebug() << "Erreur chargement tâches:" ;
    }
}
void ProjetMainWindow::addNewTask()
{
    QString taskName = ui->addts->text().trimmed();
    if (taskName.isEmpty()) return;

    // Récupérer l'ID du projet sélectionné
    int idProjet = ui->Projet_todo->currentData().toInt();

    // Valider qu'un projet est sélectionné (pas "Tous les projets")
    if (idProjet == -1) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un projet spécifique");
        return;
    }

    // Insérer dans la base de données
    int taskId = insertTaskToDatabase(taskName, "to do", idProjet);
    if (taskId <= 0) return;

    // Ajouter à l'interface uniquement si le filtre correspond
    if (ui->Projet_todo->currentData().toInt() == idProjet) {
        QListWidgetItem *item = new QListWidgetItem(taskName);
        item->setData(Qt::UserRole, taskId);
        item->setFlags(item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        ui->todo->addItem(item);
    }

    ui->addts->clear(); // Vider le champ de saisie
}
int ProjetMainWindow::insertTaskToDatabase(const QString &name, const QString &status, int idProjet)
{
    QSqlQuery query;
    query.prepare("INSERT INTO todo (name, statut, id_proj) VALUES (?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(status);
    query.addBindValue(idProjet);

    if (!query.exec()) {
        qDebug() << "Erreur insertion tâche:" ;
        return -1;
    }

    //return query.lastInsertId().toInt(); // Retourne l'ID de la nouvelle tâche
    query.prepare("SELECT TODO_SEQ.CURRVAL FROM DUAL");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
}
void ProjetMainWindow::refreshProjectsList()
{
    // Sauvegarder la sélection actuelle
    int currentId = ui->Projet_todo->currentData().toInt();
    // Recharger les projets
    loadProjectsToComboBox();

    // Restaurer la sélection si elle existe encore
    int index = ui->Projet_todo->findData(currentId);
    if (index >= 0) {
        ui->Projet_todo->setCurrentIndex(index);
    } else {
        ui->Projet_todo->setCurrentIndex(0); // "Tous les projets"
    }


}

//suppression de tache des proj anulle
void ProjetMainWindow::deleteTasksForProject(int projectId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM todo WHERE id_proj = :id");
    query.bindValue(":id", projectId);

    if (!query.exec()) {
        qDebug() << "Erreur lors de la suppression des tâches : " ;
    } else {
        qDebug() << "Tâches du projet " << projectId << " supprimées.";
    }


}
QString ProjetMainWindow::genererResume(int id_projet)
{ QSqlQuery query;
    query.prepare("SELECT nom_proj, date_deb, date_fin FROM projet WHERE id_proj = :idProjet");
    query.bindValue(":idProjet", id_projet);

    QString nom_projet;
    QDate date_debut;
    QDate date_fin;

    if (query.exec() && query.next()) {
        // Récupérer les informations du projet
        nom_projet = query.value(0).toString();
        date_debut = query.value(1).toDate();
        date_fin = query.value(2).toDate();
    } else {
        qDebug() << "Erreur SQL : " ;
        return "Erreur lors de la récupération des informations du projet.";
    }


    //QSqlQuery query;
    query.prepare("SELECT statut FROM TODO WHERE ID_PROJ = :idProjet");
    query.bindValue(":idProjet", id_projet);

    int totalTaches = 0;
    int tachesToDo = 0;
    int tachesInProgress = 0;
    int tachesDone = 0;

    if (query.exec()) {
        while (query.next()) {
            totalTaches++;
            QString statut = query.value(0).toString();
            if (statut == "to do") {
                tachesToDo++;
            } else if (statut == "In progress") {
                tachesInProgress++;
            } else if (statut == "done") {
                tachesDone++;
            }
        }

        // Calculer le pourcentage d'avancement
        double pourcentageFini = (totalTaches > 0) ? (double)tachesDone / totalTaches * 100 : 0;
        double pourcentageInProgress = (totalTaches > 0) ? (double)tachesInProgress / totalTaches * 100 : 0;

        // Créer un résumé détaillé
        QString resume;
        resume.append("************** Rapport du projet : " + nom_projet + " **************\n\n");

        resume.append("Résumé général :\n");
        resume.append(QString("Le projet \"%1\" a démarré le %2 et est prévu pour se terminer le %3.\n\n")
                          .arg(nom_projet)
                          .arg(date_debut.toString("dd/MM/yyyy"))
                          .arg(date_fin.toString("dd/MM/yyyy")));

        resume.append("État des tâches :\n");
        resume.append(QString("Nombre total de tâches à réaliser : %1\n").arg(totalTaches));
        resume.append(QString("Tâches terminées : %1 (%2%)\n").arg(tachesDone).arg(pourcentageFini, 0, 'f', 2));
        resume.append(QString("Tâches en cours : %1 (%2%)\n").arg(tachesInProgress).arg(pourcentageInProgress, 0, 'f', 2));
        resume.append(QString("Tâches à faire : %1\n\n").arg(tachesToDo));

        resume.append("Taux d'avancement global :\n");
        resume.append(QString("Le projet a actuellement un avancement de %1%%.\n\n").arg(pourcentageFini, 0, 'f', 2));



        // Alertes et recommandations basées sur l'avancement
        if (pourcentageFini < 50) {
            resume.append("ALERTE : Le projet est en retard. Moins de 50% des tâches sont terminées.\n");
            resume.append("Des mesures correctives doivent être prises pour accélérer la progression.\n");
        } else if (pourcentageFini >= 50 && pourcentageFini < 80) {
            resume.append("Le projet avance à un bon rythme, mais il reste encore des efforts à fournir.\n");
        } else if (pourcentageFini == 100) {
            resume.append("Félicitations ! Le projet est terminé avec toutes les tâches complétées.\n");
        }

        resume.append("\n******************************************************\n");

        return resume;
    } else {
        qDebug() << "Erreur SQL : " ;
        return "Erreur lors de la génération du résumé.";
    }
}
void ProjetMainWindow::on_pushButton_58_clicked()
{
    int idProjet = ui->Projet_todo->currentData().toInt();

    QString resume = genererResume(idProjet); // appelle ta fonction de résumé

    DialogResume *dialog = new DialogResume(this);
    dialog->setResumeText(resume);
    dialog->exec(); // Affiche le dialog de façon modale
}

