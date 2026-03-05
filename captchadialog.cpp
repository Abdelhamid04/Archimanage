#include "captchadialog.h"
#include "ui_captchadialog.h"
#include <QMessageBox>
#include <QRandomGenerator>
#include <QTimer>
#include <QPainter>
#include <QColor>

CaptchaDialog::CaptchaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaptchaDialog),
    captchaVerified(false)  // Initialisation à false
{
    ui->setupUi(this);

    // Connecter le bouton de régénération à la méthode correspondante
    connect(ui->pushButton_regenerate, &QPushButton::clicked, this, &CaptchaDialog::on_pushButton_regenerate_clicked);


    // Générer une captcha au hasard dès l'ouverture du dialogue
    regenerateCaptcha();
}

CaptchaDialog::~CaptchaDialog()
{
    delete ui;
}

void CaptchaDialog::regenerateCaptcha()
{
    // Génère une captcha aléatoire avec 6 caractères (lettres et chiffres)
    QString characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    captchaText.clear();
    for (int i = 0; i < 6; ++i) {
        int index = QRandomGenerator::global()->bounded(characters.size());
        captchaText.append(characters.at(index));
    }

    // Met à jour l'affichage de la captcha sur l'interface
    drawCaptcha();
}

void CaptchaDialog::drawCaptcha()
{
    // Création d'une image pour dessiner la captcha
    QPixmap pixmap(300, 100);
    pixmap.fill(Qt::white);  // Remplir l'arrière-plan en blanc

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Choisir une police plus personnalisée
    QFont font("Comic Sans MS", 35, QFont::Bold); // Police Comic Sans MS, taille 35, en gras
    painter.setFont(font);

    // Dessiner le texte de la captcha avec déformations et couleurs aléatoires
    for (int i = 0; i < captchaText.length(); ++i) {
        painter.save();
        painter.translate(30 + i * 30, 50);  // Positionnement des caractères
        painter.rotate(QRandomGenerator::global()->bounded(-30, 30));  // Rotation aléatoire

        // Appliquer une couleur aléatoire pour chaque caractère
        QColor color(QRandomGenerator::global()->bounded(50, 255),
                     QRandomGenerator::global()->bounded(50, 255),
                     QRandomGenerator::global()->bounded(50, 255));
        painter.setPen(color);

        painter.drawText(0, 0, QString(captchaText[i]));
        painter.restore();
    }

    // Ajouter du bruit visuel : Dessiner des lignes de bruit
    drawNoise(painter);

    // Afficher l'image sur le QLabel
    ui->label_captcha->setPixmap(pixmap);
}

void CaptchaDialog::drawNoise(QPainter &painter)
{
    // Ajouter des lignes de bruit
    for (int i = 0; i < 5; ++i) {
        int x1 = QRandomGenerator::global()->bounded(0, 200);
        int y1 = QRandomGenerator::global()->bounded(0, 80);
        int x2 = QRandomGenerator::global()->bounded(0, 200);
        int y2 = QRandomGenerator::global()->bounded(0, 80);

        painter.setPen(QPen(QColor(0, 0, 0, 100), 1));  // Couleur noire semi-transparente
        painter.drawLine(x1, y1, x2, y2);
    }

    // Ajouter des lignes horizontales pour brouiller l'image
    for (int i = 0; i < 5; ++i) {
        int y = QRandomGenerator::global()->bounded(0, 80);
        painter.setPen(QPen(QColor(0, 0, 0, 100), 1));  // Couleur noire semi-transparente
        painter.drawLine(0, y, 200, y);
    }

    // Ajouter des lignes verticales pour un effet de bruit supplémentaire
    for (int i = 0; i < 5; ++i) {
        int x = QRandomGenerator::global()->bounded(0, 200);
        painter.setPen(QPen(QColor(0, 0, 0, 100), 1));  // Couleur noire semi-transparente
        painter.drawLine(x, 0, x, 80);
    }
}

void CaptchaDialog::on_pushButton_verifier_clicked()
{
    QString enteredCaptcha = ui->lineEdit_captcha->text().trimmed();  // Récupère la saisie de l'utilisateur

    // Vérifie si la captcha saisie correspond au texte généré
    if (enteredCaptcha == captchaText) {
        captchaVerified = true;  // La captcha est validée
        accept();  // Ferme la fenêtre avec un statut accepté
    } else {
        captchaVerified = false;  // La captcha est invalide
        QMessageBox::warning(this, "Erreur", "Captcha incorrect. Veuillez réessayer.");

        // Vide le champ de saisie du captcha si incorrect
        ui->lineEdit_captcha->clear();  // Vide le champ de saisie

        regenerateCaptcha();  // Regénère la captcha après un échec
    }
}

// Méthode pour savoir si la captcha a été validée
bool CaptchaDialog::isCaptchaValid() const
{
    return captchaVerified;  // Retourne l'état de la validation
}

void CaptchaDialog::on_pushButton_regenerate_clicked()
{
    // Vide le champ de saisie du captcha
    ui->lineEdit_captcha->clear();  // Vide le champ de saisie

    regenerateCaptcha();  // Appelle la méthode pour régénérer la captcha
}

void CaptchaDialog::on_pushButton_fermer_clicked()
{
    this->reject();  // Ferme la boîte de dialogue sans valider
}
