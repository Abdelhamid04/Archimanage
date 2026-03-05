#include "clientmainwindow.h"
#include "ui_clientmainwindow.h"
#include "client.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QtCharts>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QChartView>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QTextDocument>
#include <QSqlError>
#include <QSqlQuery>

#include "login.h"
#include "employe.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QMessageBox>
#include <QPainter>

ClientMainWindow::ClientMainWindow(const employe& e, QWidget *parent) :
    QWidget(parent),  // Utilisation de QWidget ici aussi
    ui(new Ui::ClientMainWindow),
    m_networkManager(new QNetworkAccessManager(this))  // Assure-toi que tu as bien un fichier .ui adapté pour QWidget
    ,employeConnecte(e)

{
    ui->setupUi(this);

    qDebug() << "Nombre d'enfants visibles dans ClientMainWindow : " << this->findChildren<QWidget*>().size();

    ui->tableWidget->setRowCount(10);  // S'assurer qu'il est référencé


    afficherTable();
    afficherStats(); // Premier affichage
    setupStatsView();
    setupPieChart(); // Ajout de cette ligne
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &ClientMainWindow::onTableSelectionChanged);
    //  QMessageBox::information(this, "Succès", "Client ajouté et stats mises à jour")
    //    ;
    connect(ui->searchFrame, &QLineEdit::textChanged, this, &ClientMainWindow::on_lineEdit_textChanged);
    connect(ui->triNomButton, &QPushButton::clicked, this, &ClientMainWindow::on_triNomButton_clicked);
    connect(ui->triDateButton, &QPushButton::clicked, this, &ClientMainWindow::on_triDateButton_clicked);
    connect(ui->exportPdfButton, &QPushButton::clicked, this, &ClientMainWindow::exporterEnPDF);
    connect(ui->bsms, &QPushButton::clicked, this, &Client::notifierClientsFideles);
    connect(ui->pushButton_generate, &QPushButton::clicked, this, &ClientMainWindow::onGenerateClicked);
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, [this]() {
        int row = ui->tableWidget->currentRow();
        if (row >= 0) {
            int idClient = ui->tableWidget->item(row, 0)->text().toInt();  // Colonne 0 = ID

        }
    });

    ui->tableView->setModel(Client::afficherPointsFidelite());
    Client::calculerPointsTousLesClients();




}



void ClientMainWindow::onTableSelectionChanged()
{
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        // Remplir les champs avec les données de la ligne sélectionnée
        ui->prenomLineEdit->setText(ui->tableWidget->item(row, 2)->text()); // Prénom
        ui->nomLineEdit->setText(ui->tableWidget->item(row, 1)->text());    // Nom
        ui->emailLineEdit->setText(ui->tableWidget->item(row, 3)->text());  // Email
        ui->telephoneLineEdit->setText(ui->tableWidget->item(row, 4)->text()); // Téléphone
        ui->adresseLineEdit->setText(ui->tableWidget->item(row, 5)->text()); // Adresse
        // Définir le type de client dans la comboBox
        QString type = ui->tableWidget->item(row, 6)->text();

        int index = ui->typeClientComboBox->findText(type);
        if (index >= 0) {
            ui->typeClientComboBox->setCurrentIndex(index);
        }
    }
}

ClientMainWindow::~ClientMainWindow()
{
    delete ui;


}

void ClientMainWindow::afficherTable() {
    // 1. Effacer le contenu existant
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    // 2. Récupérer les données
    QSqlQuery query("SELECT id_client, nom, prenom, email, telephone, adresse, type_client, date_creation FROM clients");

    // 3. Afficher les données
    int row = 0;
    while (query.next()) {
        ui->tableWidget->insertRow(row);
        for (int col = 0; col < 8; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
            ui->tableWidget->setItem(row, col, item);
        }
        row++;
    }

    // 4. Optionnel: Ajuster la taille des colonnes
    ui->tableWidget->resizeColumnsToContents();
}
// Contrôle de saisie pour le nom
void ClientMainWindow::on_nomLineEdit_textChanged(const QString &text)
{
    QRegularExpression regex("^[A-Za-zÀ-ÿ]{0,10}$"); // Autorise uniquement les lettres (max 10)
    if (!regex.match(text).hasMatch()) {
        ui->nomLineEdit->backspace(); // Supprime le dernier caractère invalide
        QMessageBox::warning(this, "Erreur de saisie", "Le nom ne doit contenir que des lettres (max 10 caractères).");
    }
}

// Contrôle de saisie pour le prénom
void ClientMainWindow::on_prenomLineEdit_textChanged(const QString &text)
{
    QRegularExpression regex("^[A-Za-zÀ-ÿ]{0,10}$"); // Autorise uniquement les lettres (max 10)
    if (!regex.match(text).hasMatch()) {
        ui->prenomLineEdit->backspace(); // Supprime le dernier caractère invalide
        QMessageBox::warning(this, "Erreur de saisie", "Le prénom ne doit contenir que des lettres (max 10 caractères).");
    }
}

// Contrôle de saisie pour l'adresse
void ClientMainWindow::on_adresseLineEdit_textChanged(const QString &text)
{
    QRegularExpression regex("^[A-Za-z0-9À-ÿ ,.-]{0,10}$"); // Autorise lettres, chiffres et certains symboles (max 10)
    if (!regex.match(text).hasMatch()) {
        ui->adresseLineEdit->backspace(); // Supprime le dernier caractère invalide
        QMessageBox::warning(this, "Erreur de saisie", "L'adresse ne doit pas dépasser 10 caractères et ne peut contenir que des lettres, chiffres ou symboles autorisés (, . -).");
    }
}

// Contrôle de saisie pour le téléphone
void ClientMainWindow::on_telephoneLineEdit_textChanged(const QString &text)
{
    QRegularExpression regex("^[245789]\\d{0,7}$"); // 8 chiffres commençant par 2/4/5/7/8/9
    if (!regex.match(text).hasMatch()) {
        ui->telephoneLineEdit->backspace(); // Supprime le dernier caractère invalide
        QMessageBox::warning(this, "Erreur de saisie", "Le numéro doit commencer par 2/4/5/7/8/9 et avoir 8 chiffres.");
    }
}

// Contrôle de saisie pour l'email
void ClientMainWindow::on_emailLineEdit_textChanged(const QString &text)
{
    QRegularExpression allowedChars(R"(^[A-Za-z0-9._%+-@]*$)");
    if (!allowedChars.match(text).hasMatch()) {
        ui->emailLineEdit->backspace();
    }
}

void ClientMainWindow::on_emailLineEdit_editingFinished()
{
    QString email = ui->emailLineEdit->text().trimmed();
    if (email.isEmpty()) return;

    QRegularExpression emailRegex(R"(^[A-Za-z0-9._%+-]+@(gmail|yahoo|outlook|hotmail)\.(com|fr|net)$)");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Email invalide. Domaines acceptés: gmail.com, yahoo.com, etc.");
        ui->emailLineEdit->clear();
        ui->emailLineEdit->setFocus();
    }
}

void ClientMainWindow::on_ajouterButton_clicked()
{
    // Récupération des valeurs
    QString nom = ui->nomLineEdit->text();
    QString prenom = ui->prenomLineEdit->text();
    QString email = ui->emailLineEdit->text();
    QString telephone = ui->telephoneLineEdit->text();
    QString adresse = ui->adresseLineEdit->text();
    QString typeClient = ui->typeClientComboBox->currentText();

    // Validation
    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty() ||
        telephone.isEmpty() || adresse.isEmpty() || typeClient.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs doivent être remplis !");
        return;
    }

    // Création et ajout du client
    Client client(nom, prenom, email, telephone, adresse, typeClient, QDate::currentDate());
    int newId = client.ajouter();  // Un seul appel à ajouter()

    if(newId != -1) {
        // Mise à jour de l'interface
        afficherTable();
        afficherStats();

        QMessageBox::information(this, "Succès",
                                 QString("Client ajouté avec succès! (ID: %1)").arg(newId));

        // Réinitialisation des champs
        ui->nomLineEdit->clear();
        ui->prenomLineEdit->clear();
        ui->emailLineEdit->clear();
        ui->telephoneLineEdit->clear();
        ui->adresseLineEdit->clear();
        ui->typeClientComboBox->setCurrentIndex(0);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout du client !");
    }
}

void ClientMainWindow::on_modifierButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un client à modifier !");
        return;
    }

    // Récupérer l'ID depuis la première colonne
    int id = ui->tableWidget->item(row, 0)->text().toInt();
    QString nom = ui->nomLineEdit->text();
    QString prenom = ui->prenomLineEdit->text();
    QString email = ui->emailLineEdit->text();
    QString telephone = ui->telephoneLineEdit->text();
    QString adresse = ui->adresseLineEdit->text();
    QString typeClient = ui->typeClientComboBox->currentText();

    // Validation des champs
    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty() || telephone.isEmpty() || adresse.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs doivent être remplis !");
        return;
    }

    Client client;
    if (client.modifier(id, nom, prenom, email, telephone, adresse, typeClient)) {
        QMessageBox::information(this, "Succès", "Client modifié avec succès !");
        afficherTable(); // Rafraîchir le tableau
        afficherStats(); // Mettre à jour les statistiques
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification !");
    }
    ui->nomLineEdit->clear();
    ui->prenomLineEdit->clear();
    ui->emailLineEdit->clear();
    ui->telephoneLineEdit->clear();
    ui->adresseLineEdit->clear();
    ui->typeClientComboBox->setCurrentIndex(0);
}


void ClientMainWindow::on_supprimerButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un client !");
        return;
    }

    int id = ui->tableWidget->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Confirmation", "Supprimer ce client ?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;

    Client client;
    if (client.supprimer(id)) {
        QMessageBox::information(this, "Succès", "Client supprimé !");
        afficherTable();
        afficherStats();  // Mise à jour des statistiques après suppression
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de suppression !");
    }
}

void ClientMainWindow::setupStatsView()
{
    // 1. Trouver le QGraphicsView (adaptez le nom exact)
    QGraphicsView *statsView = findChild<QGraphicsView*>("graphicsView");
    if (!statsView) {
        qDebug() << "Erreur: QGraphicsView introuvable!";
        return;
    }

    // 2. Créer une scène
    QGraphicsScene *scene = new QGraphicsScene(this);
    statsView->setScene(scene);

    // 3. Créer un diagramme circulaire
    QPieSeries *series = new QPieSeries();
    series->append("Particuliers", 70);
    series->append("Entreprises", 30);

    // 4. Configurer l'apparence
    series->setLabelsVisible(true);
    for (QPieSlice *slice : series->slices()) {
        slice->setLabelVisible(true);
        slice->setLabelColor(Qt::black);
    }

    // 5. Créer et configurer le chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition clients");
    chart->legend()->setAlignment(Qt::AlignRight);

    // 6. Créer une vue pour le chart
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 7. Ajouter à la scène
    scene->addWidget(chartView);
}
void ClientMainWindow::setupPieChart()
{
    // Vérifier que le QGraphicsView existe dans l'interface
    if (!ui->graphicsView) { // Remplacez graphicsView par le nom exact de votre widget
        qDebug() << "Erreur: QGraphicsView introuvable!";
        return;
    }

    // Créer une scène graphique
    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    // Créer un diagramme circulaire
    QPieSeries *series = new QPieSeries();

    // Ajouter des données (exemple)
    series->append("Particuliers", 70);
    series->append("Entreprises", 30);

    // Configurer l'apparence
    series->setLabelsVisible(true);
    series->setLabelsPosition(QPieSlice::LabelOutside);

    // Créer et configurer le chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des clients");
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setAnimationOptions(QChart::AllAnimations);

    // Créer une vue pour le chart
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Ajouter la vue à la scène
    QGraphicsProxyWidget *proxy = scene->addWidget(chartView);
    proxy->setPos(0, 0);

    // Ajuster la taille
    chartView->resize(ui->graphicsView->size());
}
void ClientMainWindow::afficherStats()
{
    // 1. Vérifier que graphicsView existe
    if (!ui->graphicsView) {
        qDebug() << "Erreur: QGraphicsView introuvable!";
        return;
    }

    // 2. Récupérer les statistiques des clients
    Client::ClientStats stats = Client::calculerStats();

    // 3. Créer une nouvelle scène pour le graphique
    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    // 4. Créer le diagramme circulaire
    QPieSeries *series = new QPieSeries();

    if (stats.particuliers > 0) {
        QPieSlice *slice = series->append("Particuliers (" + QString::number(stats.particuliers) + ")", stats.particuliers);
        slice->setColor(QColor("#005F6B"));  // Bleu pétrole
        slice->setLabelVisible(true);
    }

    if (stats.entreprises > 0) {
        QPieSlice *slice = series->append("Entreprises (" + QString::number(stats.entreprises) + ")", stats.entreprises);
        slice->setColor(QColor("#8B4513"));  // Marron
        slice->setLabelVisible(true);
    }

    // 5. Créer et configurer le graphique
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des Clients (Total: " + QString::number(stats.totalClients) + ")");
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setAnimationOptions(QChart::AllAnimations);

    // 6. Ajouter le graphique dans QChartView
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 7. Nettoyer la scène et ajouter le nouveau graphique
    scene->clear();
    scene->addWidget(chartView);
}

// Fonction pour dessiner le graphique
void ClientMainWindow::drawBarChart(QGraphicsScene *scene, const Client::ClientStats &stats, int startX, int startY)
{
    const int barWidth = 30;
    const int maxHeight = 150;
    int maxValue = 0;

    // Trouver la valeur maximale pour l'échelle
    QMapIterator<QString, int> it(stats.clientsParMois);
    while (it.hasNext()) {
        it.next();
        if (it.value() > maxValue) maxValue = it.value();
    }

    // Dessiner les axes
    scene->addLine(startX, startY + maxHeight, startX + 300, startY + maxHeight, QPen(Qt::black)); // Axe X
    scene->addLine(startX, startY, startX, startY + maxHeight, QPen(Qt::black)); // Axe Y

    // Dessiner les barres
    int xPos = startX + 20;
    it.toFront(); // Réinitialiser l'itérateur

    while (it.hasNext()) {
        it.next();
        QString month = it.key();
        int count = it.value();
        int barHeight = (count * maxHeight) / qMax(1, maxValue);

        // Barre
        scene->addRect(xPos, startY + maxHeight - barHeight,
                       barWidth, barHeight,
                       QPen(Qt::black), QBrush(QColor("#FFAE9D")));

        // Étiquette mois
        QGraphicsTextItem *monthLabel = scene->addText(month.mid(5,2) + "/" + month.mid(2,2));
        monthLabel->setPos(xPos - 5, startY + maxHeight + 5);
        monthLabel->setFont(QFont("Arial", 8));

        // Valeur
        QGraphicsTextItem *valueLabel = scene->addText(QString::number(count));
        valueLabel->setPos(xPos, startY + maxHeight - barHeight - 20);
        valueLabel->setFont(QFont("Arial", 8));

        xPos += barWidth + 20;
    }
}
void ClientMainWindow::on_rechercherButton_clicked()
{
    QString critere = ui->searchFrame->text().trimmed();
    if (critere.isEmpty()) {
        afficherTable(); // Afficher tous les clients si le champ est vide
        return;
    }

    QSqlQueryModel *model = Client::rechercher(critere);
    ui->tableWidget->setRowCount(model->rowCount());
    ui->tableWidget->setColumnCount(model->columnCount());

    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(model->data(model->index(row, col)).toString());
            ui->tableWidget->setItem(row, col, item);
        }
    }
}


void ClientMainWindow::on_triNomButton_clicked()
{
    static bool ascendant = true;
    QSqlQueryModel *model = Client::trierParNom(ascendant);
    afficherModelDansTable(model);
    ascendant = !ascendant;

    ui->triNomButton->setText(ascendant ? "Tri par Nom ▲" : "Tri par Nom ▼");
}
void ClientMainWindow::on_triDateButton_clicked()
{
    static bool ascendant = true;
    QSqlQueryModel *model = Client::trierParDate(ascendant);
    afficherModelDansTable(model);
    ascendant = !ascendant;


    ui->triDateButton->setText(ascendant ? "Tri par Date ▲" : "Tri par Date ▼");
}

void ClientMainWindow::on_lineEdit_textChanged(const QString &text)
{
    if (text.isEmpty()) {
        afficherTable();
    } else {
        // Appeler la recherche seulement si le texte a au moins 2 caractères
        if (text.length() >= 2) {
            QSqlQueryModel *model = Client::rechercher(text);
            afficherModelDansTable(model);
        }
    }
}


void ClientMainWindow::afficherModelDansTable(QSqlQueryModel *model)
{
    ui->tableWidget->setRowCount(model->rowCount());
    ui->tableWidget->setColumnCount(model->columnCount());

    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(model->data(model->index(row, col)).toString());
            ui->tableWidget->setItem(row, col, item);
        }
    }
}


void ClientMainWindow::exporterEnPDF()
{
    QSqlQueryModel *model = Client::afficher();

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF",
                                                    QDir::homePath(),
                                                    "Fichiers PDF (*.pdf)");
    if (fileName.isEmpty())
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(15, 15, 15, 15));

    QString html;
    html += "<h1 align='center'>Liste des Clients</h1>";
    html += "<h3 align='center'>" + QDate::currentDate().toString("dd/MM/yyyy") + "</h3>";
    html += "<table border='1' cellspacing='0' cellpadding='3' width='100%'>";

    // En-têtes de colonnes
    html += "<tr bgcolor='#FFAE9D'>";
    for (int col = 0; col < model->columnCount(); ++col) {
        html += "<th>" + model->headerData(col, Qt::Horizontal).toString() + "</th>";
    }
    html += "</tr>";

    // Données
    for (int row = 0; row < model->rowCount(); ++row) {
        html += "<tr>";
        for (int col = 0; col < model->columnCount(); ++col) {
            html += "<td>" + model->data(model->index(row, col)).toString() + "</td>";
        }
        html += "</tr>";
    }
    html += "</table>";

    // Générer le PDF
    QTextDocument document;
    document.setHtml(html);
    document.print(&printer);

    QMessageBox::information(this, "Succès", "Export PDF terminé avec succès!");
}

void ClientMainWindow::on_bsms_clicked()
{
    Client::notifierClientsFideles();  // Appel de la méthode statique

    QMessageBox::information(this, "SMS envoyés", "Les clients fidèles ont été notifiés par SMS.");
}
QString ClientMainWindow::generatePromptFromUI()
{
    QString typeMaison = ui->comboBox_type->currentText();      // S+1, S+2, villa, etc.
    QString styleMaison = ui->comboBox->currentText();          // moderne, traditionnel

    // Infos quantitatives depuis les champs numériques (spinBox par exemple)
    int nbChambres = ui->spinBox_chambreslab->value();
    int nbCuisines = ui->spinBox_cuisines->value();
    int nbSallesBain = ui->spinBox_sdb->value();
    int nbSall = ui->spinBox_s->value();

    QString prompt;

    // Description principale
    prompt += "Plan architectural 3D vue du dessus d'une maison " + typeMaison +
              " de style " + styleMaison + ", conforme aux normes PMR, avec :\n";

    // Composition des pièces
    prompt += QString("- %1 chambre(s) à coucher\n").arg(nbChambres);
    prompt += QString("- %1 cuisine(s)\n").arg(nbCuisines);
    prompt += QString("- %1 salle(s) de bain\n").arg(nbSallesBain);
    prompt += QString("- %1 salle(s) d'eau\n").arg(nbSall);

    // Éléments extérieurs cochés
    QStringList elements;
    if (ui->garage->isChecked()) elements << "un garage intégré";
    if (ui->jardin->isChecked()) elements << "un jardin paysagé";
    if (ui->picine->isChecked()) elements << "une piscine moderne";
    if (!elements.isEmpty())
        prompt += "- Éléments extérieurs : " + elements.join(", ") + "\n";

    // Contraintes de rendu
    prompt += R"(
Vue isométrique éclatée avec :
- Murs partiellement coupés pour voir l'intérieur
- Mobilier réaliste dans chaque pièce
- Dimensions et cotes visibles
- Étiquettes claires pour chaque pièce
- Textures réalistes : bois, carrelage, béton
- Lumière naturelle simulée
- Fond blanc, style architectural propre
- Rendu HD
)";

    return prompt;
}


void ClientMainWindow::onGenerateClicked()
{
    // Affiche une alerte pour informer l'utilisateur
    QMessageBox::information(this, "Génération", "L'image est en cours de génération...");

    QNetworkRequest request(QUrl("https://api.openai.com/v1/images/generations"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    const QByteArray openAiKey = qgetenv("OPENAI_API_KEY");
    if (openAiKey.isEmpty()) {
        QMessageBox::warning(this, "Configuration manquante", "OPENAI_API_KEY non configuree.");
        return;
    }
    request.setRawHeader("Authorization", "Bearer " + openAiKey);

    QJsonObject json{
        {"model", "dall-e-3"},
        {"prompt", generatePromptFromUI()}, // ou generatePromptFromUI()
        {"n", 1},
        {"size", "1792x1024"},
        {"quality", "hd"}
    };

    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "Erreur API", reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            QMessageBox::critical(this, "Erreur JSON", parseError.errorString());
            reply->deleteLater();
            return;
        }

        QJsonArray dataArray = jsonDoc.object().value("data").toArray();
        if (dataArray.isEmpty()) {
            QMessageBox::critical(this, "Erreur", "Aucune image générée.");
            reply->deleteLater();
            return;
        }

        QString imageUrl = dataArray[0].toObject().value("url").toString();
        if (imageUrl.isEmpty()) {
            QMessageBox::critical(this, "Erreur", "Lien image vide.");
            reply->deleteLater();
            return;
        }

        // Télécharger l'image
        QNetworkReply *imageReply = m_networkManager->get(QNetworkRequest(QUrl(imageUrl)));
        connect(imageReply, &QNetworkReply::finished, [this, imageReply]() {
            if (imageReply->error() != QNetworkReply::NoError) {
                QMessageBox::critical(this, "Erreur image", imageReply->errorString());
                imageReply->deleteLater();
                return;
            }

            QPixmap pixmap;
            if (!pixmap.loadFromData(imageReply->readAll())) {
                QMessageBox::critical(this, "Erreur", "Impossible de charger l'image.");
                imageReply->deleteLater();
                return;
            }

            // Affichage dans le GraphicsView
            QGraphicsScene *scene = new QGraphicsScene(this);
            scene->setBackgroundBrush(Qt::white);
            QGraphicsPixmapItem *item = scene->addPixmap(pixmap);
            ui->graphicsView_3D->setScene(scene);
            ui->graphicsView_3D->setRenderHints(
                QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
            ui->graphicsView_3D->fitInView(item, Qt::KeepAspectRatioByExpanding);

            imageReply->deleteLater();
        });

        reply->deleteLater();
    });
}

void ClientMainWindow::onOpenAIReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(this, "Erreur API", reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QString responseText;

    if (jsonDoc.isObject()) {
        QJsonObject obj = jsonDoc.object();
        if (obj.contains("choices")) {
            responseText = obj["choices"].toArray()[0].toObject()["message"].toObject()["content"].toString();
        }
    }

    QMessageBox::information(this, "Réponse OpenAI", responseText);
    reply->deleteLater();
}
