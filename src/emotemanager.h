#ifndef EMOTEMANAGER_H
#define EMOTEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QMap>
#include <QNetworkAccessManager>
#include <QMovie>

struct Emote {
    QString id;
    QString name;
    QString url;
    QPixmap pixmap;
    QMovie* movie = nullptr;
    bool animated = false;
    int width = 28;
    int height = 28;
};

class EmoteManager : public QObject {
    Q_OBJECT
    
public:
    static EmoteManager& instance();
    
    void loadTwitchGlobalEmotes(const QString& token);
    void loadChannelEmotes(const QString& channelId, const QString& token);
    void loadBTTVEmotes(const QString& channelId);
    void loadFFZEmotes(const QString& channelId);
    void load7TVEmotes(const QString& channelId);
    
    Emote* getEmote(const QString& name);
    bool hasEmote(const QString& name);
    
    void downloadEmote(const QString& name, const QString& url, bool animated);
    QString getCachePath();
    
signals:
    void emoteLoaded(const QString& name);
    void emotesUpdated();
    
private slots:
    void handleEmoteDownload(QNetworkReply* reply);
    void handleTwitchEmotesResponse(QNetworkReply* reply);
    void handleBTTVResponse(QNetworkReply* reply);
    void handleFFZResponse(QNetworkReply* reply);
    void handle7TVResponse(QNetworkReply* reply);
    
private:
    EmoteManager();
    QNetworkAccessManager* nam;
    QMap<QString, Emote*> emotes;
    QMap<QNetworkReply*, QString> pendingDownloads;
};

#endif
