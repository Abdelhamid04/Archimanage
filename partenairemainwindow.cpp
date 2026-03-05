#include "partenairemainwindow.h"
#include "ui_partenairemainwindow.h"
#include "partenaire.h"
#include "arduino.h"
#include <QWidget>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextBrowser>
#include <QtNetwork>
#include <QSslSocket>
PartenaireMainWindow::PartenaireMainWindow(const employe& e, QWidget *parent):
    QWidget(parent),
    ui(new Ui::PartenaireMainWindow),
    employeConnecte(e)
{
    ui->setupUi(this);

    arduino2 = new Arduino(this);
    arduino2->setPortName("COM4");

    int connectionStatus = arduino2->connect_arduino();

    if(connectionStatus == 0) {
        ui->textEditLogs->append("Arduino connecté sur " + arduino2->getarduino_port_name());
        connect(arduino2->getserial(), &QSerialPort::readyRead, this, &PartenaireMainWindow::readSerialData);
    }
    else if(connectionStatus == 1) {
        ui->textEditLogs->append("Échec de l'ouverture du port Arduino");
    }
    else {
        ui->textEditLogs->append("Aucun Arduino détecté !");
    }
    afficherTable();
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    setupStatistiques();
    connect(ui->historyButton, &QPushButton::clicked, this, &PartenaireMainWindow::showHistory);

    // Input validation for nom, prenom, entreprise (max 10 characters)
    ui->nomLineEdit->setMaxLength(10);
    ui->prenomLineEdit->setMaxLength(10);
    ui->entrepriseLineEdit->setMaxLength(10);
    ui->telephoneLineEdit->setMaxLength(8);

    connect(ui->emailLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@(gmail\.com|yahoo\.fr|outlook\.com)$)");
        bool isValid = regex.match(text).hasMatch();

        if (!text.isEmpty() && !isValid) {
            ui->emailLineEdit->setStyleSheet("color: red;");
            ui->ajouterButton->setEnabled(false); // Désactiver le bouton
        } else {
            ui->emailLineEdit->setStyleSheet("color: green;");
            ui->ajouterButton->setEnabled(true); // Activer le bouton si email valide
        }
    });

    // Dans le constructeur PartenaireMainWindow, connectez le nouveau bouton
    connect(ui->emailButton, &QPushButton::clicked, this, &PartenaireMainWindow::on_emailButton_clicked);
    connect(ui->telephoneLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        // 1. Vérifier si le texte contient des caractères non numériques
        if (text.contains(QRegularExpression("[^0-9]"))) {
            QMessageBox::warning(this, "Erreur", "Seuls les chiffres sont autorisés !");

            // Supprimer les caractères non numériques
            QString cleanedText = text;
            cleanedText.remove(QRegularExpression("[^0-9]"));
            ui->telephoneLineEdit->setText(cleanedText);
            return; // On sort pour éviter les vérifications inutiles
        }

        // 2. Vérifier que le premier chiffre est valide (Tunisie)
        if (!text.isEmpty() && !QRegularExpression("^[2-57-9]").match(text).hasMatch()) {
            QMessageBox::warning(this, "Erreur", "Le numéro doit commencer par 2, 3, 4, 5, 7 ou 9 (Tunisie) !");

            // Supprimer le premier caractère invalide
            QString correctedText = text.mid(1); // Garde tout sauf le 1er caractère
            ui->telephoneLineEdit->setText(correctedText);
        }
    });



    // Prevent numeric input in nom and prenom fields
    // Pour le champ Nom
    connect(ui->nomLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString modifiedText = text;
        // Regex: n'autorise que les lettres (sans accents)
        if (modifiedText.contains(QRegularExpression("[^a-zA-Z]"))) {
            QMessageBox::warning(this, "Erreur", "Le champ Nom ne peut contenir que des lettres (A-Z, a-z) !");
            modifiedText.remove(QRegularExpression("[^a-zA-Z]"));
            ui->nomLineEdit->setText(modifiedText);
        }
    });

    // Pour le champ Prénom (identique)
    connect(ui->prenomLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString modifiedText = text;
        if (modifiedText.contains(QRegularExpression("[^a-zA-Z]"))) {
            QMessageBox::warning(this, "Erreur", "Le champ Prénom ne peut contenir que des lettres (A-Z, a-z) !");
            modifiedText.remove(QRegularExpression("[^a-zA-Z]"));
            ui->prenomLineEdit->setText(modifiedText);
        }
    });
    connect(ui->entrepriseLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString modifiedText = text;
        if (modifiedText.contains(QRegularExpression("[^a-zA-Z]"))) {
            QMessageBox::warning(this, "Erreur", "Le champ Prénom ne peut contenir que des lettres (A-Z, a-z) !");
            modifiedText.remove(QRegularExpression("[^a-zA-Z]"));
            ui->entrepriseLineEdit->setText(modifiedText);
        }
    });
}

void PartenaireMainWindow::on_pushButtonEnregistrer_clicked() {
    enregistrerEmpreinteDansArduino();
}

void PartenaireMainWindow::on_pushButtonIdentifier_clicked() {
    if (envoyerCommandeArduino("2")) {
        ui->textEditLogs->append("Commande d'identification envoyée à l'Arduino.");
    }
}

bool PartenaireMainWindow::envoyerCommandeArduino(const QString &commande) {
    if (!arduino2->getserial() || !arduino2->getserial()->isOpen()) {
        QMessageBox::critical(this, "Erreur", "Port série non connecté !");
        return false;
    }

    return arduino2->write_to_arduino((commande + "\n").toUtf8()) == 0;
}

void PartenaireMainWindow::afficherPartenaireParEmpreinte(int id) {
    QSqlQuery query;
    query.prepare("SELECT * FROM PARTENAIRE WHERE ID_FINGERPRINT = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger les données du partenaire");
        return;
    }

    // Affichage dans le tableau
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(1);

    for (int col = 0; col < 10; ++col) {
        QString value = query.value(col).toString();
        if (col == 8) { // Colonne Date
            QDateTime date = query.value(8).toDateTime();
            value = date.date().toString("yyyy-MM-dd");
        }
        ui->tableWidget->setItem(0, col, new QTableWidgetItem(value));
    }

    // Mettre à jour les statistiques
    setupStatistiques();

    // Ouvrir la porte
    envoyerCommandeArduino("3");
}

void PartenaireMainWindow::enregistrerEmpreinteDansArduino() {
    int row = ui->tableWidget->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un partenaire !");
        return;
    }

    // Récupération de l'ID du partenaire sélectionné
    int idPartenaire = ui->tableWidget->item(row, 0)->text().toInt();
    QString nom = ui->tableWidget->item(row, 1)->text();

    // On suppose que l'empreinte sera enregistrée sous le même ID
    int idEmpreinte = idPartenaire;

    if (envoyerCommandeArduino("1")) {
        envoyerCommandeArduino(QString::number(idEmpreinte));

        // Mise à jour du champ ID_FINGERPRINT dans la base de données
        QSqlQuery update;
        update.prepare("UPDATE PARTENAIRE SET ID_FINGERPRINT = :finger WHERE ID_PARTENAIRE = :id");
        update.bindValue(":finger", idEmpreinte);
        update.bindValue(":id", idPartenaire);

        if (!update.exec()) {
            QMessageBox::critical(this, "Erreur SQL", "Impossible de mettre à jour l'empreinte dans la base.");
            qDebug() << "Erreur SQL : " << update.lastError().text();
        } else {
            QMessageBox::information(this, "Enregistrement",
                                     "Empreinte associée à " + nom + ". Suivez les instructions sur le module.");
        }
    }
}


void PartenaireMainWindow::readSerialData() {

    static QString buffer; // Buffer pour accumuler les données
    QByteArray data = arduino2->read_from_arduino(); // Lire les données de l'Arduino

    if (data.isEmpty()) {
        return; // Si aucune donnée, on quitte
    }

    buffer += QString::fromUtf8(data); // Ajouter les nouvelles données au buffer

    // Traiter ligne par ligne tant qu'on a des \n
    while (buffer.contains('\n')) {
        int index = buffer.indexOf('\n');
        QString line = buffer.left(index).trimmed(); // Extraire une ligne complète
        buffer.remove(0, index + 1); // Supprimer la ligne traitée du buffer

        qDebug() << "Arduino >>" << line;
        ui->textEditLogs->append("Arduino >> " + line);

        if (line == "OK") {
            // Enregistrement empreinte terminé
            int row = ui->tableWidget->currentRow();
            if (row != -1) {
                int idPartenaire = ui->tableWidget->item(row, 0)->text().toInt();
                int idEmpreinte = idPartenaire;

                QSqlQuery update;
                update.prepare("UPDATE PARTENAIRE SET ID_FINGERPRINT = :finger WHERE ID_PARTENAIRE = :id");
                update.bindValue(":finger", idEmpreinte);
                update.bindValue(":id", idPartenaire);

                if (!update.exec()) {
                    QMessageBox::critical(this, "Erreur SQL", "Échec mise à jour de l'empreinte !");
                    qDebug() << update.lastError().text();
                } else {
                    QMessageBox::information(this, "Succès", "Empreinte enregistrée et liée au partenaire.");
                }
            }
        }
        else if (line.startsWith("Erreur")) {
            QMessageBox::critical(this, "Erreur Arduino", line);
        }
        else if (line.startsWith("ID=")) {
            bool ok;
            int id = line.mid(3).toInt(&ok); // Extraire l'ID après "ID="
            if (!ok || id <= 0) {
                QMessageBox::warning(this, "Erreur", "ID d'empreinte invalide reçu !");
                continue;
            }

            afficherPartenaireParEmpreinte(id);

            QSqlQuery query;
            query.prepare("SELECT PRENOM, NOM FROM PARTENAIRE WHERE ID_FINGERPRINT = :id");
            query.bindValue(":id", id);

            if (query.exec() && query.next()) {
                QString prenom = query.value(0).toString();
                QString nom = query.value(1).toString();

                QString message = "NOM=" + nom + ";PRENOM=" + prenom + "\n";
                arduino2->write_to_arduino(message.toUtf8()); // Utilisez write_to_arduino() au lieu de write()

                QMessageBox::information(this, "Accès autorisé",
                                         QString("Bienvenue %1 %2\nAccès autorisé").arg(prenom, nom));
            } else {
                QMessageBox::warning(this, "Non reconnu", "Aucun partenaire lié à cette empreinte.");
            }
        }
    }
}

void PartenaireMainWindow::generateEmailWithOpenAI(const QString &nom, const QString &prenom, const QString &type, const QString &objet, QTextEdit *bodyEdit) {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QByteArray apiKey = qgetenv("OPENAI_API_KEY");
    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, "Configuration manquante", "OPENAI_API_KEY non configuree.");
        return;
    }
    request.setRawHeader("Authorization", "Bearer " + apiKey);

    // 👉 Le prompt IA tient compte de l'objet du mail
    QString prompt = QString(
                         "Rédige un email professionnel pour %1 %2, un partenaire de type '%3', ayant pour objet : \"%4\".\n"
                         "Le message doit être courtois, concis, et correspondre à ce contexte."
                         ).arg(prenom, nom, type, objet);

    QJsonArray messages;
    messages.append(QJsonObject({{"role", "system"}, {"content", "Tu es un assistant intelligent qui écrit des emails professionnels."}}));
    messages.append(QJsonObject({{"role", "user"}, {"content", prompt}}));

    QJsonObject body;
    body["model"] = "gpt-3.5-turbo";
    body["messages"] = messages;

    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "Erreur IA", "Échec de la génération : " + reply->errorString());
            reply->deleteLater();
            return;
        }

        QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
        QString generatedText = responseDoc["choices"].toArray()[0].toObject()["message"].toObject()["content"].toString();
        bodyEdit->setPlainText(generatedText);
        reply->deleteLater();
    });
}


QString PartenaireMainWindow::generateSmartEmail(const QString &nom, const QString &prenom, const QString &type) {
    QString body = QString(
                       "Bonjour %1 %2,\n\n"
                       "Merci pour votre engagement en tant que partenaire de type '%3'. Nous sommes heureux de vous compter parmi nous.\n\n"
                       "N'hésitez pas à nous contacter pour toute question ou suggestion.\n\n"
                       "Cordialement,\n"
                       "L'équipe ArchiManage."
                       ).arg(prenom, nom, type);

    return body;
}



void PartenaireMainWindow::on_emailButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un partenaire !");
        return;
    }

    QString recipientEmail = ui->tableWidget->item(row, 3)->text(); // Colonne 3 = email
    QString nom = ui->tableWidget->item(row, 1)->text(); // Colonne 1 = nom
    QString prenom = ui->tableWidget->item(row, 2)->text(); // Colonne 2 = prénom

    QDialog *emailDialog = new QDialog(this);
    emailDialog->setWindowTitle("Envoyer un Email à " + nom + " " + prenom);
    emailDialog->resize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(emailDialog);

    // Champ pour l'email du destinataire (désactivé car pré-rempli)
    QLabel *recipientLabel = new QLabel("Destinataire:", emailDialog);
    QLineEdit *recipientEdit = new QLineEdit(recipientEmail, emailDialog);
    recipientEdit->setReadOnly(true);

    // Champ pour l'objet
    QLabel *subjectLabel = new QLabel("Objet:", emailDialog);
    QLineEdit *subjectEdit = new QLineEdit(emailDialog);
    subjectEdit->setPlaceholderText("Objet de l'email");

    // Champ pour le corps du message
    QLabel *bodyLabel = new QLabel("Message:", emailDialog);
    QTextEdit *bodyEdit = new QTextEdit(emailDialog);
    bodyEdit->setAcceptRichText(false);

    // Template de message avec salutation personnalisée
    QString defaultBody = QString("Bonjour %1 %2,\n\n").arg(prenom, nom);
    bodyEdit->setPlainText(defaultBody);

    // Boutons
    QPushButton *aiGenerateButton = new QPushButton("Générer avec IA", emailDialog);


    QPushButton *sendButton = new QPushButton("Envoyer", emailDialog);
    QPushButton *cancelButton = new QPushButton("Annuler", emailDialog);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(sendButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(aiGenerateButton);


    // Ajout des widgets au layout
    layout->addWidget(recipientLabel);
    layout->addWidget(recipientEdit);
    layout->addWidget(subjectLabel);
    layout->addWidget(subjectEdit);
    layout->addWidget(bodyLabel);
    layout->addWidget(bodyEdit);
    layout->addLayout(buttonLayout);

    // Connexion des signaux
    connect(aiGenerateButton, &QPushButton::clicked, [=]() {
        QString type = ui->tableWidget->item(row, 6)->text(); // colonne 6 = type
        QString objet = subjectEdit->text(); // l'objet saisi par l'utilisateur
        generateEmailWithOpenAI(nom, prenom, type, objet, bodyEdit);
    });


    connect(cancelButton, &QPushButton::clicked, emailDialog, &QDialog::reject);
    connect(sendButton, &QPushButton::clicked, [=]() {
        QString subject = subjectEdit->text().trimmed();
        QString body = bodyEdit->toPlainText().trimmed();

        // Validation des champs
        if (subject.isEmpty() || body.isEmpty()) {
            QMessageBox::warning(emailDialog, "Erreur", "L'objet et le message sont obligatoires !");
            return;
        }

        // Envoi de l'email
        if (sendAutomaticMail(recipientEmail, subject, body)) {
            QMessageBox::information(emailDialog, "Succès", "Email envoyé avec succès à " + nom + " " + prenom + " !");

            // Log de l'action
            int partenaireId = ui->tableWidget->item(row, 0)->text().toInt();
            logAction("EMAIL", partenaireId,
                      QString("Email envoyé à %1 %2").arg(nom, prenom),
                      QVariantMap(),
                      QVariantMap{{"Objet", subject}, {"Message", body.left(100) + "..."}});

            emailDialog->accept();
        } else {
            QMessageBox::critical(emailDialog, "Erreur", "Échec de l'envoi de l'email !");
        }
    });

    emailDialog->exec();
}
bool PartenaireMainWindow::sendAutomaticMail(const QString &recipient, const QString &subject, const QString &body) {
    QString from = QString::fromUtf8(qgetenv("SMTP_FROM"));
    QString username = QString::fromUtf8(qgetenv("SMTP_USERNAME"));
    QString password = QString::fromUtf8(qgetenv("SMTP_PASSWORD"));
    if (from.isEmpty()) {
        from = username;
    }
    if (from.isEmpty() || username.isEmpty() || password.isEmpty()) {
        qDebug() << "SMTP configuration is missing (SMTP_FROM/SMTP_USERNAME/SMTP_PASSWORD).";
        return false;
    }

    QSslSocket socket;
    socket.connectToHostEncrypted("smtp.gmail.com", 465);

    if (!socket.waitForConnected(5000)) {
        qDebug() << "Failed to connect to SMTP server:" << socket.errorString();
        return false;
    }

    if (!socket.waitForReadyRead(5000)) {
        qDebug() << "Failed to receive greeting message from SMTP server";
        return false;
    }

    QString response = socket.readAll();
    qDebug() << "SMTP Response:" << response;

    // HELO command
    if (!sendSmtpCommand(socket, "HELO smtp.gmail.com\r\n", "250")) {
        return false;
    }

    // AUTH LOGIN
    if (!sendSmtpCommand(socket, "AUTH LOGIN\r\n", "334")) {
        return false;
    }

    // Username
    QString base64Username = QByteArray(username.toUtf8()).toBase64();
    if (!sendSmtpCommand(socket, base64Username + "\r\n", "334")) {
        return false;
    }

    // Password
    QString base64Password = QByteArray(password.toUtf8()).toBase64();
    if (!sendSmtpCommand(socket, base64Password + "\r\n", "235")) {
        return false;
    }

    // MAIL FROM
    QString mailFromCommand = QString("MAIL FROM: <%1>\r\n").arg(from);
    if (!sendSmtpCommand(socket, mailFromCommand, "250")) {
        return false;
    }

    // RCPT TO
    QString rcptToCommand = QString("RCPT TO: <%1>\r\n").arg(recipient);
    if (!sendSmtpCommand(socket, rcptToCommand, "250")) {
        return false;
    }

    // DATA
    if (!sendSmtpCommand(socket, "DATA\r\n", "354")) {
        return false;
    }

    // Message
    QString message = QString("From: %1\r\nTo: %2\r\nSubject: %3\r\n\r\n%4\r\n.\r\n")
                          .arg(from, recipient, subject, body);
    if (!sendSmtpCommand(socket, message, "250")) {
        return false;
    }

    // QUIT
    sendSmtpCommand(socket, "QUIT\r\n", "221");
    bool success = (socket.state() == QAbstractSocket::ConnectedState);
    socket.close();

    if (success) {
        qDebug() << "Email sent successfully to" << recipient;
    } else {
        qDebug() << "Failed to send email to" << recipient;
    }

    return success;

    socket.close();
    qDebug() << "Email sent successfully to" << recipient;
    return true;
}

bool PartenaireMainWindow::sendSmtpCommand(QSslSocket &socket, const QString &command, const QString &expectedResponse) {
    socket.write(command.toUtf8());
    if (!socket.waitForBytesWritten(5000)) {
        qDebug() << "Failed to send command:" << command.trimmed();
        return false;
    }

    if (!socket.waitForReadyRead(5000)) {
        qDebug() << "No response to command:" << command.trimmed();
        return false;
    }

    QString response = socket.readAll();
    qDebug() << "Command:" << command.trimmed() << "| Response:" << response.trimmed();

    if (!response.startsWith(expectedResponse)) {
        qDebug() << "Unexpected response to command:" << command.trimmed();
        return false;
    }

    return true;
}

void PartenaireMainWindow::logAction(const QString &actionType, int partenaireId, const QString &details, const QVariantMap &oldValues, const QVariantMap &newValues)
{
    // Convertir les QVariant en strings pour uniformiser
    QVariantMap normalizedOldValues;
    QVariantMap normalizedNewValues;

    for (auto it = oldValues.begin(); it != oldValues.end(); ++it) {
        normalizedOldValues[it.key()] = it.value().toString();
    }

    for (auto it = newValues.begin(); it != newValues.end(); ++it) {
        normalizedNewValues[it.key()] = it.value().toString();
    }

    QSqlQuery query;
    query.prepare("INSERT INTO PARTENAIRE_HISTORY (ACTION_TYPE, PARTENAIRE_ID, DETAILS, OLD_VALUES, NEW_VALUES, USER_ID) "
                  "VALUES (:actionType, :partenaireId, :details, :oldValues, :newValues, :userId)");

    query.bindValue(":actionType", actionType);
    query.bindValue(":partenaireId", partenaireId);
    query.bindValue(":details", details);
    query.bindValue(":oldValues", QJsonDocument::fromVariant(normalizedOldValues).toJson(QJsonDocument::Compact));
    query.bindValue(":newValues", QJsonDocument::fromVariant(normalizedNewValues).toJson(QJsonDocument::Compact));
    query.bindValue(":userId", 1);

    if (!query.exec()) {
        qDebug() << "Erreur logAction:" << query.lastError().text();
        qDebug() << "Requête:" << query.lastQuery();
    }
}


void PartenaireMainWindow::showHistory()
{
    QDialog *historyDialog = new QDialog(this);
    historyDialog->setWindowTitle("Historique des Actions");
    historyDialog->resize(1200, 600);  // Augmenter la largeur pour plus d'espace

    QVBoxLayout *layout = new QVBoxLayout(historyDialog);

    // Ajouter un bouton de fermeture
    QPushButton *closeButton = new QPushButton("Fermer", historyDialog);
    connect(closeButton, &QPushButton::clicked, historyDialog, &QDialog::close);

    QTableWidget *historyTable = new QTableWidget(historyDialog);
    historyTable->setColumnCount(6);
    historyTable->setHorizontalHeaderLabels({"ID", "Type", "Partenaire", "Date", "Détails", "Modifications"});
    historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    historyTable->setColumnWidth(5, 400);  // Plus large pour les modifications
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Requête pour récupérer l'historique
    QSqlQuery query;
    query.prepare("SELECT h.ID, h.ACTION_TYPE, h.PARTENAIRE_ID, h.ACTION_DATE, h.DETAILS, "
                  "h.OLD_VALUES, h.NEW_VALUES, p.NOM, p.PRENOM "
                  "FROM PARTENAIRE_HISTORY h "
                  "LEFT JOIN PARTENAIRE p ON h.PARTENAIRE_ID = p.ID_PARTENAIRE "
                  "ORDER BY h.ACTION_DATE DESC");

    if (!query.exec()) {
        qDebug() << "Erreur lors de la récupération de l'historique:" << query.lastError().text();
        QMessageBox::critical(this, "Erreur", "Impossible de charger l'historique");
        return;
    }

    int row = 0;
    while (query.next()) {
        historyTable->insertRow(row);

        QString actionType = query.value(1).toString();
        QString actionText;

        if (actionType == "ADD") actionText = "Ajout";
        else if (actionType == "UPDATE") actionText = "Modification";
        else actionText = "Suppression";

        QString partenaireInfo = QString("%1 (%2 %3)")
                                     .arg(query.value(2).toString())
                                     .arg(query.value(7).toString())
                                     .arg(query.value(8).toString());

        // Traitement des modifications - version simplifiée
        QString modifications;
        if (actionType == "UPDATE") {
            QJsonParseError parseError;
            QJsonDocument oldDoc = QJsonDocument::fromJson(query.value(5).toByteArray(), &parseError);
            QJsonDocument newDoc = QJsonDocument::fromJson(query.value(6).toByteArray(), &parseError);

            QVariantMap oldMap = oldDoc.toVariant().toMap();
            QVariantMap newMap = newDoc.toVariant().toMap();

            QStringList changes;
            QStringList fields = {"NOM", "PRENOM", "EMAIL", "TELEPHONE", "ADRESSE", "TYPE", "ENTREPRISE", "DATE_AJOUT", "IDE"};

            for (const QString &field : fields) {
                QString oldVal = oldMap.value(field).toString().trimmed();
                QString newVal = newMap.value(field).toString().trimmed();

                if (field == "IDE") {
                    oldVal = QString::number(oldMap.value(field).toDouble());
                    newVal = QString::number(newMap.value(field).toDouble());
                }

                if (oldVal != newVal) {
                    changes << QString("%1: %2 → %3").arg(field).arg(oldVal.isEmpty() ? "N/A" : oldVal).arg(newVal.isEmpty() ? "N/A" : newVal);
                }
            }
            modifications = changes.join("\n");
        } else if (actionType == "ADD") {
            modifications = "Nouveau partenaire créé";
        } else if (actionType == "DELETE") {
            modifications = "Partenaire supprimé";
        }

        // Remplissage des colonnes
        historyTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        historyTable->setItem(row, 1, new QTableWidgetItem(actionText));
        historyTable->setItem(row, 2, new QTableWidgetItem(partenaireInfo));
        historyTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toDateTime().toString("dd/MM/yyyy HH:mm")));
        historyTable->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        historyTable->setItem(row, 5, new QTableWidgetItem(modifications));

        row++;
    }

    // Ajustement automatique des colonnes
    historyTable->resizeColumnsToContents();
    historyTable->horizontalHeader()->setStretchLastSection(true);

    // Ajout des éléments au layout
    layout->addWidget(historyTable);
    layout->addWidget(closeButton, 0, Qt::AlignRight);

    historyDialog->exec();
}

void PartenaireMainWindow::on_resetButton_clicked()
{
    afficherTable(); // Call the function that loads the full table
    ui->searchLineEdit->clear(); // Clear the search input
    ui->sortComboBox->setCurrentIndex(0); // Reset the combo box to its default value
}
void PartenaireMainWindow::on_searchButton_clicked()
{
    QString criteria = ui->sortComboBox->currentText();
    QString searchTerm = ui->searchLineEdit->text();

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données est fermée !");
        return;
    }

    if (searchTerm.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer un terme de recherche !");
        return;
    }

    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    if (criteria == "ID") {
        query.prepare("SELECT * FROM PARTENAIRE WHERE TO_CHAR(ID_PARTENAIRE) LIKE :searchTerm ORDER BY ID_PARTENAIRE");
    } else if (criteria == "Date") {
        query.prepare("SELECT * FROM PARTENAIRE WHERE TO_CHAR(DATE_AJOUT, 'YYYY-MM-DD') LIKE :searchTerm ORDER BY DATE_AJOUT");
    } else if (criteria == "Name") {
        query.prepare("SELECT * FROM PARTENAIRE WHERE NOM LIKE :searchTerm ORDER BY NOM");
    } else {
        QMessageBox::warning(this, "Erreur", "Critère de tri invalide !");
        return;
    }

    query.bindValue(":searchTerm", "%" + searchTerm + "%");

    if (!query.exec()) {
        qDebug() << "Query failed:" << query.lastQuery();
        qDebug() << "Error:" << query.lastError().text();
        QMessageBox::critical(this, "Erreur", "Échec de l'exécution de la requête : " + query.lastError().text());
        return;
    }

    model->setQuery(query);
    ui->tableWidget->setRowCount(model->rowCount());
    ui->tableWidget->setColumnCount(model->columnCount());
    QStringList headers = {"ID", "Nom", "Prénom", "Email", "Téléphone", "Adresse", "Type", "Entreprise", "Date Ajout", "IDE"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Activer le tri des colonnes
    ui->tableWidget->setSortingEnabled(true);

    for (int i = 0; i < model->rowCount(); ++i) {
        for (int j = 0; j < model->columnCount(); ++j) {
            if (j == 8) { // Date Ajout column
                QDateTime dateTime = model->data(model->index(i, j)).toDateTime();
                QString dateOnly = dateTime.date().toString("yyyy-MM-dd");
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(dateOnly));
            } else {
                QString value = model->data(model->index(i, j)).toString();
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(value));
            }
        }
    }
}


void PartenaireMainWindow::on_exportPdfButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export PDF", "", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) {
        return;
    }

    if (!fileName.endsWith(".pdf")) {
        fileName += ".pdf";
    }

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setOutputFileName(fileName);

    QTextDocument doc;

    // CSS styling
    QString style = "<style>"
                    "body { font-family: Arial; font-size: 10pt; }"
                    "h1 { color: #FFAE9D; text-align: center; }"
                    "table { width: 100%; border-collapse: collapse; margin-top: 20px; }"
                    "th { background-color: #FFAE9D; color: white; padding: 8px; text-align: left; }"
                    "td { padding: 6px; border: 1px solid #ddd; }"
                    "tr:nth-child(even) { background-color: #f9f9f9; }"
                    "</style>";

    QString html = "<html><head>" + style + "</head><body>";
    html += "<h1>Liste des Partenaires</h1>";
    html += "<table>";

    // Headers
    html += "<tr>";
    for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
        html += "<th>" + ui->tableWidget->horizontalHeaderItem(col)->text() + "</th>";
    }
    html += "</tr>";

    // Data
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        html += "<tr>";
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            QTableWidgetItem* item = ui->tableWidget->item(row, col);
            html += "<td>" + (item ? item->text() : "") + "</td>";
        }
        html += "</tr>";
    }

    html += "</table></body></html>";

    doc.setHtml(html);

    // Print with multi-page support
    doc.print(&printer);

    QMessageBox::information(this, "Success", "Data exported to PDF successfully!");
}


// Export data to Excel
bool PartenaireMainWindow::exportToExcel(const QString &fileName)
{
    QAxObject* excel = new QAxObject("Excel.Application");
    if (!excel) {
        QMessageBox::critical(this, "Error", "Could not start Excel application.");
        return false;
    }

    excel->setProperty("Visible", false);
    QAxObject* workbooks = excel->querySubObject("Workbooks");
    QAxObject* workbook = workbooks->querySubObject("Add");
    QAxObject* sheets = workbook->querySubObject("Worksheets");
    QAxObject* sheet = sheets->querySubObject("Item(int)", 1);

    // Get column headers
    QStringList headers;
    for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
        headers << ui->tableWidget->horizontalHeaderItem(col)->text();
    }

    // Write headers
    for (int col = 0; col < headers.size(); ++col) {
        QAxObject* cell = sheet->querySubObject("Cells(int,int)", 1, col + 1);
        cell->setProperty("Value", headers.at(col));
        delete cell;
    }

    // Write data
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            QTableWidgetItem* item = ui->tableWidget->item(row, col);
            if (item) {
                QAxObject* cell = sheet->querySubObject("Cells(int,int)", row + 2, col + 1);
                cell->setProperty("Value", item->text());
                delete cell;
            }
        }
    }

    // Auto-fit columns
    QAxObject* usedRange = sheet->querySubObject("UsedRange");
    QAxObject* columns = usedRange->querySubObject("Columns");
    columns->dynamicCall("AutoFit()");

    // Save and close
    workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(fileName));
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");

    delete sheet;
    delete sheets;
    delete workbook;
    delete workbooks;
    delete excel;

    return true;
}

// Import data from Excel
bool PartenaireMainWindow::importFromExcel(const QString &fileName)
{
    QAxObject* excel = new QAxObject("Excel.Application");
    if (!excel) {
        QMessageBox::critical(this, "Erreur", "Impossible de démarrer l'application Excel.");
        return false;
    }

    excel->setProperty("Visible", false);
    QAxObject* workbooks = excel->querySubObject("Workbooks");
    QAxObject* workbook = workbooks->querySubObject("Open(const QString&)", QDir::toNativeSeparators(fileName));
    QAxObject* sheets = workbook->querySubObject("Worksheets");
    QAxObject* sheet = sheets->querySubObject("Item(int)", 1);

    QAxObject* usedRange = sheet->querySubObject("UsedRange");
    QAxObject* rows = usedRange->querySubObject("Rows");
    QAxObject* columns = usedRange->querySubObject("Columns");

    int rowCount = rows->property("Count").toInt();
    int colCount = columns->property("Count").toInt();

    if (rowCount <= 1 || colCount == 0) {
        QMessageBox::warning(this, "Erreur", "Le fichier Excel est vide ou mal formaté.");
        workbook->dynamicCall("Close()");
        excel->dynamicCall("Quit()");
        delete usedRange;
        delete rows;
        delete columns;
        delete sheet;
        delete sheets;
        delete workbook;
        delete workbooks;
        delete excel;
        return false;
    }

    // Clear current table
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(rowCount - 1); // Assuming first row is headers
    ui->tableWidget->setColumnCount(colCount);

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction(); // Start a database transaction

    QSqlQuery query;
    query.prepare("INSERT INTO PARTENAIRE (ID_PARTENAIRE, NOM, PRENOM, EMAIL, TELEPHONE, "
                  "ADRESSE, TYPE, ENTREPRISE, DATE_AJOUT, IDE) "
                  "VALUES (partenaire_seq.NEXTVAL, :nom, :prenom, :email, :telephone, :adresse, :type, :entreprise, TO_DATE(:dateAjout, 'YYYY-MM-DD'), :ide)");
    // Read data
    for (int row = 2; row <= rowCount; ++row) { // Start from row 2 (skip headers)
        QStringList rowData;
        for (int col = 1; col <= colCount; ++col) {
            QAxObject* cell = sheet->querySubObject("Cells(int,int)", row, col);
            QVariant value = cell->property("Value");
            rowData << value.toString();
            delete cell;
        }

        // Validation des champs obligatoires
        bool ok;
        int idPartenaire = rowData[0].toInt(&ok);
        if (!ok || idPartenaire <= 0) {
            qDebug() << "Invalid ID_PARTENAIRE for row:" << row;
            continue; // Ignore cette ligne
        }

        QString nom = rowData[1];
        if (nom.isEmpty()) {
            qDebug() << "Nom manquant pour la ligne :" << row;
            continue; // Ignore cette ligne
        }

        QString prenom = rowData[2];
        QString email = rowData[3];
        QString telephone = rowData[4];
        QString adresse = rowData[5];
        QString type = rowData[6];
        QString entreprise = rowData[7];

        QString dateStr = rowData[8];
        QDateTime dateTime = QDateTime::fromString(dateStr, Qt::ISODate);
        QDate dateAjout = dateTime.isValid() ? dateTime.date() : QDate::currentDate();

        int ide = rowData[9].toInt(&ok);
        if (!ok) {
            qDebug() << "Invalid IDE for row:" << row;
            continue; // Ignore cette ligne
        }

        // Préparer la requête
        query.bindValue(":id_partenaire", idPartenaire);
        query.bindValue(":nom", nom);
        query.bindValue(":prenom", prenom);
        query.bindValue(":email", email);
        query.bindValue(":telephone", telephone);
        query.bindValue(":adresse", adresse);
        query.bindValue(":type", type);
        query.bindValue(":entreprise", entreprise);
        query.bindValue(":dateAjout", dateAjout.toString("yyyy-MM-dd"));
        query.bindValue(":ide", ide);

        if (!query.exec()) {
            qDebug() << "Erreur lors de l'insertion dans la base de données:" << query.lastError().text();
            db.rollback(); // Rollback transaction on error
            QMessageBox::critical(this, "Erreur", "Échec de l'importation des données dans la base de données.");
            workbook->dynamicCall("Close()");
            excel->dynamicCall("Quit()");
            delete usedRange;
            delete rows;
            delete columns;
            delete sheet;
            delete sheets;
            delete workbook;
            delete workbooks;
            delete excel;
            return false;
        }
    }
    db.commit(); // Commit transaction if all rows are inserted successfully

    // Close Excel
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");

    delete usedRange;
    delete rows;
    delete columns;
    delete sheet;
    delete sheets;
    delete workbook;
    delete workbooks;
    delete excel;

    QMessageBox::information(this, "Succès", "Données importées avec succès depuis Excel !");
    afficherTable(); // Refresh the table view
    return true;
}
void PartenaireMainWindow::on_exportExcelButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Excel", "", "Excel Files (*.xlsx)");
    if (fileName.isEmpty()) {
        return;
    }

    if (!fileName.endsWith(".xlsx")) {
        fileName += ".xlsx";
    }

    if (exportToExcel(fileName)) {
        QMessageBox::information(this, "Success", "Data exported to Excel successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Failed to export data to Excel!");
    }
}

void PartenaireMainWindow::on_importExcelButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import Excel", "", "Excel Files (*.xlsx *.xls)");
    if (fileName.isEmpty()) {
        return;
    }

    if (importFromExcel(fileName)) {
        QMessageBox::information(this, "Success", "Data imported from Excel successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Failed to import data from Excel!");
    }
}

void PartenaireMainWindow::setupStatistiques()
{

    // 1. Vérifier si un layout existe déjà
    if (ui->chartWidget->layout() != nullptr) {
        // 2. Supprimer proprement l'ancien layout et son contenu
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



    QSqlQuery query;
    query.prepare("SELECT type, COUNT(*) as count FROM partenaire GROUP BY type");

    if (!query.exec()) {
        qDebug() << "Erreur statistiques:" << query.lastError().text();
        return;
    }

    QPieSeries *series = new QPieSeries();
    int totalCount = 0;

    // Calcul du total
    while (query.next()) {
        totalCount += query.value(1).toInt();
    }

    // Ré-exécuter la requête pour créer les tranches
    query.first();
    query.previous(); // Reset pour parcourir à nouveau

    while (query.next()) {
        QString type = query.value(0).toString().trimmed();
        int count = query.value(1).toInt();
        double percentage = (count * 100.0) / totalCount;

        // Formatage : "Type (XX%)"
        QString label = QString("%1 (%2%)")
                            .arg(type.isEmpty() ? "Autre" : type)
                            .arg(percentage, 0, 'f', 1); // 1 décimale

        QPieSlice *slice = series->append(label, count);
        slice->setLabelVisible(true);
    }

    // Style du graphique
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Statistiques Des Partenaires Par Type");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Intégration dans l'UI
    QVBoxLayout *layout = new QVBoxLayout(ui->chartWidget);
    layout->addWidget(chartView);
    ui->chartWidget->setLayout(layout);
}



PartenaireMainWindow::~PartenaireMainWindow()
{

    delete ui;
}

void PartenaireMainWindow::on_ajouterButton_clicked()
{
    QString nom = ui->nomLineEdit->text();
    QString prenom = ui->prenomLineEdit->text();
    QString email = ui->emailLineEdit->text();
    QString telephone = ui->telephoneLineEdit->text();
    QString adresse = ui->adresseLineEdit->text();
    QString type = ui->typeComboBox->currentText();
    QString entreprise = ui->entrepriseLineEdit->text();

    QDate dateAjout = ui->dateAjoutDateEdit->date();

    // Vérification des champs obligatoires
    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires !");
        return;
    }

    bool ok;
    int employeId = ui->employeIdLineEdit->text().toInt(&ok);

    if (!ok || employeId <= 0) {
        QMessageBox::warning(this, "Erreur", "L'ID employé doit être un nombre entier positif !");
        return;
    }

    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires !");
        return;
    }

    QRegularExpression emailRegex(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "L'adresse email n'est pas valide !");
        return;
    }
    QRegularExpression telRegex(R"(^[234579]\d{7}$)"); // Starts with 2, 3, 4, 5, 7, or 9 and has exactly 8 digits
    if (!telRegex.match(telephone).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Le numéro de téléphone doit commencer par 2, 3, 4, 5, 7 ou 9 et contenir exactement 8 chiffres !\nCe numéro n'est pas un numéro tunisien.");
        return;
    }
    QRegularExpression nameRegex("^[A-Za-zÀ-ÖØ-öø-ÿ]+$"); // Lettres et accents uniquement

    if (nom.length() < 3 || nom.length() > 10 || !nameRegex.match(nom).hasMatch()) {
        if (nom.length() > 10) {
            QMessageBox::warning(this, "Erreur", "Le nom ne peut pas dépasser 10 caractères !");
        } else {
            QMessageBox::warning(this, "Erreur", "Le nom doit contenir entre 3 et 10 caractères et uniquement des lettres !");
        }
        return;
    }

    // Contrôle pour le prénom (minimum 3 caractères et maximum 10 caractères, uniquement des lettres)
    if (prenom.length() < 3 || prenom.length() > 10 || !nameRegex.match(prenom).hasMatch()) {
        if (prenom.length() > 10) {
            QMessageBox::warning(this, "Erreur", "Le prénom ne peut pas dépasser 10 caractères !");
        } else {
            QMessageBox::warning(this, "Erreur", "Le prénom doit contenir entre 3 et 10 caractères et uniquement des lettres !");
        }
        return;
    }

    // Contrôle pour l'entreprise (minimum 3 caractères et maximum 10 caractères, uniquement des lettres)
    if (entreprise.length() < 3 || entreprise.length() > 10 || !nameRegex.match(entreprise).hasMatch()) {
        if (entreprise.length() > 10) {
            QMessageBox::warning(this, "Erreur", "L'entreprise ne peut pas dépasser 10 caractères !");
        } else {
            QMessageBox::warning(this, "Erreur", "L'entreprise doit contenir entre 3 et 10 caractères et uniquement des lettres !");
        }
        return;
    }
    if (adresse.length() < 3) {
        QMessageBox::warning(this, "Erreur", "Le adresse doit contenir au moins 3 caractères !");
        return;
    }

    Partenaire p(0, nom, prenom, email, telephone, adresse, type, entreprise, dateAjout, employeId);

    // Avoid double message display by verifying the result of the operation
    if (p.ajouter()) {
        logAction("ADD", p.getLastInsertedId(),
                  QString("Ajout du partenaire %1 %2").arg(nom).arg(prenom));

        QMessageBox::information(this, "Succès", "Partenaire ajouté avec succès !");
        afficherTable();
        setupStatistiques();
        QString subject = "Confirmation de partenariat avec ArchiManage";
        QString body = QString("Bonjour %1 %2,\n\n"
                               "Nous confirmons avec plaisir votre inscription en tant que partenaire officiel de Archimanage.\n\n"
                               "**Détails :**\n"
                               "- Nom : %1 %2\n"
                               "- Entreprise : %3\n"
                               "- Type de partenariat : %4\n"
                               "- Date d'effet : %5\n\n"
                               "Pour toute question, contactez-nous à Abdlahmid.ellab@gmail.com.\n\n"
                               "Bien cordialement,\n"
                               "Ellab Abdelhamid").arg(nom, prenom, entreprise, type, QDate::currentDate().toString("dd/MM/yyyy"));
        sendAutomaticMail(email, subject, body);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout du partenaire !");
    }

    // Réinitialiser les champs du formulaire après l'ajout
    ui->nomLineEdit->clear();
    ui->prenomLineEdit->clear();
    ui->emailLineEdit->clear();
    ui->telephoneLineEdit->clear();
    ui->adresseLineEdit->clear();
    ui->typeComboBox->setCurrentIndex(0);
    ui->entrepriseLineEdit->clear();
    ui->dateAjoutDateEdit->setDate(QDate::currentDate());
    ui->employeIdLineEdit->clear();

}

void PartenaireMainWindow::afficherTable()
{
    Partenaire p;
    QSqlQueryModel *model = p.afficher();
    ui->tableWidget->setRowCount(model->rowCount());
    ui->tableWidget->setColumnCount(model->columnCount());
    QStringList headers = {"ID", "Nom", "Prénom", "Email", "Téléphone", "Adresse", "Type", "Entreprise", "Date Ajout","IDE" };
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    for (int i = 0; i < model->rowCount(); ++i) {
        for (int j = 0; j < model->columnCount(); ++j) {
            if (j == 8) { // Date Ajout column
                QDateTime dateTime = model->data(model->index(i, j)).toDateTime();
                QString dateOnly = dateTime.date().toString("yyyy-MM-dd");
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(dateOnly));
            } else {
                QString value = model->data(model->index(i, j)).toString();
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(value));
            }
        }
    }
}

void PartenaireMainWindow::on_modifierButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un partenaire à modifier !");
        return;
    }

    int id = ui->tableWidget->item(row, 0)->text().toInt();
    QString nom = ui->tableWidget->item(row, 1)->text();
    QString prenom = ui->tableWidget->item(row, 2)->text();
    QString email = ui->tableWidget->item(row, 3)->text();
    QString telephone = ui->tableWidget->item(row, 4)->text();
    QString adresse = ui->tableWidget->item(row, 5)->text();
    QString type = ui->tableWidget->item(row, 6)->text();
    QString entreprise = ui->tableWidget->item(row, 7)->text();
    QDate dateAjout = QDate::fromString(ui->tableWidget->item(row, 8)->text(), "yyyy-MM-dd");
    int employeId = ui->tableWidget->item(row, 9)->text().toInt();

    ui->nomLineEdit->setText(nom);
    ui->prenomLineEdit->setText(prenom);
    ui->emailLineEdit->setText(email);
    ui->telephoneLineEdit->setText(telephone);
    ui->adresseLineEdit->setText(adresse);
    ui->typeComboBox->setCurrentText(type);
    ui->entrepriseLineEdit->setText(entreprise);
    ui->dateAjoutDateEdit->setDate(dateAjout);
    ui->employeIdLineEdit->setText(QString::number(employeId));

    ui->ajouterButton->setText("Modifier");

    // Disconnect any existing connections to avoid duplicates
    QObject::disconnect(ui->ajouterButton, &QPushButton::clicked, nullptr, nullptr);

    // Connect the button to the modification slot
    connect(ui->ajouterButton, &QPushButton::clicked, this, [=]() {
        on_modifierButton_confirmed(id);

    });
}

void PartenaireMainWindow::on_modifierButton_confirmed(int id)
{
    // Récupération des nouvelles valeurs depuis l'interface
    QString nom = ui->nomLineEdit->text();
    QString prenom = ui->prenomLineEdit->text();
    QString email = ui->emailLineEdit->text();
    QString telephone = ui->telephoneLineEdit->text();
    QString adresse = ui->adresseLineEdit->text();
    QString type = ui->typeComboBox->currentText();
    QString entreprise = ui->entrepriseLineEdit->text();
    QDate dateAjout = ui->dateAjoutDateEdit->date();
    QString employeIdStr = ui->employeIdLineEdit->text();

    // Validation des champs obligatoires
    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires !");
        return;
    }

    // Validation de l'ID employé
    bool ok;
    int employeId = employeIdStr.toInt(&ok);
    if (!ok || employeId <= 0) {
        QMessageBox::warning(this, "Erreur", "L'ID employé doit être un nombre entier positif !");
        return;
    }

    // Validation de l'email
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "L'adresse email n'est pas valide !");
        return;
    }

    // Validation du téléphone
    QRegularExpression telRegex(R"(^[234579]\d{7}$)");
    if (!telRegex.match(telephone).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Le numéro doit commencer par 2,3,4,5,7 ou 9 et contenir 8 chiffres !");
        return;
    }

    // Validation des noms
    QRegularExpression nameRegex("^[A-Za-zÀ-ÖØ-öø-ÿ]+$");
    if (nom.length() < 3 || nom.length() > 10 || !nameRegex.match(nom).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Le nom doit contenir 3-10 lettres !");
        return;
    }

    if (prenom.length() < 3 || prenom.length() > 10 || !nameRegex.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Le prénom doit contenir 3-10 lettres !");
        return;
    }

    if (entreprise.length() < 3 || entreprise.length() > 10 || !nameRegex.match(entreprise).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "L'entreprise doit contenir 3-10 lettres !");
        return;
    }

    if (adresse.length() < 3) {
        QMessageBox::warning(this, "Erreur", "L'adresse doit contenir au moins 3 caractères !");
        return;
    }

    // Récupération des ANCIENNES valeurs depuis la base
    QSqlQuery oldDataQuery;
    oldDataQuery.prepare("SELECT NOM, PRENOM, EMAIL, TELEPHONE, ADRESSE, TYPE, ENTREPRISE, DATE_AJOUT, IDE "
                         "FROM PARTENAIRE WHERE ID_PARTENAIRE = :id");
    oldDataQuery.bindValue(":id", id);

    if (!oldDataQuery.exec()) {
        qDebug() << "Erreur récupération anciennes valeurs:" << oldDataQuery.lastError();
        return;
    }

    QVariantMap oldValues;
    if (oldDataQuery.next()) {
        oldValues["NOM"] = oldDataQuery.value("NOM");
        oldValues["PRENOM"] = oldDataQuery.value("PRENOM");
        oldValues["EMAIL"] = oldDataQuery.value("EMAIL");
        oldValues["TELEPHONE"] = oldDataQuery.value("TELEPHONE");
        oldValues["ADRESSE"] = oldDataQuery.value("ADRESSE");
        oldValues["TYPE"] = oldDataQuery.value("TYPE");
        oldValues["ENTREPRISE"] = oldDataQuery.value("ENTREPRISE");
        oldValues["DATE_AJOUT"] = oldDataQuery.value("DATE_AJOUT").toDate().toString("yyyy-MM-dd");
        oldValues["IDE"] = oldDataQuery.value("IDE");
    }

    // Création de l'objet Partenaire avec les nouvelles valeurs
    Partenaire p(id, nom, prenom, email, telephone, adresse, type, entreprise, dateAjout, employeId);

    // Enregistrement des NOUVELLES valeurs
    QVariantMap newValues;
    newValues["NOM"] = nom;
    newValues["PRENOM"] = prenom;
    newValues["EMAIL"] = email;
    newValues["TELEPHONE"] = telephone;
    newValues["ADRESSE"] = adresse;
    newValues["TYPE"] = type;
    newValues["ENTREPRISE"] = entreprise;
    newValues["DATE_AJOUT"] = dateAjout.toString("yyyy-MM-dd");
    newValues["IDE"] = QString::number(employeId);

    // Debug des valeurs
    qDebug() << "Anciennes valeurs:" << oldValues;
    qDebug() << "Nouvelles valeurs:" << newValues;

    // Modification dans la base
    if (p.modifier(id)) {
        // Log de l'action dans l'historique
        logAction("UPDATE", id, "Modification partenaire", oldValues, newValues);

        QMessageBox::information(this, "Succès", "Partenaire modifié avec succès !");
        afficherTable();
        setupStatistiques();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification !");
    }

    // Réinitialisation de l'interface
    ui->nomLineEdit->clear();
    ui->prenomLineEdit->clear();
    ui->emailLineEdit->clear();
    ui->telephoneLineEdit->clear();
    ui->adresseLineEdit->clear();
    ui->typeComboBox->setCurrentIndex(0);
    ui->entrepriseLineEdit->clear();
    ui->dateAjoutDateEdit->setDate(QDate::currentDate());
    ui->employeIdLineEdit->clear();

    // Réactivation du mode Ajout
    ui->ajouterButton->setText("Ajouter");
    disconnect(ui->ajouterButton, nullptr, nullptr, nullptr);
    connect(ui->ajouterButton, &QPushButton::clicked, this, &PartenaireMainWindow::on_ajouterButton_clicked);
}

void PartenaireMainWindow::on_supprimerButton_clicked()
{


    // Create a Partenaire object
    Partenaire p;
    int row = ui->tableWidget->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un partenaire à supprimer !");
        return;
    }

    int id = ui->tableWidget->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Confirmation", "Êtes-vous sûr de vouloir supprimer ce partenaire ?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;
    if (p.supprimer(id)) {
        logAction("DELETE", id,
                  QString("Suppression du partenaire ID %1").arg(id));

        afficherTable();
        setupStatistiques();
    }

    if (p.supprimer(id)) {
        QMessageBox::information(this, "Succès", "Partenaire supprimé avec succès !");
        afficherTable();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression du partenaire !");
    }
}
