#include "chatbotemploye.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QDebug>
#include <QNetworkReply>
#include <QVBoxLayout>

ChatBotEmploye::ChatBotEmploye(QWidget *parent) : QWidget(parent) {
    networkManager = new QNetworkAccessManager(this);
    apiKey = QString::fromUtf8(qgetenv("OPENAI_API_KEY"));

    // Création des widgets
    chatHistory = new QTextEdit(this);
    chatHistory->setReadOnly(true); // Empêche l'édition
    messageInput = new QLineEdit(this);
    sendButton = new QPushButton("Envoyer", this);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chatHistory);
    layout->addWidget(messageInput);
    layout->addWidget(sendButton);

    // Connexion du bouton
    connect(sendButton, &QPushButton::clicked, this, [this]() {
        QString message = messageInput->text();
        if (!message.isEmpty()) {
            chatHistory->append("Vous: " + message); // Affiche le message de l'utilisateur
            this->sendMessage(message);
            messageInput->clear();
        }
    });

    // Connexion pour afficher les réponses
    connect(this, &ChatBotEmploye::responseReceived, this, [this](const QString &response) {
        chatHistory->append("Bot: " + response);
    });

    this->setFixedSize(700, 500);
      // Redimensionne la fenêtre du chatbot à 600x400

}

void ChatBotEmploye::sendMessage(const QString &message)
{
    if (message.isEmpty()) return;
    if (apiKey.isEmpty()) {
        emit responseReceived("OPENAI_API_KEY non configuree.");
        return;
    }

    QUrl url("https://api.openai.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());

    // Préparation du message au format OpenAI
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = message;

    QJsonArray messages;
    messages.append(userMessage);

    QJsonObject json;
    json["model"] = "gpt-3.5-turbo"; // ou "gpt-4" si tu l’as
    json["messages"] = messages;

    QJsonDocument jsonDoc(json);
    QByteArray postData = jsonDoc.toJson();

    QNetworkReply *reply = networkManager->post(request, postData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        this->handleNetworkReply(reply);
    });
}

void ChatBotEmploye::handleNetworkReply(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit responseReceived("Erreur OpenAI : " + reply->errorString());
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    if (jsonResponse.isObject()) {
        QJsonObject obj = jsonResponse.object();
        if (obj.contains("choices")) {
            QJsonArray choices = obj["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject firstChoice = choices[0].toObject();
                QString replyText = firstChoice["message"].toObject()["content"].toString();
                emit responseReceived(replyText.trimmed());
                return;
            }
        }
    }

    emit responseReceived("Réponse inattendue d’OpenAI");
}
