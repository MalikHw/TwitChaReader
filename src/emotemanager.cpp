#include "emotemanager.h"
#include "constants.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QImageReader>

EmoteManager& EmoteManager::instance() {
    static EmoteManager inst;
    return inst;
}

EmoteManager::EmoteManager() {
    nam = new QNetworkAccessManager(this);
    
    QString cachePath = getCachePath();
    QDir dir(cachePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString EmoteManager::getCachePath() {
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/emotes";
}

void EmoteManager::loadTwitchGlobalEmotes(const QString& token) {
    QUrl url("https://api.twitch.tv/helix/chat/emotes/global");
    QNetworkRequest req(url);
    req.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    req.setRawHeader("Client-Id", TWITCH_APP_CLIENT_ID.toUtf8());
    
    QNetworkReply* reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, &EmoteManager::handleTwitchEmotesResponse);
}

void EmoteManager::loadChannelEmotes(const QString& channelId, const QString& token) {
    QUrl url(QString("https://api.twitch.tv/helix/chat/emotes?broadcaster_id=%1").arg(channelId));
    QNetworkRequest req(url);
    req.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    req.setRawHeader("Client-Id", TWITCH_APP_CLIENT_ID.toUtf8());
    
    QNetworkReply* reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, &EmoteManager::handleTwitchEmotesResponse);
}

void EmoteManager::handleTwitchEmotesResponse(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    
    if (!doc.isObject()) return;
    
    QJsonArray data = doc.object()["data"].toArray();
    for (const auto& item : data) {
        QJsonObject emote = item.toObject();
        QString name = emote["name"].toString();
        QString id = emote["id"].toString();
        
        QJsonObject images = emote["images"].toObject();
        QString url = images["url_2x"].toString();
        if (url.isEmpty()) {
            url = images["url_1x"].toString();
        }
        
        QString format = emote["format"].toArray()[0].toString();
        bool animated = format == "animated";
        
        downloadEmote(name, url, animated);
    }
}

void EmoteManager::loadBTTVEmotes(const QString& channelName) {
    QUrl url(QString("https://api.betterttv.net/3/cached/users/twitch/%1").arg(channelName));
    QNetworkRequest req(url);
    
    QNetworkReply* reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, &EmoteManager::handleBTTVResponse);
}

void EmoteManager::handleBTTVResponse(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    
    if (!doc.isObject()) return;
    
    QJsonArray channelEmotes = doc.object()["channelEmotes"].toArray();
    QJsonArray sharedEmotes = doc.object()["sharedEmotes"].toArray();
    
    auto processEmotes = [this](const QJsonArray& arr) {
        for (const auto& item : arr) {
            QJsonObject emote = item.toObject();
            QString code = emote["code"].toString();
            QString id = emote["id"].toString();
            QString imageType = emote["imageType"].toString();
            
            QString url = QString("https://cdn.betterttv.net/emote/%1/2x").arg(id);
            bool animated = imageType == "gif";
            
            downloadEmote(code, url, animated);
        }
    };
    
    processEmotes(channelEmotes);
    processEmotes(sharedEmotes);
}

void EmoteManager::loadFFZEmotes(const QString& channelName) {
    QUrl url(QString("https://api.frankerfacez.com/v1/room/%1").arg(channelName));
    QNetworkRequest req(url);
    
    QNetworkReply* reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, &EmoteManager::handleFFZResponse);
}

void EmoteManager::handleFFZResponse(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    
    if (!doc.isObject()) return;
    
    QJsonObject sets = doc.object()["sets"].toObject();
    for (const QString& setId : sets.keys()) {
        QJsonArray emoticons = sets[setId].toObject()["emoticons"].toArray();
        for (const auto& item : emoticons) {
            QJsonObject emote = item.toObject();
            QString name = emote["name"].toString();
            QJsonObject urls = emote["urls"].toObject();
            
            QString url = urls["2"].toString();
            if (url.isEmpty()) {
                url = urls["1"].toString();
            }
            
            if (url.startsWith("//")) {
                url = "https:" + url;
            }
            
            downloadEmote(name, url, false);
        }
    }
}

void EmoteManager::load7TVEmotes(const QString& channelName) {
    QUrl url(QString("https://7tv.io/v3/users/twitch/%1").arg(channelName));
    QNetworkRequest req(url);
    
    QNetworkReply* reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, &EmoteManager::handle7TVResponse);
}

void EmoteManager::handle7TVResponse(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    
    if (!doc.isObject()) return;
    
    QJsonObject emoteSet = doc.object()["emote_set"].toObject();
    QJsonArray emotes = emoteSet["emotes"].toArray();
    
    for (const auto& item : emotes) {
        QJsonObject emote = item.toObject();
        QString name = emote["name"].toString();
        QString id = emote["id"].toString();
        
        QJsonObject data = emote["data"].toObject();
        bool animated = data["animated"].toBool();
        
        QString url = QString("https://cdn.7tv.app/emote/%1/2x.%2").arg(id).arg(animated ? "gif" : "webp");
        
        downloadEmote(name, url, animated);
    }
}

void EmoteManager::downloadEmote(const QString& name, const QString& url, bool animated) {
    if (emotes.contains(name)) {
        return;
    }
    
    QString cachePath = getCachePath();
    QString ext = animated ? "gif" : "png";
    QString filePath = QString("%1/%2.%3").arg(cachePath).arg(name).arg(ext);
    
    QFile file(filePath);
    if (file.exists()) {
        Emote* emote = new Emote();
        emote->name = name;
        emote->animated = animated;
        emote->url = url;
        
        if (animated) {
            emote->movie = new QMovie(filePath);
            emote->movie->start();
        } else {
            emote->pixmap.load(filePath);
        }
        
        emotes[name] = emote;
        emit emoteLoaded(name);
        return;
    }
    
    QNetworkRequest req(QUrl(url));
    QNetworkReply* reply = nam->get(req);
    pendingDownloads[reply] = name;
    connect(reply, &QNetworkReply::finished, this, &EmoteManager::handleEmoteDownload);
}

void EmoteManager::handleEmoteDownload(QNetworkReply* reply) {
    QString name = pendingDownloads.take(reply);
    
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    QString url = reply->url().toString();
    reply->deleteLater();
    
    bool animated = url.endsWith(".gif");
    
    QString cachePath = getCachePath();
    QString ext = animated ? "gif" : "png";
    QString filePath = QString("%1/%2.%3").arg(cachePath).arg(name).arg(ext);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
    }
    
    Emote* emote = new Emote();
    emote->name = name;
    emote->animated = animated;
    emote->url = url;
    
    if (animated) {
        emote->movie = new QMovie(filePath);
        emote->movie->start();
    } else {
        emote->pixmap.loadFromData(data);
    }
    
    emotes[name] = emote;
    emit emoteLoaded(name);
    emit emotesUpdated();
}

Emote* EmoteManager::getEmote(const QString& name) {
    return emotes.value(name, nullptr);
}

bool EmoteManager::hasEmote(const QString& name) {
    return emotes.contains(name);
}
