#ifndef EMPLOEMAINWINDOW_H
#define EMPLOEMAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QRegularExpression>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPalette>
#include <QTimer>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QFileDialog>
#include <QStandardPaths>
#include <numeric>
#include <QResizeEvent>
#include <QChartView>
#include <QPieSeries>
#include <QCryptographicHash>
#include "employe.h"
#include "chatbotemploye.h"

QT_BEGIN_NAMESPACE
class QChart;
class QChartView;
class QPieSeries;

namespace Ui {
class EmployeMainWindow;
}

class EmployeMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeMainWindow(const employe& e, QWidget *parent = nullptr);
    ~EmployeMainWindow();

signals:
    void modifierButtonConfirmed(int ide);

private slots:
    // CRUD
    void on_ajouterButton_clicked();
    void on_modifierButton_clicked();
    void on_modifierButton_confirmed(int ide);
    void on_supprimerButton_clicked();

    // Validation
    void on_telephoneLineEdit_textEdited(const QString &text);
    void validerNom(const QString &text);
    void validerPrenom(const QString &text);
    void validerEmail(const QString &text);
    void validerTelephone(const QString &text);
    void validerAdresse(const QString &text);
    void validerSalaire(const QString &text);
    void validerDate(const QDate &date);

    // Tri et Filtrage
    void on_TriComboBox_currentIndexChanged(int index);
    void on_croissantCheckBox_stateChanged(int state);
    void on_decroissantCheckBox_stateChanged(int state);
    void filtrerEmployes();

    // Recherche
    void effectuerRecherche();

    // Export
    void on_btnExportPDF_clicked();

    // Onglets
    void onTabChanged(int index); // Onglet employe

    // Mise à jour des graphiques
    void updatePieCharts();  // Méthode pour mettre à jour les camemberts

    // Validation mot de passe
    void validerPassword(const QString &text);
    void validerConfirmPassword(const QString &text);





    void on_capturerImageEmploye_clicked();

private:
    Ui::EmployeMainWindow *ui;
    QTimer* searchTimer; // Timer pour la recherche avec délai de 300ms
    bool m_emailError = false;

    // Affichage
    void afficherTable();
    void afficherTableTrie(QSqlQueryModel *model);
    void updateTableVisuals();

    // Gestion des données
    void appliquerTri(bool keepSelection = false);
    bool tousLesChampsSontRemplis();

    // Réinitialisation des formulaires
    void reinitialiserInterfaceCRUD();
    void reinitialiserFormulaire();

    // Initialisation des composants UI
    void setupUIComponents();
    void setupConnections();

    void afficherStatsSimplistes();

    // Gestion des mots de passe
    QString m_password;
    QString m_confirmPassword;
    bool m_passwordMatch = false;

    // Graphiques
    QChartView *chartViewStatut;  // Graphique statut (actif/inactif)
    QChartView *chartViewPoste;   // Graphique postes

    // Chatbot employé
    ChatBotEmploye *chatBotWidget;

    // Employé connecté
    employe employeConnecte;

    QString lastImagePath;
};

#endif // EMPLOEMAINWINDOW_H
