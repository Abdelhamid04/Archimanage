#ifndef PARTEANIREMAINWIDOW_H
#define PARTEANIREMAINWIDOW_H
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QAxObject>  // Pour les opérations Excel
#include <QFileDialog> // Pour les dialogues de fichiers
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QTableWidget>
#include <QVariantMap>
#include <QtNetwork>
#include <QSslSocket>
#include <QTextEdit>
#include "employe.h"
#include "arduino.h"

#define EMAIL_COLUMN_INDEX 3

namespace Ui {
class PartenaireMainWindow;
}

class PartenaireMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PartenaireMainWindow(const employe& e, QWidget *parent);
    ~PartenaireMainWindow();
    void logAction(const QString &actionType, int partenaireId, const QString &details,
                   const QVariantMap &oldValues = {}, const QVariantMap &newValues = {});



signals:
    void modifierButtonConfirmed(int id); // Signal pour la confirmation de modification

private slots:
    void on_ajouterButton_clicked();
    void on_modifierButton_clicked();
    void on_modifierButton_confirmed(int id);
    void on_supprimerButton_clicked();
    void on_exportExcelButton_clicked();
    void on_importExcelButton_clicked();
    void on_exportPdfButton_clicked();
    void on_searchButton_clicked();
    void on_resetButton_clicked();
    // void on_sendManualEmailButton_clicked();
    void on_emailButton_clicked();
    void readSerialData();
    void on_pushButtonIdentifier_clicked();
    void on_pushButtonEnregistrer_clicked();

private:
    Ui::PartenaireMainWindow *ui;

    Arduino *arduino2;
    QNetworkAccessManager *networkManager;

    QString openaiApiKey;
    void afficherTable();
    void setupStatistiques();
    bool exportToExcel(const QString &fileName);
    bool importFromExcel(const QString &fileName);
    void showHistory();
    bool sendAutomaticMail(const QString &recipient, const QString &subject, const QString &body);
    bool sendSmtpCommand(QSslSocket &socket, const QString &command, const QString &expectedResponse);
    QString generateSmartEmail(const QString &nom, const QString &prenom, const QString &type);

    void generateEmailWithOpenAI(const QString &nom, const QString &prenom, const QString &type, const QString &objet, QTextEdit *bodyEdit);
    bool envoyerCommandeArduino(const QString &commande);
    void afficherPartenaireParEmpreinte(int id);
    void enregistrerEmpreinteDansArduino();
    employe employeConnecte;
};

#endif // PARTENAIREPartenaireMainWindow_H
