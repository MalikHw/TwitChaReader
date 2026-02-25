#include "chatwidget.h"
#include "settings.h"
#include "emotemanager.h"
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextImageFormat>

ChatWidget::ChatWidget(const QString& channel, TwitchChat* chat, QWidget* parent)
    : QWidget(parent), channelName(channel), chatConnection(chat) {
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    infoLabel = new QLabel(this);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);
    
    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);
    chatDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    layout->addWidget(chatDisplay);
    
    updateTheme();
    
    statsTimer = new QTimer(this);
    connect(statsTimer, &QTimer::timeout, this, &ChatWidget::updateStats);
    statsTimer->start(5000);
}

void ChatWidget::onMessageReceived(const QString& channel, const ChatMessage& msg) {
    if (channel.toLower() != channelName.toLower()) {
        return;
    }
    
    if (Settings::instance().mutedUsers.contains(msg.username.toLower())) {
        return;
    }
    
    if (frozen) {
        messageBuffer.append(msg);
        return;
    }
    
    addMessage(msg);
    
    userMessageCounts[msg.username]++;
    
    for (const QString& word : msg.text.split(" ")) {
        if (EmoteManager::instance().hasEmote(word)) {
            emoteCounts[word]++;
        }
    }
}

void ChatWidget::addMessage(const ChatMessage& msg) {
    QString html = formatMessage(msg);
    
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    bool atBottom = chatDisplay->verticalScrollBar()->value() == 
                    chatDisplay->verticalScrollBar()->maximum();
    
    cursor.insertHtml(html);
    
    if (Settings::instance().autoScroll && atBottom) {
        chatDisplay->verticalScrollBar()->setValue(
            chatDisplay->verticalScrollBar()->maximum()
        );
    }
    
    messageCount++;
    
    if (Settings::instance().lowCpuMode && messageCount > 500) {
        QTextCursor clearCursor(chatDisplay->document());
        clearCursor.movePosition(QTextCursor::Start);
        clearCursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 100);
        clearCursor.removeSelectedText();
        messageCount -= 100;
    }
}

QString ChatWidget::formatMessage(const ChatMessage& msg) {
    QString html = "<div style='margin:2px 0;";
    
    if (shouldHighlight(msg)) {
        html += "background-color: rgba(255, 255, 0, 0.2);";
    }
    
    if (Settings::instance().compactMode) {
        html += "line-height: 1.2;";
    }
    
    html += "'>";
    
    if (Settings::instance().showTimestamps) {
        QString time = msg.timestamp.toString("hh:mm:ss");
        html += QString("<span style='color: #999; font-size: 10px;'>%1</span> ").arg(time);
    }
    
    for (const QString& badge : msg.badges) {
        if (badge == "moderator") {
            html += "<span style='color: #00ff00; font-weight: bold;'>[M]</span> ";
        } else if (badge == "vip") {
            html += "<span style='color: #ff00ff; font-weight: bold;'>[VIP]</span> ";
        } else if (badge.startsWith("subscriber")) {
            html += "<span style='color: #ff0000; font-weight: bold;'>[SUB]</span> ";
        } else if (badge == "broadcaster") {
            html += "<span style='color: #ffa500; font-weight: bold;'>[BROADCASTER]</span> ";
        }
    }
    
    QString nameColor = msg.color.name();
    html += QString("<span style='color: %1; font-weight: bold;'>%2</span>: ")
            .arg(nameColor)
            .arg(msg.displayName.isEmpty() ? msg.username : msg.displayName);
    
    QString text = msg.text;
    text.replace("<", "&lt;").replace(">", "&gt;");
    
    if (Settings::instance().showEmotes) {
        QStringList words = text.split(" ");
        QStringList processedWords;
        
        for (const QString& word : words) {
            if (EmoteManager::instance().hasEmote(word)) {
                Emote* emote = EmoteManager::instance().getEmote(word);
                if (emote) {
                    int scale = Settings::instance().emoteScale;
                    int width = emote->width * scale / 100;
                    int height = emote->height * scale / 100;
                    
                    processedWords.append(QString("<img src='emote:%1' width='%2' height='%3' title='%4'/>")
                                        .arg(word)
                                        .arg(width)
                                        .arg(height)
                                        .arg(word));
                } else {
                    processedWords.append(word);
                }
            } else {
                processedWords.append(word);
            }
        }
        
        text = processedWords.join(" ");
    }
    
    if (msg.isAction) {
        html += QString("<span style='color: %1; font-style: italic;'>%2</span>")
                .arg(nameColor)
                .arg(text);
    } else {
        html += QString("<span>%1</span>").arg(text);
    }
    
    if (msg.deleted && Settings::instance().showDeletedMessages) {
        html += " <span style='color: #ff0000; font-size: 10px;'>[DELETED]</span>";
    }
    
    html += "</div>";
    
    return html;
}

bool ChatWidget::shouldHighlight(const ChatMessage& msg) {
    if (Settings::instance().highlightedUsers.contains(msg.username.toLower())) {
        return true;
    }
    
    QString lowerText = msg.text.toLower();
    for (const QString& keyword : Settings::instance().highlightKeywords) {
        if (lowerText.contains(keyword.toLower())) {
            return true;
        }
    }
    
    if (Settings::instance().highlightOwnMessages && 
        msg.username.toLower() == Settings::instance().username.toLower()) {
        return true;
    }
    
    return false;
}

void ChatWidget::onChannelInfoUpdated(const QString& channel, const ChannelInfo& info) {
    if (channel.toLower() != channelName.toLower()) {
        return;
    }
    
    QString infoText;
    
    if (Settings::instance().showStreamInfo) {
        if (info.isLive) {
            infoText += QString("<b>%1</b>").arg(info.streamTitle);
            if (!info.streamCategory.isEmpty()) {
                infoText += QString(" - %1").arg(info.streamCategory);
            }
        } else {
            infoText += "<i>Stream Offline</i>";
        }
    }
    
    if (Settings::instance().showViewerCount && info.isLive) {
        infoText += QString(" | Viewers: %1").arg(info.viewerCount);
    }
    
    infoLabel->setText(infoText);
}

void ChatWidget::updateTheme() {
    QString bg = Settings::instance().darkMode ? "#1e1e1e" : "#ffffff";
    QString fg = Settings::instance().darkMode ? "#e0e0e0" : "#000000";
    
    if (Settings::instance().transparentBg) {
        int opacity = Settings::instance().chatOpacity;
        bg = QString("rgba(30, 30, 30, %1)").arg(opacity / 100.0);
    }
    
    chatDisplay->setStyleSheet(QString(
        "QTextEdit { background-color: %1; color: %2; border: none; font-size: %3px; font-family: '%4'; }"
    ).arg(bg).arg(fg).arg(Settings::instance().fontSize).arg(Settings::instance().customFont));
    
    infoLabel->setStyleSheet(QString(
        "QLabel { background-color: %1; color: %2; padding: 5px; border-bottom: 1px solid #333; }"
    ).arg(bg).arg(fg));
}

void ChatWidget::togglePause() {
    paused = !paused;
}

void ChatWidget::toggleFreeze() {
    frozen = !frozen;
    
    if (!frozen) {
        for (const ChatMessage& msg : messageBuffer) {
            addMessage(msg);
        }
        messageBuffer.clear();
    }
}

void ChatWidget::clear() {
    chatDisplay->clear();
    messageCount = 0;
}

void ChatWidget::updateStats() {
}

void ChatWidget::exportLog(const QString& filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream out(&file);
    out << chatDisplay->toPlainText();
    file.close();
}
