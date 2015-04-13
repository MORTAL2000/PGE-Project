#ifndef PGECONNECTEDUSER_H
#define PGECONNECTEDUSER_H

#include <QTcpSocket>

enum class PGEUserLevel {
    PGEPERM_User,
    PGEPERM_Admin
};

struct PGEConnectedUser {
    QString Username;
    PGEUserLevel permissionLevel;
    QTcpSocket* socket;
};


#endif // PGECONNECTEDUSER_H
