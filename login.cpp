#include "login.h"
#include "ui_login.h"

#include "motdepasseoublie.h"
#include "captchadialog.h"

#include "employe.h"


#include <QProcess>
#include <QSplashScreen>
#include <QMovie>
#include <QTimer>
#include <QLabel>




Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login),
    loginSuccessful(false)
{
    showSplashScreen();
    ui->setupUi(this);
    setWindowIcon(QIcon(":/logo.png"));
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    arduino1 = new Arduino(this);
    arduino1->setPortName("COM10");
    arduino1->connect_arduino();
}

Login::~Login()
{
    delete ui;
}
void Login::showSplashScreen()
{
    QSplashScreen *splash = new QSplashScreen;
    splash->setWindowFlag(Qt::FramelessWindowHint); // Supprime la bordure
    splash->setAttribute(Qt::WA_TranslucentBackground); // Transparent

    QMovie *movie = new QMovie(":/loader.gif");  // Mets ici le chemin de ton GIF
    QLabel *loaderLabel = new QLabel(splash);
    loaderLabel->setMovie(movie);
    loaderLabel->setGeometry(0, 0, movie->frameRect().width(), movie->frameRect().height());

    splash->resize(movie->frameRect().size());
    splash->show();
    movie->start();

    // Fermer le splash après 10 secondes
    QTimer::singleShot(3000, [=]() {
        movie->stop();
        splash->close();
        delete splash;
    });

    // Attendre que l'animation finisse
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, &QEventLoop::quit);
    loop.exec();
}

bool Login::isLoginSuccessful() const
{
    return loginSuccessful;
}

void Login::on_pushButton_login_clicked()
{
    QString email = ui->lineEdit_email->text().trimmed();
    QString password = ui->lineEdit_password->text().trimmed();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Champs requis", "Veuillez remplir tous les champs.");
        return;
    }

    QSqlQuery emailQuery;
    emailQuery.prepare("SELECT password FROM employe WHERE email = :email");
    emailQuery.bindValue(":email", email);

    if (emailQuery.exec()) {
        if (emailQuery.next()) {
            QString storedHash = emailQuery.value("password").toString();
            QString enteredHash = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

            if (storedHash == enteredHash) {
                QSqlQuery infoQuery;
                infoQuery.prepare("SELECT IDE, NOM, PRENOM, EMAIL, ADRESSE, POSTE, SALAIRE, STATUE, TELEPHONE, DATE_DEMBAUCHE, PASSWORD, QUESTION, REPONSE, IMAGE_PATH FROM employe WHERE email = :email");
                infoQuery.bindValue(":email", email);

                if (infoQuery.exec() && infoQuery.next()) {
                    int ide = infoQuery.value("IDE").toInt();
                    QString nom = infoQuery.value("nom").toString();
                    QString prenom = infoQuery.value("prenom").toString();
                    QString adresse = infoQuery.value("adresse").toString();
                    QString poste = infoQuery.value("poste").toString();
                    QString salaire = infoQuery.value("salaire").toString();
                    QString statue = infoQuery.value("statue").toString();
                    QString telephone = infoQuery.value("telephone").toString();
                    QDate dateEmbauche = infoQuery.value("date_dembauche").toDate();
                    QString motDePasse = infoQuery.value("password").toString();
                    QString question = infoQuery.value("question").toString();
                    QString reponse = infoQuery.value("reponse").toString();
                    QString image_path = infoQuery.value("image_path").toString();

                    employeConnecte = employe(ide, nom, prenom, email, adresse, poste, salaire, statue, telephone, dateEmbauche, motDePasse, question, reponse, image_path);

                    // Envoi du message de bienvenue à l'Arduino
                    QString welcomeMessage = "Bienvenue " + nom;
                    QByteArray messageData = welcomeMessage.toUtf8();

                    // Supposons que vous avez accès à l'objet Arduino via une variable membre
                    if (arduino1->write_to_arduino(messageData) == 0) {
                        qDebug() << "Message envoyé à l'Arduino:" << welcomeMessage;
                    } else {
                        qDebug() << "Échec de l'envoi à l'Arduino";
                    }
                    arduino1->write_to_arduino(" OPEN");
                }

                loginSuccessful = true;
                accept(); // Connexion réussie
            } else {
                QMessageBox::warning(this, "Erreur", "Mot de passe incorrect.");
                ui->lineEdit_password->clear();
                ui->lineEdit_password->setFocus();
            }
        } else {
            QMessageBox::warning(this, "Erreur", "Cet email n'existe pas.");
            ui->lineEdit_email->clear();
            ui->lineEdit_password->clear();
            ui->lineEdit_email->setFocus();
        }
    } else {
        QMessageBox::critical(this, "Erreur SQL", emailQuery.lastError().text());
    }
}

void Login::on_pushButton_motDePasseOublie_clicked()
{
    CaptchaDialog captchaDialog(this);
    captchaDialog.setModal(true);

    int result = captchaDialog.exec();

    if (result == QDialog::Accepted && captchaDialog.isCaptchaValid()) {
        MotDePasseOublie fenetre;
        fenetre.setModal(true);
        fenetre.exec();
    }


}

employe Login::getEmployeConnecte() const {
    return employeConnecte;
}


void Login::on_pushButton_clicked()
{
    // Chemin vers votre script Python
    QString scriptPath = "C:/Users/Ellab/OneDrive/Bureau/FACE RECOGNITION/main.py";

    // Créer un processus pour exécuter le script Python
    QProcess *process = new QProcess(this);
    process->start("python", QStringList() << scriptPath);

    // Vérifier si le processus a démarré avec succès
    if (process->waitForStarted()) {
        qDebug() << "Script Python démarré avec succès.";
    } else {
        qDebug() << "Erreur lors du démarrage du script Python.";
    }
    connect(process, &QProcess::readyReadStandardOutput, [=]() {
        QString output = process->readAllStandardOutput();
        QStringList parts = output.split(' ');
        QString num_role = parts.last().trimmed();
        qDebug() << "Code numérique:" << num_role;
        QString dateString = "25/04/24";
        QDate date = QDate::fromString(dateString, "dd/MM/yy");
        employeConnecte = employe(122, "admin", "admin", "admin@gmail.com", "admin", "Admin", "111", "active", "12345678", date, "240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9", "votre role ? ", "admin");
        loginSuccessful = true;
        accept(); // Connexion réussie
    });
}

