#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QMovie>
#include <QLabel>

#include "motdepasseoublie.h"
#include "employe.h"
#include "arduino.h"

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    bool isLoginSuccessful() const;
    employe getEmployeConnecte() const;

private slots:
    void on_pushButton_login_clicked();
    void on_pushButton_motDePasseOublie_clicked();

    void on_pushButton_clicked();
    void showSplashScreen();

private:
    Ui::Login *ui;
    bool loginSuccessful;
    employe employeConnecte;
    Arduino *arduino1;
};

#endif // LOGIN_H
