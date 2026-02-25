#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include "chatmessage.h"
#include "twitchchat.h"

class ChatWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ChatWidget(const QString& channel, TwitchChat* chat, QWidget* parent = nullptr);
    
    void addMessage(const ChatMessage& msg);
    void clear();
    void updateTheme();
    void exportLog(const QString& filepath);
    QString getChannel() const { return channelName; }
    
public slots:
    void onMessageReceived(const QString& channel, const ChatMessage& msg);
    void onChannelInfoUpdated(const QString& channel, const ChannelInfo& info);
    void togglePause();
    void toggleFreeze();
    
private:
    QString channelName;
    QTextEdit* chatDisplay;
    QLabel* infoLabel;
    TwitchChat* chatConnection;
    QList<ChatMessage> messageBuffer;
    bool paused = false;
    bool frozen = false;
    int messageCount = 0;
    QTimer* statsTimer;
    QMap<QString, int> userMessageCounts;
    QMap<QString, int> emoteCounts;
    
    QString formatMessage(const ChatMessage& msg);
    void updateStats();
    bool shouldHighlight(const ChatMessage& msg);
};

#endif
