#include "connection.h"
#include <QDebug>

Connection::Connection() {}

bool Connection::createconnect()
{
    bool test = false;
    QSqlDatabase db;

    // Vérifier si la connexion existe déjà
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QODBC");
    }

    db.setDatabaseName("Projetarchimanage");  // Nom de la source de données ODBC
    db.setUserName("archimanage");            // Nom de l'utilisateur
    db.setPassword("1234");                   // Mot de passe

    if (db.open()) {
        test = true;
        qDebug() << "Connexion réussie à la base de données!";
    } else {
        qDebug() << "Échec de la connexion: " << db.lastError().text();
    }

    return test;
}
void Connection::closeconnect()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        db.close();
        qDebug() << "Connexion fermée proprement.";
    }
}
