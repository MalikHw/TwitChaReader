#include "settings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>

Settings& Settings::instance() {
    static Settings inst;
    return inst;
}

QString Settings::configPath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return path + "/config.json";
}

void Settings::load() {
    QFile file(configPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull() || !doc.isObject()) {
        return;
    }
    
    QJsonObject obj = doc.object();
    
    accessToken = obj["accessToken"].toString();
    refreshToken = obj["refreshToken"].toString();
    username = obj["username"].toString();
    userId = obj["userId"].toString();
    
    QJsonArray recent = obj["recentChannels"].toArray();
    recentChannels.clear();
    for (const auto& ch : recent) {
        recentChannels.append(ch.toString());
    }
    
    QJsonArray muted = obj["mutedUsers"].toArray();
    mutedUsers.clear();
    for (const auto& u : muted) {
        mutedUsers.append(u.toString());
    }
    
    QJsonArray highlighted = obj["highlightedUsers"].toArray();
    highlightedUsers.clear();
    for (const auto& u : highlighted) {
        highlightedUsers.append(u.toString());
    }
    
    QJsonArray keywords = obj["highlightKeywords"].toArray();
    highlightKeywords.clear();
    for (const auto& k : keywords) {
        highlightKeywords.append(k.toString());
    }
    
    QJsonArray followed = obj["followedChannels"].toArray();
    followedChannels.clear();
    for (const auto& f : followed) {
        followedChannels.append(f.toString());
    }
    
    darkMode = obj["darkMode"].toBool(true);
    showTimestamps = obj["showTimestamps"].toBool(true);
    showDeletedMessages = obj["showDeletedMessages"].toBool(false);
    showEmotes = obj["showEmotes"].toBool(true);
    animatedEmotes = obj["animatedEmotes"].toBool(true);
    compactMode = obj["compactMode"].toBool(false);
    minimalMode = obj["minimalMode"].toBool(false);
    transparentBg = obj["transparentBg"].toBool(false);
    alwaysOnTop = obj["alwaysOnTop"].toBool(false);
    startMinimized = obj["startMinimized"].toBool(false);
    autoStart = obj["autoStart"].toBool(false);
    lowCpuMode = obj["lowCpuMode"].toBool(false);
    notifyMentions = obj["notifyMentions"].toBool(true);
    notifyStreamLive = obj["notifyStreamLive"].toBool(false);
    soundAlerts = obj["soundAlerts"].toBool(false);
    autoScroll = obj["autoScroll"].toBool(true);
    showViewerCount = obj["showViewerCount"].toBool(true);
    showStreamInfo = obj["showStreamInfo"].toBool(true);
    highlightOwnMessages = obj["highlightOwnMessages"].toBool(true);
    showRawIrc = obj["showRawIrc"].toBool(false);
    systemTray = obj["systemTray"].toBool(true);
    
    fontSize = obj["fontSize"].toInt(12);
    emoteScale = obj["emoteScale"].toInt(100);
    chatOpacity = obj["chatOpacity"].toInt(100);
    messageRateLimit = obj["messageRateLimit"].toInt(500);
    customFont = obj["customFont"].toString("Segoe UI");
    theme = obj["theme"].toString("dark");
}

void Settings::save() {
    QJsonObject obj;
    
    obj["accessToken"] = accessToken;
    obj["refreshToken"] = refreshToken;
    obj["username"] = username;
    obj["userId"] = userId;
    
    QJsonArray recent;
    for (const auto& ch : recentChannels) {
        recent.append(ch);
    }
    obj["recentChannels"] = recent;
    
    QJsonArray muted;
    for (const auto& u : mutedUsers) {
        muted.append(u);
    }
    obj["mutedUsers"] = muted;
    
    QJsonArray highlighted;
    for (const auto& u : highlightedUsers) {
        highlighted.append(u);
    }
    obj["highlightedUsers"] = highlighted;
    
    QJsonArray keywords;
    for (const auto& k : highlightKeywords) {
        keywords.append(k);
    }
    obj["highlightKeywords"] = keywords;
    
    QJsonArray followed;
    for (const auto& f : followedChannels) {
        followed.append(f);
    }
    obj["followedChannels"] = followed;
    
    obj["darkMode"] = darkMode;
    obj["showTimestamps"] = showTimestamps;
    obj["showDeletedMessages"] = showDeletedMessages;
    obj["showEmotes"] = showEmotes;
    obj["animatedEmotes"] = animatedEmotes;
    obj["compactMode"] = compactMode;
    obj["minimalMode"] = minimalMode;
    obj["transparentBg"] = transparentBg;
    obj["alwaysOnTop"] = alwaysOnTop;
    obj["startMinimized"] = startMinimized;
    obj["autoStart"] = autoStart;
    obj["lowCpuMode"] = lowCpuMode;
    obj["notifyMentions"] = notifyMentions;
    obj["notifyStreamLive"] = notifyStreamLive;
    obj["soundAlerts"] = soundAlerts;
    obj["autoScroll"] = autoScroll;
    obj["showViewerCount"] = showViewerCount;
    obj["showStreamInfo"] = showStreamInfo;
    obj["highlightOwnMessages"] = highlightOwnMessages;
    obj["showRawIrc"] = showRawIrc;
    obj["systemTray"] = systemTray;
    
    obj["fontSize"] = fontSize;
    obj["emoteScale"] = emoteScale;
    obj["chatOpacity"] = chatOpacity;
    obj["messageRateLimit"] = messageRateLimit;
    obj["customFont"] = customFont;
    obj["theme"] = theme;
    
    QFile file(configPath());
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    
    file.write(QJsonDocument(obj).toJson());
    file.close();
}
