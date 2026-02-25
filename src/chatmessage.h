#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QColor>

struct ChatMessage {
    QString id;
    QString username;
    QString displayName;
    QString text;
    QColor color;
    QDateTime timestamp;
    QStringList badges;
    QStringList emotes;
    bool deleted = false;
    bool isAction = false;
    bool highlighted = false;
    QString userId;
    int bits = 0;
};

#endif
