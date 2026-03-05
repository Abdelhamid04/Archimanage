#ifndef DESIGNSMAINWINDOW_H
#define DESIGNSMAINWINDOW_H

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
#include <QLabel>
#include <QEvent>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFormLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QBuffer>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QSettings>  // Ajoutez cette ligne avec les autres includes
#include "employe.h"

class ChatbotDialog;

// Déclaration anticipée de la classe Ui
namespace Ui {
class designsMainWindow;
}

// Classe utilitaire pour gérer les clics sur QLabel
class ClickHandler : public QObject {
    Q_OBJECT
public:
    ClickHandler(QLabel *label, QObject *parent = nullptr)
        : QObject(parent), label(label) {
        label->installEventFilter(this);
    }

signals:
    void clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonPress) {
            emit clicked();
            return true;
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QLabel *label;
};

class designsMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit designsMainWindow(const employe& e, QWidget *parent = nullptr);
    ~designsMainWindow();
    int generateNewId();  // Add this line

private slots:
    void on_ajouterButton_clicked();
    void on_modifierButton_clicked();
    void on_supprimerButton_clicked();
    void on_selectImageButton_clicked();
    void on_modifierButton_confirmed(int id);
    void on_exportPdfButton_clicked();
    void on_exportExcelButton_clicked();
    void on_importExcelButton_clicked();
    void on_searchButton_clicked();
    void on_resetButton_clicked();
    void on_pushButton_18_clicked(); // Catalogue
    void onSearchClicked();

    void setCurrentUser(int userId);


private:
    Ui::designsMainWindow *ui;
    QString imagePath;
    int currentUserId;  // Déclaration du membr
    QNetworkAccessManager *networkManager;
    QNetworkAccessManager *imageDownloadManager;  // Ajoutez cette ligne
    QPixmap lastGeneratedImage;
    bool isProcessingImage;
    QString openaiApiKey;
    void afficherTable();
    void setupStatistiques();
    bool exportToExcel(const QString &fileName);
    bool importFromExcel(const QString &fileName);
    void generateImageFromPrompt(const QString &prompt);
    void displayGeneratedImage(const QPixmap &pixmap, const QString &prompt);

    void handleImageSave(const QUrl &link);
    void handleImageLink(const QUrl &url);
    employe employeConnecte;
};

#endif // DESIGNSdesignsMainWindow_H
