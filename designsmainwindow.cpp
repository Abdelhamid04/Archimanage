
#include "designsmainwindow.h"
#include "ui_designsmainwindow.h"
#include "designs.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDir>
#include <QBuffer>
#include <QPrinter>
#include <QTextDocument>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QTextBrowser>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QProgressBar> // Ajoutez cette ligne
#include <QJsonArray>  // Ajoutez cette ligne
#include <QStackedWidget>
#include "login.h"
#include "employe.h"

void designsMainWindow::handleImageLink(const QUrl &url)
{
    if (url.scheme() == "save" && !lastGeneratedImage.isNull()) {
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Enregistrer l'image",
            QDir::homePath() + "/design_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".png",
            "Images (*.png *.jpg)"
            );

        if (!fileName.isEmpty()) {
            lastGeneratedImage.save(fileName);
        }
    }
}
void designsMainWindow::handleImageSave(const QUrl &link)
{
    // Pour les deux solutions
    if ((link.scheme() == "saveimage" || link.toString() == "saveimage://save")
        && !lastGeneratedImage.isNull()) {
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Enregistrer l'image",
            QDir::homePath() + "/design_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".png",
            "Images (*.png *.jpg)"
            );

        if (!fileName.isEmpty() && lastGeneratedImage.save(fileName)) {
            ui->chatArea->append("<p>Image sauvegardée: " + fileName + "</p>");
        }
    }
}

void designsMainWindow::generateImageFromPrompt(const QString &prompt)
{
    if (prompt.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer une description pour l'image.");
        return;
    }

    isProcessingImage = true;
    ui->sendButton->setEnabled(false);
    ui->chatArea->append("<p><b>Vous:</b> " + prompt.toHtmlEscaped() + "</p>");
    ui->chatArea->append("<p><i>Génération en cours...</i></p>");

    QUrl url("https://api.openai.com/v1/images/generations");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QByteArray apiKey = qgetenv("OPENAI_API_KEY");
    if (apiKey.isEmpty()) {
        ui->chatArea->append("<p style='color:red;'>OPENAI_API_KEY non configuree.</p>");
        isProcessingImage = false;
        ui->sendButton->setEnabled(true);
        return;
    }
    request.setRawHeader("Authorization", "Bearer " + apiKey);

    QJsonObject json;
    json.insert("prompt", prompt);
    json.insert("n", 1);
    json.insert("size", "512x512");
    json.insert("response_format", "url");

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, prompt]() {
        isProcessingImage = false;
        ui->sendButton->setEnabled(true);

        if (reply->error() != QNetworkReply::NoError) {
            ui->chatArea->append("<p style='color:red;'>Erreur: " + reply->errorString() + "</p>");
            reply->deleteLater();
            return;
        }

        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
        QJsonObject jsonObject = jsonResponse.object();

        if (jsonObject.contains("data")) {
            QJsonArray dataArray = jsonObject["data"].toArray();
            if (!dataArray.isEmpty()) {
                QString imageUrl = dataArray[0].toObject()["url"].toString();

                QNetworkRequest imageRequest(imageUrl);
                QNetworkReply* imageReply = imageDownloadManager->get(imageRequest);

                connect(imageReply, &QNetworkReply::finished, this, [this, imageReply, prompt]() {
                    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> replyPtr(imageReply);

                    if (imageReply->error() == QNetworkReply::NoError) {
                        QByteArray imageData = imageReply->readAll();
                        QPixmap pixmap;
                        if (pixmap.loadFromData(imageData)) {
                            displayGeneratedImage(pixmap, prompt);

                            // Sauvegarde automatique optionnelle
                            QString saveDir = QDir::homePath() + "/AI_Images/";
                            QDir().mkpath(saveDir);
                            QString fileName = saveDir + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".png";

                            if (pixmap.save(fileName)) {
                                ui->chatArea->append("<p><small>Image sauvegardée dans: " + fileName + "</small></p>");
                            }
                        }
                    }
                });
            }
        }
        reply->deleteLater();
    });
}

void designsMainWindow::setCurrentUser(int userId)
{
    currentUserId = userId;
}



void designsMainWindow::onSearchClicked()
{
    // 1. Nettoyer les anciens résultats
    QLayout* mainLayout = this->layout();

    if (mainLayout) {
        QLayoutItem* child;
        while ((child = mainLayout->takeAt(0)) != nullptr) {
            if (child->widget()) {
                child->widget()->deleteLater();
            }
            delete child;
        }
    }

    // 2. Exécuter la requête
    QSqlQuery query;
    if (!query.prepare("SELECT id, nom, image_data FROM MA_TABLE_IMAGES "
                       "WHERE (LOWER(description) LIKE ? OR LOWER(tags) LIKE ?) "
                       "AND ROWNUM <= 10")) {
        qDebug() << "Erreur de préparation de requête:" << query.lastError();
        return;
    }

    query.addBindValue("%" + ui->searchLineEdit->text().toLower() + "%");
    query.addBindValue("%" + ui->searchLineEdit->text().toLower() + "%");

    if (!query.exec()) {
        qDebug() << "Erreur d'exécution de requête:" << query.lastError();
        return;
    }

    // 3. Créer un nouveau conteneur scrollable
    QScrollArea* scrollArea = new QScrollArea(this);
    QWidget* container = new QWidget(scrollArea);
    QGridLayout* gridLayout = new QGridLayout(container);

    int row = 0, col = 0;
    const int colsPerRow = 3;

    // 4. Afficher les résultats
    while (query.next()) {
        QWidget* itemWidget = new QWidget(container);
        QVBoxLayout* itemLayout = new QVBoxLayout(itemWidget);

        // Image
        QLabel* imgLabel = new QLabel(itemWidget);
        QByteArray imgData = query.value(2).toByteArray();
        QPixmap pixmap;
        pixmap.loadFromData(imgData);
        imgLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));

        // Description
        QLabel* descLabel = new QLabel(
            QString("%1\nID: %2").arg(query.value(1).toString())
                .arg(query.value(0).toInt()),
            itemWidget
            );

        // Assemblage
        itemLayout->addWidget(imgLabel);
        itemLayout->addWidget(descLabel);
        itemWidget->setLayout(itemLayout);

        // Positionnement dans la grille
        gridLayout->addWidget(itemWidget, row, col);
        col = (col + 1) % colsPerRow;
        if (col == 0) row++;
    }

    // 5. Finaliser l'interface
    container->setLayout(gridLayout);
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);

    if (!mainLayout) {
        mainLayout = new QVBoxLayout(this);
        this->setLayout(mainLayout);
    }
    mainLayout->addWidget(scrollArea);
}

designsMainWindow::designsMainWindow(const employe& e, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::designsMainWindow)
    , currentUserId(-1) // -1 = non connecté
    ,employeConnecte(e)

{
    ui->setupUi(this);

    afficherTable();
    setupStatistiques();
    networkManager = new QNetworkAccessManager(this);
    imageDownloadManager = new QNetworkAccessManager(this);

    // Configurer chatArea
    ui->chatArea->setOpenLinks(false);
    ui->chatArea->setOpenExternalLinks(false);
    connect(ui->chatArea, &QTextBrowser::anchorClicked, this, &designsMainWindow::handleImageSave);

    // Connecter le bouton de génération
    connect(ui->sendButton, &QPushButton::clicked, this, [this]() {
        QString prompt = ui->inputField->text().trimmed();
        ui->inputField->clear();
        if (!prompt.isEmpty()) {
            generateImageFromPrompt(prompt);
        }
    });

}
void designsMainWindow::displayGeneratedImage(const QPixmap &pixmap, const QString &prompt)
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");

    // Taille très réduite (miniature)
    const int thumbnailWidth = 100;  // Largeur désirée en pixels
    const int thumbnailHeight = 100; // Hauteur désirée en pixels

    QString html = QString(
                       "<div style='margin:2px; padding:2px; display:inline-block;'>"
                       "  <img src='data:image/png;base64,%1' "
                       "       width='%2' height='%3' "
                       "       style='border:1px solid #ddd; cursor:pointer;'"
                       "       onclick=\"this.style.width=this.style.width=='%2px'?'300px':'%2px'\"/>"
                       "  <div style='font-size:8pt; color:#666;'>%4</div>"
                       "  <p><a href='saveimage://save' style='color:blue; text-decoration:none;'>"
                       "     [Télécharger]</a></p>"
                       "</div>"
                       ).arg(QString(imageData.toBase64()),
                            QString::number(thumbnailWidth),
                            QString::number(thumbnailHeight),
                            prompt.toHtmlEscaped().left(20) + (prompt.length() > 20 ? "..." : ""));

    ui->chatArea->append(html);
    lastGeneratedImage = pixmap;
}


designsMainWindow::~designsMainWindow()
{
    delete ui;
}

void designsMainWindow::afficherTable()
{
    QSqlQueryModel *model = Design::afficher();
    ui->tableWidget->setRowCount(model->rowCount());
    ui->tableWidget->setColumnCount(model->columnCount());

    QStringList headers = {"ID", "Nom", "Type", "Date de création", "IDE", "Description", "Image"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < model->rowCount(); ++i) {
        for (int j = 0; j < model->columnCount(); ++j) {
            if (j == 6) { // Colonne Image
                QByteArray imageData = model->data(model->index(i, j)).toByteArray();
                QPixmap pixmap;
                pixmap.loadFromData(imageData);
                QLabel *imageLabel = new QLabel;
                imageLabel->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->tableWidget->setCellWidget(i, j, imageLabel);
            } else if (j == 3) { // Colonne Date
                QDateTime dateTime = model->data(model->index(i, j)).toDateTime();
                QString dateOnly = dateTime.date().toString("yyyy-MM-dd");
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(dateOnly));
            } else {
                QString value = model->data(model->index(i, j)).toString();
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(value));
            }
        }
    }
    ui->tableWidget->resizeColumnsToContents();
}

void designsMainWindow::setupStatistiques()
{
    // 1. Vérifier si un layout existe déjà
    if (ui->chartWidget->layout() != nullptr) {
        QLayout* oldLayout = ui->chartWidget->layout();

        // Supprimer tous les widgets du layout
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }

        // Supprimer le layout lui-même
        delete oldLayout;
    }

    // 2. Requête SQL
    QSqlQuery query;
    query.prepare("SELECT TYPE_DESIGN, COUNT(*) as count FROM DESIGNS GROUP BY TYPE_DESIGN");

    if (!query.exec()) {
        qDebug() << "Erreur statistiques:" << query.lastError().text();
        return;
    }

    QPieSeries *series = new QPieSeries();
    int totalCount = 0;

    // 3. Calcul du total
    while (query.next()) {
        totalCount += query.value(1).toInt();
    }

    // 4. Repositionner le curseur pour repartir
    query.first();
    query.previous();

    // 5. Ajout des tranches avec pourcentage
    while (query.next()) {
        QString type = query.value(0).toString().trimmed();
        int count = query.value(1).toInt();
        double percentage = (count * 100.0) / totalCount;

        QString label = QString("%1 (%2%)")
                            .arg(type.isEmpty() ? "Autre" : type)
                            .arg(percentage, 0, 'f', 1);

        QPieSlice *slice = series->append(label, count);
        slice->setLabelVisible(true);
    }

    // 6. Création du graphique
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des Designs par Type");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 7. Intégration dans le layout
    QVBoxLayout *layout = new QVBoxLayout(ui->chartWidget);
    layout->addWidget(chartView);
    ui->chartWidget->setLayout(layout);
}


void designsMainWindow::on_selectImageButton_clicked()
{
    imagePath = QFileDialog::getOpenFileName(this, "Sélectionner une image", "", "Images (*.png *.jpg *.jpeg)");
    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible de charger l'image !");
        }
    }
}

void designsMainWindow::on_ajouterButton_clicked()
{
    QString nom = ui->nomLineEdit->text();
    QString type = ui->typeComboBox->currentText();
    QDate dateCreation = ui->dateCreationDateEdit->date();
    int ide = ui->ideLineEdit->text().toInt();
    QString description = ui->descriptionTextEdit->toPlainText();

    // Validation des champs
    if (nom.isEmpty() || description.isEmpty() || imagePath.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs sont obligatoires !");
        return;
    }

    // Préparation de l'image
    QByteArray imageData;
    QFile imageFile(imagePath);
    if (imageFile.open(QIODevice::ReadOnly)) {
        imageData = imageFile.readAll();
        imageFile.close();
    } else {
        QMessageBox::warning(this, "Erreur", "Impossible de lire l'image !");
        return;
    }

    Design design(nom, type, dateCreation, ide, description, imageData);
    if (design.ajouter()) {
        QMessageBox::information(this, "Succès", "Design ajouté avec succès !");
        afficherTable();
        setupStatistiques();

        // Réinitialisation des champs
        ui->nomLineEdit->clear();
        ui->ideLineEdit->clear();
        ui->descriptionTextEdit->clear();
        ui->imageLabel->clear();
        imagePath = "";
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout du design !");
    }
}

void designsMainWindow::on_modifierButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un design à modifier !");
        return;
    }

    int id = ui->tableWidget->item(row, 0)->text().toInt();
    QString nom = ui->tableWidget->item(row, 1)->text();
    QString type = ui->tableWidget->item(row, 2)->text();
    QDate dateCreation = QDate::fromString(ui->tableWidget->item(row, 3)->text(), "yyyy-MM-dd");
    int ide = ui->tableWidget->item(row, 4)->text().toInt();
    QString description = ui->tableWidget->item(row, 5)->text();

    // Remplir les champs avec les données existantes
    ui->nomLineEdit->setText(nom);
    ui->typeComboBox->setCurrentText(type);
    ui->dateCreationDateEdit->setDate(dateCreation);
    ui->ideLineEdit->setText(QString::number(ide));
    ui->descriptionTextEdit->setPlainText(description);

    // Charger l'image existante
    QByteArray imageData;
    QLabel *imageLabel = qobject_cast<QLabel*>(ui->tableWidget->cellWidget(row, 6));
    if (imageLabel) {
        QPixmap pixmap = imageLabel->pixmap();
        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
    }

    // Modifier le bouton Ajouter en Modifier
    ui->ajouterButton->setText("Modifier");
    ui->ajouterButton->disconnect();
    connect(ui->ajouterButton, &QPushButton::clicked, this, [=]() {
        on_modifierButton_confirmed(id);
    });
}

void designsMainWindow::on_modifierButton_confirmed(int id)
{
    QString nom = ui->nomLineEdit->text();
    QString type = ui->typeComboBox->currentText();
    QDate dateCreation = ui->dateCreationDateEdit->date();
    int ide = ui->ideLineEdit->text().toInt();
    QString description = ui->descriptionTextEdit->toPlainText();
    QByteArray imageData;

    // Si une nouvelle image a été sélectionnée
    if (!imagePath.isEmpty()) {
        QFile imageFile(imagePath);
        if (imageFile.open(QIODevice::ReadOnly)) {
            imageData = imageFile.readAll();
            imageFile.close();
        }
    } else {
        // Garder l'image existante
        int row = ui->tableWidget->currentRow();
        QLabel *imageLabel = qobject_cast<QLabel*>(ui->tableWidget->cellWidget(row, 6));
        if (imageLabel) {
            QPixmap pixmap = imageLabel->pixmap();
            QBuffer buffer(&imageData);
            buffer.open(QIODevice::WriteOnly);
            pixmap.save(&buffer, "PNG");
        }
    }

    Design design(nom, type, dateCreation, ide, description, imageData);
    if (design.modifier(id)) {
        QMessageBox::information(this, "Succès", "Design modifié avec succès !");
        afficherTable();
        setupStatistiques();

        // Réinitialiser le bouton
        ui->ajouterButton->setText("Ajouter");
        ui->ajouterButton->disconnect();
        connect(ui->ajouterButton, &QPushButton::clicked, this, &designsMainWindow::on_ajouterButton_clicked);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification du design !");
    }
}

void designsMainWindow::on_supprimerButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un design à supprimer !");
        return;
    }

    int id = ui->tableWidget->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Confirmation", "Êtes-vous sûr de vouloir supprimer ce design ?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    Design design("", "", QDate(), 0, "", QByteArray());
    if (design.supprimer(id)) {
        QMessageBox::information(this, "Succès", "Design supprimé avec succès !");
        afficherTable();
        setupStatistiques();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression du design !");
    }
}

void designsMainWindow::on_exportPdfButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "", "PDF (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    QString html = "<h1>Liste des Designs</h1><table border='1'><tr>";

    // En-têtes
    for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
        html += "<th>" + ui->tableWidget->horizontalHeaderItem(col)->text() + "</th>";
    }
    html += "</tr>";

    // Données
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        html += "<tr>";
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            if (col == 6) { // Image
                html += "<td>[Image]</td>";
            } else {
                QTableWidgetItem *item = ui->tableWidget->item(row, col);
                html += "<td>" + (item ? item->text() : "") + "</td>";
            }
        }
        html += "</tr>";
    }
    html += "</table>";

    doc.setHtml(html);
    doc.print(&printer);
    QMessageBox::information(this, "Succès", "Export PDF terminé !");
}

void designsMainWindow::on_searchButton_clicked()
{
    QString searchTerm = ui->searchLineEdit->text();
    QString criteria = ui->sortComboBox->currentText();

    if (searchTerm.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Entrez un terme de recherche !");
        return;
    }

    QSqlQuery query;
    QString queryStr = "SELECT * FROM DESIGNS WHERE ";

    if (criteria == "ID") {
        queryStr += "TO_CHAR(ID_DESIGN) LIKE ? ORDER BY ID_DESIGN";
    } else if (criteria == "Nom") {
        queryStr += "LOWER(NOM_DESIGN) LIKE ? ORDER BY NOM_DESIGN";
    } else if (criteria == "Type") {
        queryStr += "LOWER(TYPE_DESIGN) LIKE ? ORDER BY TYPE_DESIGN";
    } else if (criteria == "Date") {
        queryStr += "TO_CHAR(DATE_CREATION, 'YYYY-MM-DD') LIKE ? ORDER BY DATE_CREATION";
    } else {
        QMessageBox::warning(this, "Erreur", "Critère de recherche invalide !");
        return;
    }
    ui->tableWidget->setSortingEnabled(true);

    query.prepare(queryStr);
    query.addBindValue("%" + searchTerm.toLower() + "%");

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur", "Erreur de recherche : " + query.lastError().text());
        return;
    }

    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(query);


    // Affichage dans la table
    ui->tableWidget->setRowCount(model->rowCount());
    ui->tableWidget->setColumnCount(model->columnCount());

    QStringList headers = {"ID", "Nom", "Type", "Date de création", "IDE", "Description", "Image"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < model->rowCount(); ++i) {
        for (int j = 0; j < model->columnCount(); ++j) {
            if (j == 6) { // Colonne image
                QByteArray imageData = model->data(model->index(i, j)).toByteArray();
                QPixmap pixmap;
                pixmap.loadFromData(imageData);
                QLabel *imageLabel = new QLabel;
                imageLabel->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->tableWidget->setCellWidget(i, j, imageLabel);
            } else if (j == 3) { // Date
                QDateTime dateTime = model->data(model->index(i, j)).toDateTime();
                QString dateOnly = dateTime.date().toString("yyyy-MM-dd");
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(dateOnly));
            } else {
                QString value = model->data(model->index(i, j)).toString();
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(value));
            }
        }
    }

    ui->tableWidget->resizeColumnsToContents();
}


void designsMainWindow::on_resetButton_clicked()
{
    ui->searchLineEdit->clear();
    ui->sortComboBox->setCurrentIndex(0);
    afficherTable();
}

void designsMainWindow::on_pushButton_18_clicked() // Catalogue
{
    QDialog *bookDialog = new QDialog(this);
    bookDialog->setWindowTitle("Catalogue des Designs - Mode Livre");
    bookDialog->resize(900, 650);
    bookDialog->setStyleSheet("QDialog { background-color: #FAF2EA; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(bookDialog);

    // Boutons de navigation
    QPushButton *prevButton = new QPushButton("< Précédent");
    QPushButton *nextButton = new QPushButton("Suivant >");

    QString buttonStyle = "QPushButton {"
                          "    background-color: #FFAE9D;"
                          "    color: #392E2C;"
                          "    border: 2px solid #CCAEA4;"
                          "    border-radius: 10px;"
                          "    padding: 5px 15px;"
                          "    font-size: 14px;"
                          "}"
                          "QPushButton:hover {"
                          "    background-color: #B39188;"
                          "    border-color: #FFAE9D;"
                          "}";

    prevButton->setStyleSheet(buttonStyle);
    nextButton->setStyleSheet(buttonStyle);

    // StackedWidget pour les pages
    QStackedWidget *pageStack = new QStackedWidget();
    pageStack->setMinimumSize(800, 550);

    // Récupérer les images avec les noms
    QList<QPair<QString, QByteArray>> designs = Design::getAllImagesWithNames();

    // Création des pages
    int itemsPerPage = 2;
    int pageCount = (designs.size() + itemsPerPage - 1) / itemsPerPage;

    for (int page = 0; page < pageCount; page++) {
        QWidget *pageWidget = new QWidget();
        QHBoxLayout *pageLayout = new QHBoxLayout(pageWidget);

        for (int i = 0; i < itemsPerPage; i++) {
            int index = page * itemsPerPage + i;
            if (index >= designs.size()) break;

            const auto &design = designs[index];
            QPixmap pixmap;
            pixmap.loadFromData(design.second);

            if (!pixmap.isNull()) {
                QFrame *frame = new QFrame();
                frame->setFrameShape(QFrame::StyledPanel);
                frame->setStyleSheet("QFrame { background: white; border-radius: 5px; }");
                frame->setFixedSize(350, 500);

                QVBoxLayout *frameLayout = new QVBoxLayout(frame);
                frameLayout->setAlignment(Qt::AlignCenter);

                QLabel *imageLabel = new QLabel();
                imageLabel->setPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                imageLabel->setAlignment(Qt::AlignCenter);

                QLabel *infoLabel = new QLabel(QString("%1\nPage %2/%3")
                                                   .arg(design.first) // Affiche le nom au lieu de l'ID
                                                   .arg(page + 1)
                                                   .arg(pageCount));
                infoLabel->setAlignment(Qt::AlignCenter);
                infoLabel->setStyleSheet("color: #392E2C; font-weight: bold;");

                frameLayout->addWidget(imageLabel);
                frameLayout->addWidget(infoLabel);

                pageLayout->addWidget(frame, 0, Qt::AlignCenter);
            }
        }

        pageStack->addWidget(pageWidget);
    }

    // [...] (le reste du code reste inchangé)
    // Navigation
    connect(prevButton, &QPushButton::clicked, [pageStack]() {
        int current = pageStack->currentIndex();
        if (current > 0) pageStack->setCurrentIndex(current - 1);
    });

    connect(nextButton, &QPushButton::clicked, [pageStack, pageCount]() {
        int current = pageStack->currentIndex();
        if (current < pageCount - 1) pageStack->setCurrentIndex(current + 1);
    });

    // Layout
    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->addWidget(prevButton);
    navLayout->addStretch();
    navLayout->addWidget(nextButton);

    mainLayout->addLayout(navLayout);
    mainLayout->addWidget(pageStack);

    // Bouton Fermer
    QPushButton *closeButton = new QPushButton("Fermer");
    closeButton->setStyleSheet(buttonStyle);
    connect(closeButton, &QPushButton::clicked, bookDialog, &QDialog::close);

    mainLayout->addWidget(closeButton, 0, Qt::AlignRight);

    bookDialog->exec();
}
void designsMainWindow::on_exportExcelButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en Excel", "", "Fichiers Excel (*.xlsx)");
    if (fileName.isEmpty())
        return;

    // Ajouter l'extension si elle n'est pas présente
    if (!fileName.endsWith(".xlsx"))
        fileName += ".xlsx";

    // Créer un document Excel
    QAxObject excel("Excel.Application");
    if (excel.isNull()) {
        QMessageBox::critical(this, "Erreur", "Excel n'est pas installé !");
        return;
    }

    excel.setProperty("Visible", false);
    QAxObject *workbooks = excel.querySubObject("Workbooks");
    QAxObject *workbook = workbooks->querySubObject("Add");
    QAxObject *sheets = workbook->querySubObject("Worksheets");
    QAxObject *sheet = sheets->querySubObject("Item(int)", 1);

    // Écrire les en-têtes
    for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
        QAxObject *cell = sheet->querySubObject("Cells(int,int)", 1, col+1);
        cell->setProperty("Value", ui->tableWidget->horizontalHeaderItem(col)->text());
        delete cell;
    }

    // Écrire les données
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidget->item(row, col);
            if (item) {
                QAxObject *cell = sheet->querySubObject("Cells(int,int)", row+2, col+1);
                cell->setProperty("Value", item->text());
                delete cell;
            }
        }
    }

    // Sauvegarder et fermer
    workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(fileName));
    workbook->dynamicCall("Close()");
    excel.dynamicCall("Quit()");

    QMessageBox::information(this, "Succès", "Export Excel terminé !");
}
void designsMainWindow::on_importExcelButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Importer depuis Excel", "", "Fichiers Excel (*.xlsx *.xls)");
    if (fileName.isEmpty())
        return;

    QAxObject excel("Excel.Application");
    if (excel.isNull()) {
        QMessageBox::critical(this, "Erreur", "Excel n'est pas installé !");
        return;
    }

    excel.setProperty("Visible", false);
    QAxObject *workbooks = excel.querySubObject("Workbooks");
    QAxObject *workbook = workbooks->querySubObject("Open(const QString&)", QDir::toNativeSeparators(fileName));
    QAxObject *sheet = workbook->querySubObject("Worksheets")->querySubObject("Item(int)", 1);
    QAxObject *usedRange = sheet->querySubObject("UsedRange");
    QAxObject *rows = usedRange->querySubObject("Rows");
    QAxObject *columns = usedRange->querySubObject("Columns");

    int rowCount = rows->property("Count").toInt();
    int colCount = columns->property("Count").toInt();

    // Vider la table actuelle
    ui->tableWidget->setRowCount(0);

    // Lire les données (en ignorant la première ligne d'en-têtes)
    for (int row = 2; row <= rowCount; ++row) {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        for (int col = 1; col <= colCount; ++col) {
            QAxObject *cell = sheet->querySubObject("Cells(int,int)", row, col);
            QVariant value = cell->property("Value");
            delete cell;

            if (col <= ui->tableWidget->columnCount()) {
                QTableWidgetItem *item = new QTableWidgetItem(value.toString());
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, col-1, item);
            }
        }
    }

    // Fermer Excel
    workbook->dynamicCall("Close()");
    excel.dynamicCall("Quit()");

    QMessageBox::information(this, "Succès", "Importation Excel terminée !");
}
