#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QJsonObject>
#include <QStringList>
#include <QFont>
#include <QColor>

class Settings {
public:
    static Settings& instance();
    
    void load();
    void save();
    
    QString accessToken;
    QString refreshToken;
    QString username;
    QString userId;
    QStringList recentChannels;
    QStringList mutedUsers;
    QStringList highlightedUsers;
    QStringList highlightKeywords;
    
    bool darkMode = true;
    bool showTimestamps = true;
    bool showDeletedMessages = false;
    bool showEmotes = true;
    bool animatedEmotes = true;
    bool compactMode = false;
    bool minimalMode = false;
    bool transparentBg = false;
    bool alwaysOnTop = false;
    bool startMinimized = false;
    bool autoStart = false;
    bool lowCpuMode = false;
    bool notifyMentions = true;
    bool notifyStreamLive = false;
    bool soundAlerts = false;
    bool autoScroll = true;
    bool showViewerCount = true;
    bool showStreamInfo = true;
    bool highlightOwnMessages = true;
    bool showRawIrc = false;
    
    int fontSize = 12;
    int emoteScale = 100;
    int chatOpacity = 100;
    int messageRateLimit = 500;
    QString customFont = "Segoe UI";
    
    QString theme = "dark";
    bool systemTray = true;
    
    QStringList followedChannels;
    
private:
    Settings() = default;
    QString configPath();
};

#endif
