#ifndef TWITCHCHAT_H
#define TWITCHCHAT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "chatmessage.h"
#include <QNetworkAccessManager>

struct ChannelInfo {
    QString channelName;
    QString channelId;
    int viewerCount = 0;
    QString streamTitle;
    QString streamCategory;
    bool isLive = false;
};

class TwitchChat : public QObject {
    Q_OBJECT
    
public:
    explicit TwitchChat(QObject* parent = nullptr);
    ~TwitchChat();
    
    void connectToChat(const QString& token, const QString& username);
    void joinChannel(const QString& channel);
    void leaveChannel(const QString& channel);
    void sendMessage(const QString& channel, const QString& message);
    void disconnect();
    
    ChannelInfo getChannelInfo(const QString& channel);
    void updateChannelInfo(const QString& channel);
    
signals:
    void connected();
    void disconnected();
    void messageReceived(const QString& channel, const ChatMessage& msg);
    void userNotice(const QString& channel, const QString& message);
    void channelInfoUpdated(const QString& channel, const ChannelInfo& info);
    void connectionError(const QString& error);
    void rawMessage(const QString& message);
    
private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void handlePing();
    void handleChannelInfoResponse(QNetworkReply* reply);
    
private:
    QTcpSocket* socket;
    QTimer* pingTimer;
    QString currentToken;
    QString currentUsername;
    QStringList joinedChannels;
    QMap<QString, ChannelInfo> channelInfoCache;
    QNetworkAccessManager* nam;
    
    void parseMessage(const QString& line);
    ChatMessage parsePrivMsg(const QString& line);
    QMap<QString, QString> parseTags(const QString& tags);
};

#endif
