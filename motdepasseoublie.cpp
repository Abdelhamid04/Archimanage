#include "motdepasseoublie.h"
#include "ui_motdepasseoublie.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QRegularExpression>

MotDePasseOublie::MotDePasseOublie(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MotDePasseOublie)
{
    ui->setupUi(this);

    ui->lineEdit_newPassword->setEchoMode(QLineEdit::Password);

    // Afficher la question secrète après saisie de l'email
    connect(ui->lineEdit_email, &QLineEdit::editingFinished, this, &MotDePasseOublie::afficherQuestion);

    // Contrôle dynamique du mot de passe
    connect(ui->lineEdit_newPassword, &QLineEdit::textChanged, this, &MotDePasseOublie::verifierMotDePasse);
}

MotDePasseOublie::~MotDePasseOublie()
{
    delete ui;
}

void MotDePasseOublie::afficherQuestion()
{
    QString email = ui->lineEdit_email->text().trimmed();

    if (email.isEmpty())
        return;

    QSqlQuery query;
    query.prepare("SELECT question FROM employe WHERE email = :email");
    query.bindValue(":email", email);

    if (query.exec() && query.next()) {
        QString question = query.value("question").toString();
        ui->label_question->setText("Question secrète : " + question);
    } else {
        ui->label_question->setText("Aucune question trouvée pour cet email.");
    }
}

void MotDePasseOublie::verifierMotDePasse()
{
    QString password = ui->lineEdit_newPassword->text().trimmed();
    QRegularExpression regex("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,}$");

    if (regex.match(password).hasMatch()) {
        // ✅ Bordure verte
        ui->lineEdit_newPassword->setStyleSheet("QLineEdit { border: 2px solid green; border-radius: 4px; }");
    } else {
        // ❌ Bordure rouge
        ui->lineEdit_newPassword->setStyleSheet("QLineEdit { border: 2px solid red; border-radius: 4px; }");
    }
}

void MotDePasseOublie::on_pushButton_valider_clicked()
{
    QString email = ui->lineEdit_email->text().trimmed();
    QString reponse = ui->lineEdit_reponse->text().trimmed();
    QString newPassword = ui->lineEdit_newPassword->text().trimmed();

    if (email.isEmpty() || reponse.isEmpty() || newPassword.isEmpty()) {
        QMessageBox::warning(this, "Champs requis", "Veuillez remplir tous les champs.");
        return;
    }

    // Vérification de la validité du mot de passe
    QRegularExpression regex("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,}$");
    if (!regex.match(newPassword).hasMatch()) {
        QMessageBox::warning(this, "Mot de passe invalide", "Le mot de passe doit contenir au moins 8 caractères, incluant lettres et chiffres.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT reponse FROM employe WHERE email = :email");
    query.bindValue(":email", email);

    if (query.exec() && query.next()) {
        QString reponseStockee = query.value("reponse").toString();

        if (reponse == reponseStockee) {
            QString newPasswordHash = QString(QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256).toHex());

            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE employe SET password = :password WHERE email = :email");
            updateQuery.bindValue(":password", newPasswordHash);
            updateQuery.bindValue(":email", email);

            if (updateQuery.exec()) {
                QMessageBox::information(this, "Succès", "Votre mot de passe a été réinitialisé.");
                accept();  // Ferme la fenêtre
            } else {
                QMessageBox::critical(this, "Erreur SQL", updateQuery.lastError().text());
            }
        } else {
            QMessageBox::warning(this, "Réponse incorrecte", "La réponse secrète est incorrecte.");
        }
    } else {
        QMessageBox::warning(this, "Email introuvable", "Aucun compte trouvé pour cet email.");
    }
}

void MotDePasseOublie::on_pushButton_retour_clicked()
{
    this->close(); // Ferme la fenêtre actuelle
    // Optionnel : ici tu peux aussi émettre un signal pour rouvrir la fenêtre de login si elle est gérée ailleurs
}
