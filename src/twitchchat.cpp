#include "twitchchat.h"
#include "settings.h"
#include "constants.h"
#include <QRegularExpression>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

TwitchChat::TwitchChat(QObject* parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    pingTimer = new QTimer(this);
    nam = new QNetworkAccessManager(this);
    
    connect(socket, &QTcpSocket::connected, this, &TwitchChat::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TwitchChat::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TwitchChat::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &TwitchChat::onError);
    
    connect(pingTimer, &QTimer::timeout, this, &TwitchChat::handlePing);
    pingTimer->setInterval(60000);
}

TwitchChat::~TwitchChat() {
    disconnect();
}

void TwitchChat::connectToChat(const QString& token, const QString& username) {
    currentToken = token;
    currentUsername = username;
    
    socket->connectToHost("irc.chat.twitch.tv", 6667);
}

void TwitchChat::onConnected() {
    socket->write("CAP REQ :twitch.tv/tags twitch.tv/commands\r\n");
    socket->write(QString("PASS oauth:%1\r\n").arg(currentToken).toUtf8());
    socket->write(QString("NICK %1\r\n").arg(currentUsername).toUtf8());
    
    pingTimer->start();
    emit connected();
}

void TwitchChat::onDisconnected() {
    pingTimer->stop();
    emit disconnected();
}

void TwitchChat::onError(QAbstractSocket::SocketError error) {
    emit connectionError(socket->errorString());
}

void TwitchChat::handlePing() {
    socket->write("PING :tmi.twitch.tv\r\n");
}

void TwitchChat::joinChannel(const QString& channel) {
    QString ch = channel.toLower();
    if (!ch.startsWith("#")) {
        ch = "#" + ch;
    }
    
    socket->write(QString("JOIN %1\r\n").arg(ch).toUtf8());
    joinedChannels.append(ch);
    
    updateChannelInfo(ch.mid(1));
}

void TwitchChat::leaveChannel(const QString& channel) {
    QString ch = channel.toLower();
    if (!ch.startsWith("#")) {
        ch = "#" + ch;
    }
    
    socket->write(QString("PART %1\r\n").arg(ch).toUtf8());
    joinedChannels.removeAll(ch);
}

void TwitchChat::sendMessage(const QString& channel, const QString& message) {
    QString ch = channel.toLower();
    if (!ch.startsWith("#")) {
        ch = "#" + ch;
    }
    
    socket->write(QString("PRIVMSG %1 :%2\r\n").arg(ch).arg(message).toUtf8());
}

void TwitchChat::disconnect() {
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
}

void TwitchChat::onReadyRead() {
    while (socket->canReadLine()) {
        QString line = socket->readLine().trimmed();
        
        if (Settings::instance().showRawIrc) {
            emit rawMessage(line);
        }
        
        parseMessage(line);
    }
}

void TwitchChat::parseMessage(const QString& line) {
    if (line.startsWith("PING")) {
        socket->write("PONG :tmi.twitch.tv\r\n");
        return;
    }
    
    if (line.contains("PRIVMSG")) {
        ChatMessage msg = parsePrivMsg(line);
        
        QRegularExpression channelRe("#(\\w+)");
        QRegularExpressionMatch match = channelRe.match(line);
        if (match.hasMatch()) {
            QString channel = match.captured(1);
            emit messageReceived(channel, msg);
        }
    }
    
    if (line.contains("USERNOTICE")) {
        QRegularExpression msgRe("USERNOTICE #(\\w+) :(.*)");
        QRegularExpressionMatch match = msgRe.match(line);
        if (match.hasMatch()) {
            emit userNotice(match.captured(1), match.captured(2));
        }
    }
    
    if (line.contains("CLEARMSG")) {
        QRegularExpression idRe("target-msg-id=([^;\\s]+)");
        QRegularExpressionMatch match = idRe.match(line);
        if (match.hasMatch()) {
        }
    }
}

QMap<QString, QString> TwitchChat::parseTags(const QString& tags) {
    QMap<QString, QString> result;
    
    QStringList pairs = tags.split(";");
    for (const QString& pair : pairs) {
        int eq = pair.indexOf("=");
        if (eq > 0) {
            QString key = pair.left(eq);
            QString value = pair.mid(eq + 1);
            result[key] = value;
        }
    }
    
    return result;
}

ChatMessage TwitchChat::parsePrivMsg(const QString& line) {
    ChatMessage msg;
    
    QRegularExpression re("^@([^\\s]+) :(\\w+)!\\w+@\\w+\\.tmi\\.twitch\\.tv PRIVMSG #\\w+ :(.*)$");
    QRegularExpressionMatch match = re.match(line);
    
    if (!match.hasMatch()) {
        return msg;
    }
    
    QString tags = match.captured(1);
    msg.username = match.captured(2);
    msg.text = match.captured(3);
    msg.timestamp = QDateTime::currentDateTime();
    
    QMap<QString, QString> tagMap = parseTags(tags);
    
    msg.id = tagMap.value("id");
    msg.displayName = tagMap.value("display-name", msg.username);
    msg.userId = tagMap.value("user-id");
    
    QString colorStr = tagMap.value("color");
    if (!colorStr.isEmpty()) {
        msg.color = QColor(colorStr);
    } else {
        QStringList colors = {"#FF0000", "#0000FF", "#00FF00", "#B22222", "#FF7F50", 
                             "#9ACD32", "#FF4500", "#2E8B57", "#DAA520", "#D2691E",
                             "#5F9EA0", "#1E90FF", "#FF69B4", "#8A2BE2", "#00FF7F"};
        int hash = 0;
        for (QChar c : msg.username) {
            hash = hash * 31 + c.unicode();
        }
        msg.color = QColor(colors[qAbs(hash) % colors.size()]);
    }
    
    QString badgesStr = tagMap.value("badges");
    if (!badgesStr.isEmpty()) {
        QStringList badgePairs = badgesStr.split(",");
        for (const QString& badge : badgePairs) {
            msg.badges.append(badge.split("/")[0]);
        }
    }
    
    if (msg.text.startsWith("\x01" "ACTION")) {
        msg.isAction = true;
        msg.text = msg.text.mid(8);
        if (msg.text.endsWith("\x01")) {
            msg.text.chop(1);
        }
    }
    
    return msg;
}

void TwitchChat::updateChannelInfo(const QString& channel) {
    QUrl url(QString("https://api.twitch.tv/helix/streams?user_login=%1").arg(channel));
    QNetworkRequest req(url);
    req.setRawHeader("Authorization", QString("Bearer %1").arg(currentToken).toUtf8());
    req.setRawHeader("Client-Id", TWITCH_APP_CLIENT_ID.toUtf8());
    req.setProperty("channel", channel);
    
    QNetworkReply* reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, &TwitchChat::handleChannelInfoResponse);
}

void TwitchChat::handleChannelInfoResponse(QNetworkReply* reply) {
    QString channel = reply->property("channel").toString();
    
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    
    if (!doc.isObject()) return;
    
    QJsonArray data = doc.object()["data"].toArray();
    
    ChannelInfo info;
    info.channelName = channel;
    
    if (!data.isEmpty()) {
        QJsonObject stream = data[0].toObject();
        info.isLive = true;
        info.viewerCount = stream["viewer_count"].toInt();
        info.streamTitle = stream["title"].toString();
        info.streamCategory = stream["game_name"].toString();
    } else {
        info.isLive = false;
    }
    
    channelInfoCache[channel] = info;
    emit channelInfoUpdated(channel, info);
}

ChannelInfo TwitchChat::getChannelInfo(const QString& channel) {
    return channelInfoCache.value(channel, ChannelInfo());
}
