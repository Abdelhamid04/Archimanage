#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

class Connection
{
public:
    Connection();
    bool createconnect();  // Établit la connexion à la base de données
    void closeconnect();   // Ferme la connexion proprement
};

#endif // CONNECTION_H
