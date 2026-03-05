#ifndef CAPTCHA_DIALOG_H
#define CAPTCHA_DIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPixmap>
#include <QRandomGenerator>

namespace Ui {
class CaptchaDialog;
}

class CaptchaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaptchaDialog(QWidget *parent = nullptr);
    ~CaptchaDialog();

    bool isCaptchaValid() const;  // Méthode pour savoir si la captcha est valide

private slots:
    void on_pushButton_verifier_clicked();  // Slot pour le clic sur "Valider"
    void regenerateCaptcha();  // Méthode pour régénérer la captcha
    void on_pushButton_regenerate_clicked();  // Slot pour régénérer la captcha
    void on_pushButton_fermer_clicked();  // Slot pour le bouton Fermer


private:
    Ui::CaptchaDialog *ui;
    QString captchaText;  // Texte de la captcha générée
    bool captchaVerified;  // Statut de la captcha (vérifiée ou non)

    // Méthode pour dessiner la captcha avec des déformations
    void drawCaptcha();

    // Méthode pour dessiner des lignes de bruit
    void drawNoise(QPainter &painter);  // Déclaration de la méthode pour le bruit visuel
};

#endif // CAPTCHA_DIALOG_H
