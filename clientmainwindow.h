#ifndef CLIENTMAINWINDOW_H
#define CLIENTMAINWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "client.h"
#include "employe.h"
// Déclaration anticipée
class employe;

QT_BEGIN_NAMESPACE
namespace Ui {
class ClientMainWindow;
}
QT_END_NAMESPACE

class ClientMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ClientMainWindow(const employe& e, QWidget *parent = nullptr);
    ~ClientMainWindow();

    void setupStatsView();
    void addStatItem(QVBoxLayout* layout, const QString& label, const QString& value);
    void addStatItem(QGraphicsScene* scene, const QString& label, const QString& value, int x, int y);
    void drawBarChart(QGraphicsScene *scene, const Client::ClientStats &stats, int startX, int startY);

private slots:
    void on_ajouterButton_clicked();
    void on_modifierButton_clicked();
    void on_supprimerButton_clicked();
    void afficherTable();
    void afficherStats();
    void on_nomLineEdit_textChanged(const QString &text);
    void on_prenomLineEdit_textChanged(const QString &text);
    void on_adresseLineEdit_textChanged(const QString &text);
    void on_telephoneLineEdit_textChanged(const QString &text);
    void on_emailLineEdit_textChanged(const QString &text);
    void on_emailLineEdit_editingFinished();
    void onTableSelectionChanged();  // Ajoutez cette ligne
    void on_rechercherButton_clicked();
    void on_triNomButton_clicked();
    void on_triDateButton_clicked();
    void on_lineEdit_textChanged(const QString &text);
    void exporterEnPDF();
    void on_bsms_clicked();
    void onGenerateClicked(); // Bouton cliqué
    void onOpenAIReply(QNetworkReply *reply); // Gestion de la réponse

private:
    Ui::ClientMainWindow *ui;
    void setupPieChart();
    void afficherModelDansTable(QSqlQueryModel *model);
    employe employeConnecte;
    QNetworkAccessManager *m_networkManager;
    QElapsedTimer m_requestTimer;

    QString generatePromptFromUI();              // Crée le prompt à envoyer
    void sendPromptToOpenAI(const QString &prompt); // Envoie le prompt



};

#endif // CLIENTMAINWINDOW_H
