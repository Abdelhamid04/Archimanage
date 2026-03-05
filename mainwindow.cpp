 #include "mainwindow.h"
#include "ui_mainwindow.h"
#include "employemainwindow.h"
#include "clientmainwindow.h"
#include "designsmainwindow.h"
#include "partenairemainwindow.h"
#include "equipementmainwindow.h"
#include "projetmainwindow.h"
#include "login.h"
#include "employe.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>

MainWindow::MainWindow(employe e, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    employeConnecte(e)

{
    ui->setupUi(this);

    setWindowTitle("ArchiManage");
    setWindowIcon(QIcon(":/logo.png"));

    resize(1024, 768);
    ui->stackedWidget->resize(925, 625);

    qDebug() << "MainWindow lancé pour l'employé :" << employeConnecte.getEmail(), employeConnecte.getImagePath() ;

    QString welcomeMessage = "Bienvenue, " + employeConnecte.getNom() + " " + employeConnecte.getPrenom() + " !\n" +
                             "ID : " + QString::number(employeConnecte.getIde()) + "\n" +
                             "Poste : " + employeConnecte.getPoste();
    ui->label_welcome->setText(welcomeMessage);

    pageEmploye = new EmployeMainWindow(employeConnecte, this);
    pageClient = new ClientMainWindow(employeConnecte, this);
    pagedesigns = new designsMainWindow(employeConnecte, this);
    pagePartenaire = new PartenaireMainWindow(employeConnecte, this);
    pageEquipement = new EquipementMainWindow(employeConnecte, this);
    pageProjet = new ProjetMainWindow(employeConnecte, this);

    ui->stackedWidget->addWidget(pageEmploye);  // index = 1
    ui->stackedWidget->addWidget(pageClient);   // index = 2
    ui->stackedWidget->addWidget(pagedesigns);  // index = 3
    ui->stackedWidget->addWidget(pagePartenaire); // index = 4
    ui->stackedWidget->addWidget(pageEquipement); // index = 5
    ui->stackedWidget->addWidget(pageProjet);     // index = 6

    // Désactivation avec installation du filtre pour afficher un message
    if (!verifierAcces({"Res_RH", "Admin"})) {
        ui->pushButton_GestionEmployes->setEnabled(false);
        ui->pushButton_GestionEmployes->installEventFilter(this);
    }
    if (!verifierAcces({"Res_Client", "Admin"})) {
        ui->pushButton_GestionClients->setEnabled(false);
        ui->pushButton_GestionClients->installEventFilter(this);
    }
    if (!verifierAcces({"Res_Design", "Admin"})) {
        ui->pushButton_GestionDesign->setEnabled(false);
        ui->pushButton_GestionDesign->installEventFilter(this);
    }
    if (!verifierAcces({"Res_Partenaires", "Admin"})) {
        ui->pushButton_GestionPartenaire->setEnabled(false);
        ui->pushButton_GestionPartenaire->installEventFilter(this);
    }
    if (!verifierAcces({"Res_Équipements", "Admin"})) {
        ui->pushButton_GestionEquipement->setEnabled(false);
        ui->pushButton_GestionEquipement->installEventFilter(this);
    }
    if (!verifierAcces({"Res_Projet", "Admin"})) {
        ui->pushButton_GestionProjet->setEnabled(false);
        ui->pushButton_GestionProjet->installEventFilter(this);
    }

    // Connexions normales pour les boutons
    connect(ui->pushButton_Accueil, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(0);
    });

    connect(ui->pushButton_GestionEmployes, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(1);
    });

    connect(ui->pushButton_GestionClients, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(2);
    });

    connect(ui->pushButton_GestionDesign, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(3);
    });

    connect(ui->pushButton_GestionPartenaire, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(4);
    });

    connect(ui->pushButton_GestionEquipement, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(5);
    });

    connect(ui->pushButton_GestionProjet, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(6);
    });

    connect(ui->pushButton_logout, &QPushButton::clicked, this, &MainWindow::logout);
    ui->stackedWidget->setCurrentIndex(0);
    ui->profile_photo->setPixmap(QPixmap(employeConnecte.getImagePath()).scaled(ui->profile_photo->size(), Qt::KeepAspectRatio));
}


bool MainWindow::verifierAcces(QStringList postesAutorises)
{
    // Si l'utilisateur est admin, l'accès est toujours autorisé
    if (employeConnecte.getPoste() == "Admin") {
        return true;
    }

    // Si l'utilisateur n'est pas admin, vérifier si son poste est autorisé
    return postesAutorises.contains(employeConnecte.getPoste());
}



MainWindow::~MainWindow()
{
    delete ui;
}

// Fonction de déconnexion
void MainWindow::logout() {
    // Réinitialiser les informations de l'utilisateur connecté
    employeConnecte = employe();  // Réinitialiser l'objet employé connecté

    // Fermer la fenêtre actuelle
    this->close();

    // Ouvrir la fenêtre de connexion
    Login *loginWindow = new Login();
    loginWindow->show();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (obj == ui->pushButton_GestionEmployes) {
            QMessageBox::warning(this, "Accès refusé", "Vous n'avez pas accès à la gestion des employés.");
            return true;
        }
        if (obj == ui->pushButton_GestionClients) {
            QMessageBox::warning(this, "Accès refusé", "Vous n'avez pas accès à la gestion des clients.");
            return true;
        }
        if (obj == ui->pushButton_GestionDesign) {
            QMessageBox::warning(this, "Accès refusé", "Vous n'avez pas accès à la gestion des designs.");
            return true;
        }
        if (obj == ui->pushButton_GestionPartenaire) {
            QMessageBox::warning(this, "Accès refusé", "Vous n'avez pas accès à la gestion des partenaires.");
            return true;
        }
        if (obj == ui->pushButton_GestionEquipement) {
            QMessageBox::warning(this, "Accès refusé", "Vous n'avez pas accès à la gestion des équipements.");
            return true;
        }
        if (obj == ui->pushButton_GestionProjet) {
            QMessageBox::warning(this, "Accès refusé", "Vous n'avez pas accès à la gestion des projets.");
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


