#ifndef CHATBOTEMPLOYE_H
#define CHATBOTEMPLOYE_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QTextEdit>    // Ajout de l'inclusion pour QTextEdit
#include <QLineEdit>    // Ajout de l'inclusion pour QLineEdit
#include <QPushButton>  // Ajout de l'inclusion pour QPushButton

class QTextEdit;
class QLineEdit;
class QPushButton;

class ChatBotEmploye : public QWidget
{
    Q_OBJECT

public:
    explicit ChatBotEmploye(QWidget *parent = nullptr);
    void sendMessage(const QString &message);

signals:
    void responseReceived(const QString &response);

private slots:
    void handleNetworkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    QString apiKey;
    QTextEdit *chatHistory;
    QLineEdit *messageInput;
    QPushButton *sendButton;
};

#endif // CHATBOTEMPLOYE_H
