#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "employe.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>


// Déclarations anticipées pour éviter les inclusions circulaires
class EmployeMainWindow;
class ClientMainWindow;
class designsMainWindow;
class PartenaireMainWindow;
class EquipementMainWindow;
class employe;
//proj
class ProjetMainWindow;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(employe e, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Déconnexion
    void logout();


private:
    Ui::MainWindow *ui;
    EmployeMainWindow *pageEmploye;
    ClientMainWindow *pageClient;
    designsMainWindow *pagedesigns;
    PartenaireMainWindow *pagePartenaire;
    EquipementMainWindow *pageEquipement;
    //
    ProjetMainWindow *pageProjet;
    //
    employe employeConnecte;
    bool verifierAcces(QStringList postesAutorises);
    bool eventFilter(QObject *obj, QEvent *event) override;

    // ----------- Camera----------- //




};

#endif // MAINWINDOW_H
