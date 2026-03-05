#ifndef MOTDEPASSEOUBLIE_H
#define MOTDEPASSEOUBLIE_H

#include <QDialog>

namespace Ui {
class MotDePasseOublie;
}

class MotDePasseOublie : public QDialog
{
    Q_OBJECT

public:
    explicit MotDePasseOublie(QWidget *parent = nullptr);
    ~MotDePasseOublie();

private slots:
    void on_pushButton_valider_clicked();
    void afficherQuestion();  //  Méthode pour afficher la question après saisie de l'email
    void verifierMotDePasse();
    void on_pushButton_retour_clicked();

private:
    Ui::MotDePasseOublie *ui;
};

#endif // MOTDEPASSEOUBLIE_H
