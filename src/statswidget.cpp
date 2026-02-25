#include "statswidget.h"
#include <QDateTime>
#include <algorithm>

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    messagesPerMinLabel = new QLabel("Messages/min: 0", this);
    topUsersLabel = new QLabel("Top Users:", this);
    topEmotesLabel = new QLabel("Top Emotes:", this);
    
    layout->addWidget(messagesPerMinLabel);
    layout->addWidget(topUsersLabel);
    layout->addWidget(topEmotesLabel);
    layout->addStretch();
    
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &StatsWidget::updateDisplay);
    updateTimer->start(1000);
}

void StatsWidget::addMessage(const QString& username) {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    messageTimes.append(now);
    
    userCounts[username]++;
    
    while (!messageTimes.isEmpty() && now - messageTimes.first() > 60000) {
        messageTimes.removeFirst();
    }
}

void StatsWidget::addEmote(const QString& emote) {
    emoteCounts[emote]++;
}

void StatsWidget::updateDisplay() {
    messagesPerMinLabel->setText(QString("Messages/min: %1").arg(messageTimes.size()));
    
    QList<QPair<QString, int>> userList;
    for (auto it = userCounts.begin(); it != userCounts.end(); ++it) {
        userList.append(qMakePair(it.key(), it.value()));
    }
    
    std::sort(userList.begin(), userList.end(), 
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
        return a.second > b.second;
    });
    
    QString topUsers = "Top Users:\n";
    for (int i = 0; i < qMin(5, userList.size()); ++i) {
        topUsers += QString("%1: %2\n").arg(userList[i].first).arg(userList[i].second);
    }
    topUsersLabel->setText(topUsers);
    
    QList<QPair<QString, int>> emoteList;
    for (auto it = emoteCounts.begin(); it != emoteCounts.end(); ++it) {
        emoteList.append(qMakePair(it.key(), it.value()));
    }
    
    std::sort(emoteList.begin(), emoteList.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
        return a.second > b.second;
    });
    
    QString topEmotes = "Top Emotes:\n";
    for (int i = 0; i < qMin(5, emoteList.size()); ++i) {
        topEmotes += QString("%1: %2\n").arg(emoteList[i].first).arg(emoteList[i].second);
    }
    topEmotesLabel->setText(topEmotes);
}

void StatsWidget::reset() {
    messageTimes.clear();
    userCounts.clear();
    emoteCounts.clear();
    updateDisplay();
}
