#include <QApplication>
#include "login.h"
#include "mainwindow.h"
#include "connection.h"
#include "animatedsplash.h" // N'oubliez pas d'inclure le header

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ✅ Étape 0 : Afficher le splash screen animé
    AnimatedSplash splash;
    splash.startAnimation();
    QApplication::processEvents(); // Force le traitement des événements pour afficher le splash

    // ✅ Étape 1 : Créer la connexion à la base
    Connection c;
    if (!c.createconnect()) {
        splash.stopAnimation();
        QMessageBox::critical(nullptr, "Erreur", "Connexion à la base de données échouée !");
        return -1;
    }

    // ✅ Étape 2 : Lancer la fenêtre de connexion
    Login loginWindow;
    splash.stopAnimation(); // Fermer le splash quand le login apparaît

    if (loginWindow.exec() == QDialog::Accepted && loginWindow.isLoginSuccessful()) {
        employe emp = loginWindow.getEmployeConnecte();
        MainWindow mainWindow(emp);
        mainWindow.show();

        return a.exec();
    }

    return 0;
}
