#include <QMessageBox>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QScrollBar>
#include "login.h"
#include <QDesktopServices>
#include <QUrl>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QCryptographicHash>

#include "employemainwindow.h"
#include "ui_employemainwindow.h"
#include "employe.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QImage>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QRandomGenerator>

EmployeMainWindow::EmployeMainWindow(const employe& e, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::EmployeMainWindow),
    employeConnecte(e)
{
    ui->setupUi(this);

    // Debug : afficher les infos dans la console
    qDebug() << "ID:" << employeConnecte.getIde();
    qDebug() << "Nom:" << employeConnecte.getNom();
    qDebug() << "Prenom:" << employeConnecte.getPrenom();
    qDebug() << "Poste:" << employeConnecte.getPoste();

    ui->tabWidget->setCurrentIndex(0); // Focus sur l'onglet CRUD au démarrage



    // Dans MainWindow.cpp, ajoutez ce debug :
    qDebug() << "Nombre d'onglets :" << ui->tabWidget->count();
    qDebug() << "Widget onglet ChatBot :" << ui->tabWidget->widget(2);

    // Remplacez votre code actuel par :
    chatBotWidget = new ChatBotEmploye(this);

    // Vérification renforcée de l'onglet
    if(ui->tabWidget->count() > 2) {  // L'index 2 doit exister
        QWidget *chatTab = ui->tabWidget->widget(2);

        // Nettoyage complet
        QLayout *layout = chatTab->layout();
        if(layout) {
            QLayoutItem *item;
            while((item = layout->takeAt(0))) {
                if(item->widget()) {
                    item->widget()->setParent(nullptr);
                    delete item->widget();
                }
                delete item;
            }
            delete layout;
        }

        // Nouveau layout avec bordures visibles
        QVBoxLayout *newLayout = new QVBoxLayout(chatTab);
        newLayout->setContentsMargins(0, 0, 0, 0);
        newLayout->setSpacing(5);

        // Style de débogage (bordure rouge)
        chatBotWidget->setStyleSheet("border: 2px solid red;");
        newLayout->addWidget(chatBotWidget);

        qDebug() << "ChatBot intégré dans l'onglet";
    } else {
        qDebug() << "ERREUR: L'onglet ChatBot n'existe pas!";
    }


    // Initialisation des QChartView
    chartViewStatut = new QChartView(this);
    chartViewPoste = new QChartView(this);

    // Initialisation du timer de recherche
    searchTimer = new QTimer(this);
    searchTimer->setInterval(300);
    searchTimer->setSingleShot(true);

    // Configuration du layout horizontal
    ui->HLayoutStats->setContentsMargins(5, 5, 5, 5);  // Marges réduites
    ui->HLayoutStats->setSpacing(15);                  // Espace entre graphiques


    // Configuration de la table
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    afficherTable();

    // Configuration des champs
    ui->telephoneLineEdit->setInputMask("");
    ui->telephoneLineEdit->setMaxLength(8);
    ui->SalaireLineEdit->setInputMask("");
    ui->nomLineEdit->setInputMask("");
    ui->prenomLineEdit->setInputMask("");
    ui->emailLineEdit->setInputMask("");
    ui->adresseLineEdit->setInputMask("");

    // Configuration des ComboBox
    ui->posteComboBox->addItems({
        "Admin",
        "Res_RH",
        "Res_Design",
        "Res_Projet",
        "Res_Partenaires",
        "Res_Client",
        "Res_Équipements"
    });

    ui->statueComboBox->addItems({"active", "inactive"});

    // Configuration date
    ui->dateEmbaucheDateEdit->setDate(QDate::currentDate());
    ui->dateEmbaucheDateEdit->setMaximumDate(QDate::currentDate());
    ui->dateEmbaucheDateEdit->setCalendarPopup(true);

    // Validateurs
    QRegularExpressionValidator *nomValidator = new QRegularExpressionValidator(
        QRegularExpression("^[A-Za-zÀ-ÖØ-öø-ÿ\\s-]{3,10}$"), this);
    ui->nomLineEdit->setValidator(nomValidator);
    ui->nomLineEdit->setMaxLength(10);
    ui->prenomLineEdit->setValidator(nomValidator);
    ui->prenomLineEdit->setMaxLength(10);

    QRegularExpressionValidator *emailValidator = new QRegularExpressionValidator(
        QRegularExpression(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)"), this);
    ui->emailLineEdit->setValidator(emailValidator);

    QRegularExpressionValidator *salaireValidator = new QRegularExpressionValidator(
        QRegularExpression("^\\d{0,7}(\\.\\d{0,2})?$"), this);
    ui->SalaireLineEdit->setValidator(salaireValidator);

    QRegularExpressionValidator *telephoneValidator = new QRegularExpressionValidator(
        QRegularExpression("^[0-9]{0,8}$"), this);
    ui->telephoneLineEdit->setValidator(telephoneValidator);

    QRegularExpressionValidator *adresseValidator = new QRegularExpressionValidator(
        QRegularExpression("^.{3,10}$"), this);
    ui->adresseLineEdit->setValidator(adresseValidator);
    ui->adresseLineEdit->setMaxLength(10);

    QRegularExpressionValidator *passwordValidator = new QRegularExpressionValidator(
        QRegularExpression("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,}$"), this);
    ui->passwordLineEdit->setValidator(passwordValidator);
    ui->confirmPasswordLineEdit->setValidator(passwordValidator);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);



    // Configuration du tri
    ui->TriComboBox->addItem("ID", employe::ID);
    ui->TriComboBox->addItem("Nom", employe::Nom);
    ui->TriComboBox->addItem("Prénom", employe::Prenom);
    ui->TriComboBox->addItem("Salaire", employe::Salaire);
    ui->TriComboBox->addItem("Date d'embauche", employe::DateEmbauche);
    ui->croissantCheckBox->setChecked(true);

    // Configuration du filtrage
    ui->PComboBox->clear();
    ui->PComboBox->addItem("Tous");
    ui->PComboBox->addItems({
        "Admin",
        "Res_RH",
        "Res_Design",
        "Res_Projet",
        "Res_Partenaires",
        "Res_Client",
        "Res_Équipements"
    });

    ui->statutComboBox->clear();
    ui->statutComboBox->addItem("Tous");
    ui->statutComboBox->addItems({"active", "inactive"});

    ui->HLayoutStats->addWidget(chartViewStatut);
    ui->HLayoutStats->addWidget(chartViewPoste);

    // Connexions des signaux
    // Validation dynamique
    connect(ui->nomLineEdit, &QLineEdit::textChanged, this, &EmployeMainWindow::validerNom);
    connect(ui->prenomLineEdit, &QLineEdit::textChanged, this, &EmployeMainWindow::validerPrenom);
    connect(ui->emailLineEdit, &QLineEdit::textChanged, this, &EmployeMainWindow::validerEmail);
    connect(ui->telephoneLineEdit, &QLineEdit::textEdited, this, &EmployeMainWindow::on_telephoneLineEdit_textEdited);
    connect(ui->telephoneLineEdit, &QLineEdit::textChanged, this, &EmployeMainWindow::validerTelephone);
    connect(ui->adresseLineEdit, &QLineEdit::textChanged, this, &EmployeMainWindow::validerAdresse);
    connect(ui->SalaireLineEdit, &QLineEdit::textChanged, this, &EmployeMainWindow::validerSalaire);
    connect(ui->dateEmbaucheDateEdit, &QDateEdit::dateChanged, this, &EmployeMainWindow::validerDate);


    connect(ui->passwordLineEdit, &QLineEdit::textChanged, this, &EmployeMainWindow::validerPassword);
    connect(ui->confirmPasswordLineEdit, &QLineEdit::textChanged, this, &EmployeMainWindow::validerConfirmPassword);

    // Tri et filtrage
    connect(ui->TriComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EmployeMainWindow::on_TriComboBox_currentIndexChanged);
    connect(ui->croissantCheckBox, &QCheckBox::stateChanged,
            this, &EmployeMainWindow::on_croissantCheckBox_stateChanged);
    connect(ui->decroissantCheckBox, &QCheckBox::stateChanged,
            this, &EmployeMainWindow::on_decroissantCheckBox_stateChanged);
    connect(ui->filtrerButton, &QPushButton::clicked, this, &EmployeMainWindow::filtrerEmployes);

    // Recherche
    connect(ui->rechercheLineEdit, &QLineEdit::textChanged, this, [this]() {
        searchTimer->start();
    });
    connect(searchTimer, &QTimer::timeout, this, &EmployeMainWindow::effectuerRecherche);

    // Gestion des onglets
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &EmployeMainWindow::onTabChanged);





    // Style global pour les GroupBox
    QString groupBoxStyle = "QGroupBox {"
                            "border: 1px solid #ccc;"
                            "border-radius: 5px;"
                            "margin-top: 10px;"
                            "padding-top: 15px;"
                            "}";

    this->setStyleSheet(groupBoxStyle);

    // Politique de redimensionnement
    chartViewStatut->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartViewPoste->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Taille minimale recommandée
    chartViewStatut->setMinimumSize(300, 250);
    chartViewPoste->setMinimumSize(300, 250);
}

EmployeMainWindow::~EmployeMainWindow()
{
    delete ui;
}

// Validation dynamique du nom
void EmployeMainWindow::validerNom(const QString &text) {
    QRegularExpression regex("^[A-Za-zÀ-ÖØ-öø-ÿ\\s-]{3,10}$");
    if (text.isEmpty() || !regex.match(text).hasMatch()) {
        ui->nomLineEdit->setStyleSheet("border: 1px solid red;");
    } else {
        ui->nomLineEdit->setStyleSheet("border: 1px solid green;");
    }
}

// Identique pour validerPrenom()
// Validation dynamique du prénom
void EmployeMainWindow::validerPrenom(const QString &text) {
    QRegularExpression regex("^[A-Za-zÀ-ÖØ-öø-ÿ\\s-]{3,10}$");
    if (text.isEmpty() || !regex.match(text).hasMatch()) {
        ui->prenomLineEdit->setStyleSheet("border: 1px solid red;");
    } else {
        ui->prenomLineEdit->setStyleSheet("border: 1px solid green;");
    }
}

// Validation dynamique de l'email
void EmployeMainWindow::validerEmail(const QString &text) {
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    if (text.isEmpty() || !emailRegex.match(text).hasMatch()) {
        ui->emailLineEdit->setStyleSheet("border: 1px solid red;");
    } else {
        ui->emailLineEdit->setStyleSheet("border: 1px solid green;");
    }
}


//Validation dynamique du téléphone
void EmployeMainWindow::validerTelephone(const QString &text) {
    if (text.length() == 8) {  // Validation finale stricte
        ui->telephoneLineEdit->setStyleSheet("border: 1px solid green;");
    } else {
        ui->telephoneLineEdit->setStyleSheet("border: 1px solid red;");
    }
}

// Validation dynamique de l'adresse
void EmployeMainWindow::validerAdresse(const QString &text) {
    QRegularExpression regex("^.{3,10}$");
    if (text.isEmpty() || !regex.match(text).hasMatch()) {
        ui->adresseLineEdit->setStyleSheet("border: 1px solid red;");
    } else {
        ui->adresseLineEdit->setStyleSheet("border: 1px solid green;");
    }
}

// Validation dynamique du salaire
void EmployeMainWindow::validerSalaire(const QString &text) {
    bool ok;
    double salaireValue = text.toDouble(&ok);
    if (text.isEmpty() || !ok || salaireValue < 0) {
        ui->SalaireLineEdit->setStyleSheet("border: 1px solid red;");
    } else {
        ui->SalaireLineEdit->setStyleSheet("border: 1px solid green;");
    }
}


//validation dynamique de la date

void EmployeMainWindow::validerDate(const QDate &date) {
    if (date > QDate::currentDate()) {
        ui->dateEmbaucheDateEdit->setStyleSheet("border: 1px solid red;");
    } else {
        ui->dateEmbaucheDateEdit->setStyleSheet("border: 1px solid green;");
    }
}

// Méthode pour vérifier si tous les champs sont remplis
bool EmployeMainWindow::tousLesChampsSontRemplis() {
    return !ui->nomLineEdit->text().isEmpty() &&
           !ui->prenomLineEdit->text().isEmpty() &&
           !ui->emailLineEdit->text().isEmpty() &&
           !ui->adresseLineEdit->text().isEmpty() &&
           !ui->SalaireLineEdit->text().isEmpty() &&
           !ui->telephoneLineEdit->text().isEmpty();
}

void EmployeMainWindow::on_ajouterButton_clicked()
{
    // 1. Récupération des valeurs depuis l'interface
    QString nom = ui->nomLineEdit->text();
    QString prenom = ui->prenomLineEdit->text();
    QString email = ui->emailLineEdit->text();
    QString adresse = ui->adresseLineEdit->text();
    QString poste = ui->posteComboBox->currentText();
    QString salaire = ui->SalaireLineEdit->text();
    QString statue = ui->statueComboBox->currentText();
    QString telephone = ui->telephoneLineEdit->text();
    QDate date_dembauche = ui->dateEmbaucheDateEdit->date();

    QString question = ui->lineEdit_question->text();
    QString reponse = ui->lineEdit_reponse->text();

    // 2. Validation des champs
    if (!tousLesChampsSontRemplis()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs doivent être remplis !");
        return;
    }

    QStringList erreurs;
    QRegularExpression regexNomPrenom("^[A-Za-zÀ-ÖØ-öø-ÿ\\s-]+$");

    // Validation du nom
    if (nom.isEmpty() || !regexNomPrenom.match(nom).hasMatch() || nom.length() < 3) {
        erreurs << "Le nom doit contenir au moins 3 caractères valides";
    }

    // Validation du prénom
    if (prenom.isEmpty() || !regexNomPrenom.match(prenom).hasMatch() || prenom.length() < 3) {
        erreurs << "Le prénom doit contenir au moins 3 caractères valides";
    }

    // Validation de l'email
    QRegularExpression emailRegex("^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$");
    if (!emailRegex.match(email).hasMatch()) {
        erreurs << "Format d'email invalide";
    }

    // Validation du téléphone
    if (telephone.length() != 8 || !telephone.toInt()) {
        erreurs << "Le téléphone doit contenir 8 chiffres";
    }

    // Validation du salaire
    bool ok;
    salaire.toDouble(&ok);
    if (!ok) {
        erreurs << "Le salaire doit être un nombre valide";
    }

    if (!erreurs.isEmpty()) {
        QMessageBox::warning(this, "Erreur", erreurs.join("\n"));
        return;
    }

    // Vérification du password
    if (ui->passwordLineEdit->text().isEmpty() ||
        ui->confirmPasswordLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Les champs mot de passe doivent être remplis !");
        return;
    }

    if (!m_passwordMatch) {
        QMessageBox::warning(this, "Erreur", "Les mots de passe ne correspondent pas !");
        return;
    }

    // Hash simple du password (utilisez QCryptographicHash pour plus de sécurité)
    QString passwordHash = QString(QCryptographicHash::hash(
                                       ui->passwordLineEdit->text().toUtf8(), QCryptographicHash::Sha256).toHex());

    // 3. Création et ajout de l'employé
    try {
        employe nouvelEmploye(
            0,                // ID (0 pour auto-incrément)
            nom,
            prenom,
            email,
            adresse,
            poste,
            salaire,
            statue,
            telephone,
            date_dembauche,
            passwordHash,
            question,         // Question secrète
            reponse,
            lastImagePath
            );

        if (nouvelEmploye.ajouter()) {
            QMessageBox::information(this, "Succès", "Employé ajouté avec succès !");

            // 4. Réinitialisation UI
            ui->nomLineEdit->clear();
            ui->prenomLineEdit->clear();
            ui->emailLineEdit->clear();
            ui->adresseLineEdit->clear();
            ui->SalaireLineEdit->clear();
            ui->telephoneLineEdit->clear();
            ui->dateEmbaucheDateEdit->setDate(QDate::currentDate());
            ui->emailLineEdit->setStyleSheet("");
            ui->passwordLineEdit->clear();
            ui->confirmPasswordLineEdit->clear();
            ui->lineEdit_question->clear();
            ui->lineEdit_reponse->clear();

            // 5. Rafraîchissement avec tri conservé
            appliquerTri(true);
        } else {
            throw std::runtime_error(nouvelEmploye.getLastError().text().toStdString());
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Erreur", QString("Échec de l'ajout : %1").arg(e.what()));
    }
}

void EmployeMainWindow::afficherTable()
{
    // Vérifier qu'on est sur l'onglet CRUD (index 0)
    if (ui->tabWidget->currentIndex() != 0) {
        qDebug() << "Affichage table ignoré - pas sur l'onglet CRUD";
        return;
    }

    // Désactiver les mises à jour pour améliorer les performances
    ui->tableWidget->setUpdatesEnabled(false);
    QElapsedTimer timer;
    timer.start();

    try {
        employe e;
        QScopedPointer<QSqlQueryModel> model; // Gestion automatique mémoire

        // Appliquer le tri si sélectionné, sinon affichage standard
        if (ui->TriComboBox->currentIndex() >= 0) {
            employe::SortCriteria critere = static_cast<employe::SortCriteria>(
                ui->TriComboBox->currentData().toInt());
            Qt::SortOrder ordre = ui->croissantCheckBox->isChecked()
                                      ? Qt::AscendingOrder
                                      : Qt::DescendingOrder;
            model.reset(e.trier(critere, ordre));
        } else {
            model.reset(e.afficher());
        }

        if (!model) {
            throw std::runtime_error("Erreur: modèle non initialisé");
        }

        // Sauvegarder la sélection et position de défilement
        int currentRow = ui->tableWidget->currentRow();
        int scrollValue = ui->tableWidget->verticalScrollBar()->value();

        // Préparer la table
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setRowCount(model->rowCount());
        ui->tableWidget->setColumnCount(model->columnCount());

        // Définir les en-têtes
        QStringList headers = {"ID", "Nom", "Prénom", "Email", "Adresse",
                               "Poste", "Salaire", "Statut", "Téléphone", "Date d'embauche"};
        ui->tableWidget->setHorizontalHeaderLabels(headers);

        // Remplir les données
        for (int row = 0; row < model->rowCount(); ++row) {
            for (int col = 0; col < model->columnCount(); ++col) {
                QVariant data = model->data(model->index(row, col));
                QTableWidgetItem *item = new QTableWidgetItem();

                // Formatage spécial pour les dates
                if (col == 9) { // Colonne Date
                    QDate date;
                    if (data.type() == QVariant::Date) {
                        date = data.toDate();
                    } else {
                        date = QDate::fromString(data.toString(), Qt::ISODate);
                    }
                    item->setText(date.isValid() ? date.toString("dd/MM/yyyy") : "Date invalide");
                }
                // Formatage spécial pour les salaires
                else if (col == 6) { // Colonne Salaire
                    bool ok;
                    double salaire = data.toDouble(&ok);
                    item->setText(ok ? QString::number(salaire, 'f', 2) : "");
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                }
                else {
                    item->setText(data.toString());
                }

                item->setFlags(item->flags() ^ Qt::ItemIsEditable); // Rendre non éditable
                ui->tableWidget->setItem(row, col, item);
            }
        }

        // Restaurer la sélection et position de défilement
        if (currentRow >= 0 && currentRow < ui->tableWidget->rowCount()) {
            ui->tableWidget->selectRow(currentRow);
            ui->tableWidget->verticalScrollBar()->setValue(scrollValue);
        }

        // Optimiser l'affichage
        ui->tableWidget->resizeColumnsToContents();
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        ui->tableWidget->setAlternatingRowColors(true);

        qDebug() << "Table rafraîchie en" << timer.elapsed() << "ms -"
                 << model->rowCount() << "lignes affichées";

    } catch (const std::exception &e) {
        qCritical() << "Erreur dans afficherTable:" << e.what();
        QMessageBox::critical(this, "Erreur",
                              QString("Impossible d'afficher les données:\n%1").arg(e.what()));
    }

    // Réactiver les mises à jour
    ui->tableWidget->setUpdatesEnabled(true);
}
void EmployeMainWindow::on_modifierButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un employé à modifier !");
        return;
    }

    // Récupérer les données de la ligne sélectionnée
    int ide = ui->tableWidget->item(row, 0)->text().toInt();
    QString nom = ui->tableWidget->item(row, 1)->text();
    QString prenom = ui->tableWidget->item(row, 2)->text();
    QString email = ui->tableWidget->item(row, 3)->text();
    QString adresse = ui->tableWidget->item(row, 4)->text();
    QString poste = ui->tableWidget->item(row, 5)->text();
    QString salaire = ui->tableWidget->item(row, 6)->text();
    QString statue = ui->tableWidget->item(row, 7)->text();
    QString telephone = ui->tableWidget->item(row, 8)->text();
    QDate date_dembauche = QDate::fromString(ui->tableWidget->item(row, 9)->text(), "yyyy-MM-dd");

    // Remplir les champs du formulaire avec les données sélectionnées
    ui->nomLineEdit->setText(nom);
    ui->prenomLineEdit->setText(prenom);
    ui->emailLineEdit->setText(email);
    ui->adresseLineEdit->setText(adresse);
    ui->posteComboBox->setCurrentText(poste);
    ui->SalaireLineEdit->setText(salaire);
    ui->statueComboBox->setCurrentText(statue);
    ui->telephoneLineEdit->setText(telephone);
    ui->dateEmbaucheDateEdit->setDate(date_dembauche);

    // Changer le texte du bouton "Ajouter" en "Modifier"
    ui->ajouterButton->setText("Modifier");

    // Déconnecter toutes les connexions existantes du bouton "Ajouter"
    QObject::disconnect(ui->ajouterButton, nullptr, nullptr, nullptr);

    // Reconnecter le bouton "Ajouter" pour la modification
    connect(ui->ajouterButton, &QPushButton::clicked, this, [=]() {
        on_modifierButton_confirmed(ide);
    });

    // Désactiver les champs mot de passe et confirmation ici
    ui->passwordLineEdit->setDisabled(true);
    ui->confirmPasswordLineEdit->setDisabled(true);

    ui->lineEdit_question->setDisabled(true);
    ui->lineEdit_reponse->setDisabled(true);

}

void EmployeMainWindow::on_modifierButton_confirmed(int ide)
{
    if (!tousLesChampsSontRemplis()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs doivent être remplis !");
        return;
    }

    // Récupérer les données du formulaire
    QString nom = ui->nomLineEdit->text();
    QString prenom = ui->prenomLineEdit->text();
    QString email = ui->emailLineEdit->text();
    QString adresse = ui->adresseLineEdit->text();
    QString statue = ui->statueComboBox->currentText();
    QString salaire = ui->SalaireLineEdit->text();
    QString poste = ui->posteComboBox->currentText();
    QString telephone = ui->telephoneLineEdit->text();
    QDate date_dembauche = ui->dateEmbaucheDateEdit->date();

    // Ne pas récupérer ni modifier la question et la réponse
    QString question = "";  // Vous pouvez le laisser vide ou récupérer la valeur initiale si nécessaire
    QString reponse = "";   // Pareil ici, laissez vide ou récupérez l'ancienne valeur si nécessaire

    QStringList erreurs;

    // Validation du nom
    QRegularExpression regexNomPrenom("^[A-Za-zÀ-ÖØ-öø-ÿ\\s-]+$");
    if (nom.isEmpty() || !regexNomPrenom.match(nom).hasMatch() || nom.length() < 3) {
        erreurs << "Le nom doit contenir au moins 3 caractères et ne doit contenir que des lettres, des espaces ou des traits d'union.";
    }

    // Validation du prénom
    if (prenom.isEmpty() || !regexNomPrenom.match(prenom).hasMatch() || prenom.length() < 3) {
        erreurs << "Le prénom doit contenir au moins 3 caractères et ne doit contenir que des lettres, des espaces ou des traits d'union.";
    }

    // Validation de l'email
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    if (email.isEmpty() || !emailRegex.match(email).hasMatch()) {
        erreurs << "L'adresse email n'est pas valide.";
    }

    // Validation du téléphone
    QRegularExpression phoneRegex("^[0-9]{8}$");
    if (telephone.isEmpty() || !phoneRegex.match(telephone).hasMatch()) {
        erreurs << "Le téléphone doit contenir exactement 8 chiffres.";
    }

    // Validation de l'adresse
    if (adresse.isEmpty() || adresse.length() < 3) {
        erreurs << "L'adresse doit contenir au moins 3 caractères.";
    }

    // Validation du salaire
    bool ok;
    double salaireValue = salaire.toDouble(&ok);
    if (salaire.isEmpty() || !ok || salaireValue < 0) {
        erreurs << "Le salaire doit être un nombre positif.";
    }

    // Validation de la date
    if (date_dembauche > QDate::currentDate()) {
        erreurs << "La date d'embauche ne peut pas être dans le futur.";
    }

    // Vérification de l'unicité de l'email
    employe verifEmail;
    if (verifEmail.emailExists(email, ide)) {
        erreurs << "Cet email est déjà utilisé par un autre employé.";
    }

    // Si des erreurs sont détectées
    if (!erreurs.isEmpty()) {
        QString messageErreur = "Les erreurs suivantes ont été détectées :\n";
        for (const QString &erreur : erreurs) {
            messageErreur += "- " + erreur + "\n";
        }
        QMessageBox::warning(this, "Erreur", messageErreur);

        // Mise en évidence visuelle du champ email si erreur d'unicité
        if (erreurs.contains("Cet email est déjà utilisé par un autre employé.")) {
            ui->emailLineEdit->setStyleSheet("border: 1px solid red;");
        }
        return;
    }

    // Si tout est valide, procéder à la modification
    employe employeAModifier(ide, nom, prenom, email, adresse, poste, salaire, statue, telephone, date_dembauche,m_password, question, reponse );
    if (employeAModifier.modifier(ide)) {
        QMessageBox::information(this, "Succès", "Employé modifié avec succès !");
        afficherTable();

        // Réinitialiser le style du champ email après succès
        ui->emailLineEdit->setStyleSheet("");


        ui->nomLineEdit->clear();
        ui->prenomLineEdit->clear();
        ui->emailLineEdit->clear();
        ui->adresseLineEdit->clear();
        ui->SalaireLineEdit->clear();
        ui->telephoneLineEdit->clear();
        ui->posteComboBox->setCurrentIndex(0);
        ui->statueComboBox->setCurrentIndex(0);
        ui->dateEmbaucheDateEdit->setDate(QDate::currentDate());

    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification de l'employé !");
    }

    // Réinitialiser le bouton "Ajouter"
    ui->ajouterButton->setText("Ajouter");
    QObject::disconnect(ui->ajouterButton, nullptr, nullptr, nullptr);
    connect(ui->ajouterButton, &QPushButton::clicked, this, &EmployeMainWindow::on_ajouterButton_clicked);

    // Réactiver les champs mot de passe et confirmation pour une future insertion
    ui->passwordLineEdit->setEnabled(true);
    ui->confirmPasswordLineEdit->setEnabled(true);

    // Réactiver les champs question et réponse pour une future insertion si besoin
    ui->lineEdit_question->setEnabled(true);
    ui->lineEdit_reponse->setEnabled(true);
}

void EmployeMainWindow::on_supprimerButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un employé à supprimer !");
        return;
    }

    // Récupérer l'ID de l'employé à supprimer
    int id = ui->tableWidget->item(row, 0)->text().toInt();

    // Demander une confirmation avant de supprimer
    if (QMessageBox::question(this, "Confirmation", "Êtes-vous sûr de vouloir supprimer cet employé ?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;

    // Supprimer l'employé de la base de données
    employe p;
    if (p.supprimer(id)) {
        QMessageBox::information(this, "Succès", "Employé supprimé avec succès !");
        afficherTable();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression de l'employé !");
    }
}


void EmployeMainWindow::on_telephoneLineEdit_textEdited(const QString &text) {
    // Force le curseur à rester à la fin
    ui->telephoneLineEdit->setCursorPosition(text.length());
}

//tri
void EmployeMainWindow::appliquerTri(bool keepSelection)
{
    // Sauvegarde de la sélection actuelle
    int selectedId = -1;
    if (keepSelection && ui->tableWidget->currentRow() >= 0) {
        selectedId = ui->tableWidget->item(ui->tableWidget->currentRow(), 0)->text().toInt();
    }

    // Si aucun tri sélectionné, appliquer le tri par ID croissant par défaut
    if (ui->TriComboBox->currentIndex() == -1) {
        ui->TriComboBox->setCurrentIndex(0); // Index du tri par ID
        ui->croissantCheckBox->setChecked(true);
    }

    // Récupère le critère et l'ordre
    employe::SortCriteria critere = static_cast<employe::SortCriteria>(
        ui->TriComboBox->currentData().toInt());

    Qt::SortOrder ordre = ui->croissantCheckBox->isChecked()
                              ? Qt::AscendingOrder
                              : Qt::DescendingOrder;

    // Exécution du tri
    employe e;
    QSqlQueryModel *model = e.trier(critere, ordre);

    // Affichage des résultats
    afficherTableTrie(model);

    // Restauration de la sélection si demandée
    if (keepSelection && selectedId != -1) {
        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            if (ui->tableWidget->item(i, 0)->text().toInt() == selectedId) {
                ui->tableWidget->selectRow(i);
                break;
            }
        }
    }

    // Mise à jour de l'interface
    updateTableVisuals();
}

void EmployeMainWindow::on_TriComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    appliquerTri(true); // Rafraîchit le tri quand on change de critère
}

void EmployeMainWindow::on_croissantCheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        ui->decroissantCheckBox->setChecked(false); // Désactive "Décroissant"
        appliquerTri(true); // Applique le tri croissant
    } else if (!ui->decroissantCheckBox->isChecked()) {
        ui->croissantCheckBox->setChecked(true); // Force à rester coché si aucun ordre n'est sélectionné
    }
}

void EmployeMainWindow::on_decroissantCheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        ui->croissantCheckBox->setChecked(false); // Désactive "Croissant"
        appliquerTri(true); // Applique le tri décroissant
    } else if (!ui->croissantCheckBox->isChecked()) {
        ui->decroissantCheckBox->setChecked(true); // Force à rester coché si aucun ordre n'est sélectionné
    }
}

void EmployeMainWindow::afficherTableTrie(QSqlQueryModel *model)
{
    if (!model || !ui->tableWidget) {
        qDebug() << "Modèle ou tableWidget invalide";
        return;
    }

    // Récupérer le texte de recherche (converti en minuscules pour comparaison insensible à la casse)
    QString searchText = ui->rechercheLineEdit->text().trimmed().toLower();

    // Désactiver les mises à jour pour améliorer les performances
    ui->tableWidget->setUpdatesEnabled(false);

    // Sauvegarde de la sélection et position de défilement
    int currentRow = ui->tableWidget->currentRow();
    int scrollValue = ui->tableWidget->verticalScrollBar()->value();

    // Réinitialisation complète de la table
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);

    // Configuration des colonnes
    ui->tableWidget->setColumnCount(model->columnCount());
    QStringList headers;
    for (int col = 0; col < model->columnCount(); ++col) {
        headers << model->headerData(col, Qt::Horizontal).toString();
    }

    // Utilisation des headers par défaut si vide
    if (headers.isEmpty() || headers.contains("")) {
        headers = {"IDE", "Nom", "Prénom", "Email", "Adresse",
                   "Poste", "Salaire", "Statue", "Téléphone", "Date d'embauche"};
    }

    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setRowCount(model->rowCount());

    // Remplissage des données avec surlignage
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QVariant data = model->data(model->index(row, col));
            QTableWidgetItem *item = new QTableWidgetItem();

            // Traitement spécial pour les différents types de données
            QString displayText;
            switch (data.typeId()) {
            case QMetaType::QDate:
                displayText = data.toDate().toString("dd/MM/yyyy");
                break;
            case QMetaType::Int:
            case QMetaType::Double:
                displayText = QString::number(data.toDouble());
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                break;
            default:
                if (col == 9) { // Colonne Date
                    QString dateStr = data.toString();
                    QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
                    if (!date.isValid()) date = QDate::fromString(dateStr, Qt::ISODate);
                    displayText = date.isValid() ? date.toString("dd/MM/yyyy") : dateStr;
                } else {
                    displayText = data.toString();
                }
            }

            item->setText(displayText);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);

            // Surlignage si le texte correspond (insensible à la casse)
            if (!searchText.isEmpty() && displayText.toLower().contains(searchText)) {
                item->setBackground(QColor(255, 255, 150));  // Jaune clair
                item->setData(Qt::UserRole, true);  // Marquer comme correspondance

                // Optionnel : mettre en gras
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
            }

            ui->tableWidget->setItem(row, col, item);
        }
    }

    // Restauration de la sélection et position de défilement
    if (currentRow >= 0 && currentRow < ui->tableWidget->rowCount()) {
        ui->tableWidget->setCurrentCell(currentRow, 0);
    }
    ui->tableWidget->verticalScrollBar()->setValue(scrollValue);

    // Optimisations d'affichage
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->setAlternatingRowColors(true);

    // Réactiver les mises à jour
    ui->tableWidget->setUpdatesEnabled(true);
    ui->tableWidget->viewport()->update();

    // Message debug
    qDebug() << "Affichage mis à jour avec" << model->rowCount() << "lignes";
    if (!searchText.isEmpty()) {
        qDebug() << "Recherche active pour:" << searchText;
    }
}
void EmployeMainWindow::updateTableVisuals()
{
    // Optimisations d'affichage
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->setAlternatingRowColors(true);

    // Highlight du header de tri actif
    for (int i = 0; i < ui->tableWidget->columnCount(); ++i) {
        auto header = ui->tableWidget->horizontalHeaderItem(i);
        if (i == ui->TriComboBox->currentData().toInt()) {
            header->setForeground(Qt::blue);
        } else {
            header->setForeground(palette().color(QPalette::Text));
        }
    }
}

//filtrage
void EmployeMainWindow::filtrerEmployes()
{
    QString poste = ui->PComboBox->currentText();
    QString statut = ui->statutComboBox->currentText();

    employe emp;
    QSqlQueryModel* filteredModel = emp.filtrer(poste, statut);

    if (!filteredModel || filteredModel->rowCount() == 0) {
        QMessageBox::information(this, "Information", "Aucun résultat trouvé");
        if (filteredModel) delete filteredModel;
        return;
    }

    afficherTableTrie(filteredModel);
    delete filteredModel; // Nettoyage mémoire
}

//recherche
void EmployeMainWindow::effectuerRecherche()
{
    QString texte = ui->rechercheLineEdit->text().trimmed();

    if(texte.length() < 2 && !texte.isEmpty()) return;

    employe emp;
    QSqlQueryModel* model = texte.isEmpty() ? emp.afficher()
                                            : emp.rechercherMultiChamps(texte);

    if(model) {
        afficherTableTrie(model); // Cette méthode va maintenant gérer le surlignage
        delete model;
    }
}

//pdf



#include <QDesktopServices>
#include <QUrl>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

void EmployeMainWindow::on_btnExportPDF_clicked()
{
    // Génération de la date/heure
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timestamp = currentDateTime.toString("yyyyMMdd_HHmmss");
    QString displayDateTime = currentDateTime.toString("dd/MM/yyyy HH:mm");

    // 1. Sélection du fichier avec timestamp
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Exporter PDF",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + "/liste_employes_" + timestamp + ".pdf",
        "Fichiers PDF (*.pdf)"
        );
    if (fileName.isEmpty()) return;

    // 2. Configuration PDF
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setResolution(300);

    // 3. Initialisation QPainter
    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Erreur", "Échec de l'initialisation du PDF");
        return;
    }

    // 4. Calcul des dimensions (augmentation des tailles)
    const int dpi = printer.resolution();
    const int margin = (int)(0.6 * dpi); // Marge augmentée à 0.6 pouce
    QRectF pageRect = printer.pageRect(QPrinter::DevicePixel);
    int yPos = margin + 10; // Position initiale plus basse

    // 5. Définition des polices (tailles augmentées)
    QFont titleFont("Arial", 20, QFont::Bold); // Taille augmentée (20)
    QFont headerFont("Arial", 11, QFont::Bold); // Taille augmentée (11)
    QFont contentFont("Arial", 10); // Taille augmentée (10)
    QFont dateFont("Arial", 9); // Police pour la date

    // 6. Dessin du titre et date d'export (position améliorée)
    painter.setFont(titleFont);
    painter.drawText(margin, yPos, "LISTE DES EMPLOYÉS");

    painter.setFont(dateFont);
    QRect dateRect(pageRect.width() - margin - 300, yPos - 15, 300, 30);
    painter.drawText(dateRect, Qt::AlignRight, "Généré le " + displayDateTime);

    yPos += 50; // Espacement accru après le titre

    // 7. Configuration des colonnes (largeurs augmentées)
    QStringList headers = {"ID", "Nom", "Prénom", "Poste", "Téléphone", "Salaire", "Date Embauche"};
    QVector<int> colWidths = {
        (int)(0.6 * dpi),  // ID (augmenté)
        (int)(1.2 * dpi),  // Nom (augmenté)
        (int)(1.2 * dpi),  // Prénom (augmenté)
        (int)(1.8 * dpi),  // Poste (augmenté)
        (int)(1.0 * dpi),  // Téléphone (augmenté)
        (int)(1.0 * dpi),  // Salaire (augmenté)
        (int)(1.5 * dpi)   // Date (augmenté)
    };

    const int rowHeight = (int)(0.35 * dpi); // Hauteur de ligne augmentée
    const int headerHeight = (int)(0.4 * dpi); // Hauteur en-tête augmentée

    // 8. Dessin des en-têtes (avec espacement accru)
    painter.setFont(headerFont);
    int xPos = margin;

    for (int i = 0; i < headers.size(); ++i) {
        QRect headerRect(xPos, yPos, colWidths[i], headerHeight);
        painter.fillRect(headerRect, QColor(230, 230, 230));
        painter.drawRect(headerRect);
        painter.drawText(headerRect, Qt::AlignCenter, headers[i]);
        xPos += colWidths[i];
    }
    yPos += headerHeight + 5; // Espace supplémentaire après les en-têtes

    // 9. Remplissage des données (avec cellules plus grandes)
    painter.setFont(contentFont);

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        if (yPos + rowHeight > pageRect.height() - margin) {
            printer.newPage();
            yPos = margin;

            // Redessiner les en-têtes sur nouvelle page
            xPos = margin;
            painter.setFont(headerFont);
            for (int i = 0; i < headers.size(); ++i) {
                QRect headerRect(xPos, yPos, colWidths[i], headerHeight);
                painter.fillRect(headerRect, QColor(230, 230, 230));
                painter.drawRect(headerRect);
                painter.drawText(headerRect, Qt::AlignCenter, headers[i]);
                xPos += colWidths[i];
            }
            yPos += headerHeight + 5;
            painter.setFont(contentFont);
        }

        xPos = margin;
        for (int col = 0; col < headers.size(); ++col) {
            int sourceCol = 0;
            switch(col) {
            case 0: sourceCol = 0; break;
            case 1: sourceCol = 1; break;
            case 2: sourceCol = 2; break;
            case 3: sourceCol = 5; break;
            case 4: sourceCol = 8; break;
            case 5: sourceCol = 6; break;
            case 6: sourceCol = 9; break;
            }

            QString text = ui->tableWidget->item(row, sourceCol) ?
                               ui->tableWidget->item(row, sourceCol)->text() : "";

            if (col == 5) {
                bool ok;
                double salaire = text.toDouble(&ok);
                if (ok) text = QString::number(salaire, 'f', 2) + " DT";
            }

            QRect cellRect(xPos, yPos, colWidths[col], rowHeight);
            painter.drawRect(cellRect);

            // Alignement amélioré avec marges internes
            QRect textRect = cellRect.adjusted(5, 5, -5, -5);
            painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
            xPos += colWidths[col];
        }
        yPos += rowHeight;
    }

    // 10. Pied de page avec date (position améliorée)
    yPos += 30; // Espace accru avant le pied de page
    painter.drawLine(margin, yPos, pageRect.width() - margin, yPos);
    yPos += 15;
    painter.setFont(dateFont);
    painter.drawText(margin, yPos, pageRect.width() - 2*margin, 40,
                     Qt::AlignRight,
                     "Document généré le " + displayDateTime + " - " + fileName.section('/', -1));

    // 11. Finalisation
    painter.end();

    // Ouverture du PDF
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    QMessageBox::information(this, "Succès", "PDF généré avec succès!\n" + fileName);
}

void EmployeMainWindow::reinitialiserInterfaceCRUD()
{
    // Réinitialisation des champs
    ui->nomLineEdit->clear();
    ui->prenomLineEdit->clear();
    ui->emailLineEdit->clear();
    ui->adresseLineEdit->clear();
    ui->SalaireLineEdit->clear();
    ui->telephoneLineEdit->clear();
    ui->dateEmbaucheDateEdit->setDate(QDate::currentDate());
    ui->posteComboBox->setCurrentIndex(0);
    ui->statueComboBox->setCurrentIndex(0);

    ui->passwordLineEdit->clear();
    ui->confirmPasswordLineEdit->clear();


    // Réinitialisation des styles
    ui->nomLineEdit->setStyleSheet("");
    ui->prenomLineEdit->setStyleSheet("");
    ui->emailLineEdit->setStyleSheet("");
    ui->adresseLineEdit->setStyleSheet("");
    ui->SalaireLineEdit->setStyleSheet("");
    ui->telephoneLineEdit->setStyleSheet("");
    ui->dateEmbaucheDateEdit->setStyleSheet("");
    ui->passwordLineEdit->setStyleSheet("");
    ui->confirmPasswordLineEdit->setStyleSheet("");

    // Réinitialisation des filtres/tri
    ui->rechercheLineEdit->clear();
    ui->TriComboBox->setCurrentIndex(0);
    ui->croissantCheckBox->setChecked(true);
    ui->decroissantCheckBox->setChecked(false);
    ui->PComboBox->setCurrentIndex(0);
    ui->statutComboBox->setCurrentIndex(0);

    // Réinitialisation du bouton Ajouter/Modifier
    ui->ajouterButton->setText("Ajouter");
    disconnect(ui->ajouterButton, nullptr, nullptr, nullptr);
    connect(ui->ajouterButton, &QPushButton::clicked, this, &EmployeMainWindow::on_ajouterButton_clicked);

    // Désélection et rafraîchissement
    ui->tableWidget->clearSelection();
    afficherTable();
}



void EmployeMainWindow::onTabChanged(int index)
{
    qDebug() << "Changement d'onglet vers l'index:" << index;

    switch(index) {
    case 0: // Onglet CRUD
        reinitialiserInterfaceCRUD();
        afficherTable();
        break;

    case 1: // Onglet Statistiques
        afficherStatsSimplistes();
        updatePieCharts();         // Mise à jour des camemberts
        break;

    default:
        qWarning() << "Onglet non géré:" << index;
        break;
    }
}

void EmployeMainWindow::afficherStatsSimplistes()
{
    // 1. Initialisation des données
    int total = employe::getTotalEmployes();
    auto statuts = employe::getStatutCounts();
    auto salaires = employe::getSalaryStats();

    // 2. Configuration du format numérique
    QLocale locale(QLocale::French); // Pour format français (1 000,00)

    // 3. Style CSS
    const QString styleLabel = "QLabel { font-size: 14px; color: #2c3e50; }";
    const QString styleValue = "font-weight: bold; color: %1;";

    // 4. Mise à jour des labels
    ui->labelTotal->setStyleSheet(styleLabel);
    ui->labelTotal->setText("Nombre total d'employés : " +
                            QString("<span style='%1'>%2</span>")
                                .arg(styleValue.arg("#000000"))
                                .arg(total));

    ui->labelActifs->setStyleSheet(styleLabel);
    ui->labelActifs->setText("Employés actifs : " +
                             QString("<span style='%1'>%2</span>")
                                 .arg(styleValue.arg("#16a085"))
                                 .arg(statuts.value("active", 0)));

    ui->labelInactifs->setStyleSheet(styleLabel);
    ui->labelInactifs->setText("Employés inactifs : " +
                               QString("<span style='%1'>%2</span>")
                                   .arg(styleValue.arg("#c0392b"))
                                   .arg(statuts.value("inactive", 0)));

    ui->labelSalaireMoyen->setStyleSheet(styleLabel);
    ui->labelSalaireMoyen->setText("Salaire moyen : " +
                                   QString("<span style='%1'>%2 DT</span>")
                                       .arg(styleValue.arg("#000000"))
                                       .arg(locale.toString(salaires["avg"], 'f', 2)));

    ui->labelSalaireMinMax->setStyleSheet(styleLabel);
    ui->labelSalaireMinMax->setText("Fourchette salariale : " +
                                    QString("<span style='%1'>%2 DT</span> - <span style='%3'>%4 DT</span>")
                                        .arg(styleValue.arg("#c0392b"))
                                        .arg(locale.toString(salaires["min"], 'f', 2))
                                        .arg(styleValue.arg("#16a085"))
                                        .arg(locale.toString(salaires["max"], 'f', 2)));
}


void EmployeMainWindow::updatePieCharts() {
    // Nettoyage des séries existantes
    if (chartViewStatut->chart()) {
        chartViewStatut->chart()->removeAllSeries();
    }
    if (chartViewPoste->chart()) {
        chartViewPoste->chart()->removeAllSeries();
    }

    // Configuration commune
    QFont labelFont("Arial", 8);
    labelFont.setBold(true);

    // 1. Graphique Statut
    QPieSeries *seriesStatut = new QPieSeries();
    auto statutData = employe::getDataForChart(0);

    // Agrégation des données pour éviter les doublons
    QMap<QString, int> statutAggregated;
    for (const auto &item : statutData) {
        statutAggregated[item.first] += item.second.toInt();
    }

    for (const auto &key : statutAggregated.keys()) {
        QPieSlice *slice = seriesStatut->append(key, statutAggregated.value(key));
        slice->setLabelVisible(true);
        slice->setLabelFont(labelFont);
        slice->setLabel(QString("%1 (%2)").arg(key).arg(statutAggregated.value(key)));
    }

    // 2. Graphique Poste
    QPieSeries *seriesPoste = new QPieSeries();
    auto posteData = employe::getDataForChart(1);

    // Agrégation des données pour éviter les doublons
    QMap<QString, int> posteAggregated;
    for (const auto &item : posteData) {
        posteAggregated[item.first] += item.second.toInt();
    }

    for (const auto &key : posteAggregated.keys()) {
        QPieSlice *slice = seriesPoste->append(key, posteAggregated.value(key));
        slice->setLabelVisible(true);
        slice->setLabelFont(labelFont);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        slice->setLabelArmLengthFactor(0.3);
        slice->setLabel(QString("%1 (%2)").arg(key).arg(posteAggregated.value(key)));
    }

    // Configuration des graphiques
    auto configureChart = [](QChart* chart, QPieSeries* series, const QString& title) {
        chart->addSeries(series);
        chart->setTitle(title);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);
        chart->legend()->setFont(QFont("Arial", 8));
        chart->setMargins(QMargins(5, 5, 5, 5));
    };

    configureChart(chartViewStatut->chart(), seriesStatut, "Statut des employés");
    configureChart(chartViewPoste->chart(), seriesPoste, "Répartition par poste");

    // Ajustement final
    chartViewStatut->setRenderHint(QPainter::Antialiasing);
    chartViewPoste->setRenderHint(QPainter::Antialiasing);
}

void EmployeMainWindow::validerPassword(const QString &text)
{
    m_password = text;
    QRegularExpression regex("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,}$");
    bool valid = regex.match(text).hasMatch();

    ui->passwordLineEdit->setStyleSheet(valid ? "border: 1px solid green;" : "border: 1px solid red;");

    // Vérifier la correspondance si confirmPassword n'est pas vide
    if (!ui->confirmPasswordLineEdit->text().isEmpty()) {
        validerConfirmPassword(ui->confirmPasswordLineEdit->text());
    }
}

void EmployeMainWindow::validerConfirmPassword(const QString &text)
{
    m_confirmPassword = text;
    m_passwordMatch = (text == m_password);

    ui->confirmPasswordLineEdit->setStyleSheet(
        m_passwordMatch ? "border: 1px solid green;" : "border: 1px solid red;");
}





void EmployeMainWindow::on_capturerImageEmploye_clicked()
{
    QUrl url("http://172.20.10.2/capture");
    QNetworkRequest request(url);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imageData = reply->readAll();
            QPixmap pixmap;
            if (pixmap.loadFromData(imageData)) {
                ui->imageLabelEmploye->setPixmap(pixmap.scaled(ui->imageLabelEmploye->size(), Qt::KeepAspectRatio));

                // Générer un nom de fichier long et unique
                QString randomName;
                for (int i = 0; i < 20; ++i) {
                    QChar c = QChar('a' + QRandomGenerator::global()->bounded(26));
                    randomName.append(c);
                }
                QString filename = QDir::currentPath() + "/" + randomName + "_image_employe.jpg";

                if (pixmap.save(filename, "JPG")) {
                    qDebug() << "Image sauvegardée :" << filename;
                    lastImagePath = filename;
                } else {
                    qDebug() << "Échec de la sauvegarde.";
                }

                QTimer::singleShot(1000, this, [=]() {
                    ui->imageLabelEmploye->clear();
                });
            } else {
                qDebug() << "Chargement image échoué.";
            }
        } else {
            qDebug() << "Erreur réseau : " << reply->errorString();
        }
        reply->deleteLater();
        manager->deleteLater();
    });

    manager->get(request);
}

