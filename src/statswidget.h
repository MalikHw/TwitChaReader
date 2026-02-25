#ifndef STATSWIDGET_H
#define STATSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QMap>

class StatsWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit StatsWidget(QWidget* parent = nullptr);
    
    void addMessage(const QString& username);
    void addEmote(const QString& emote);
    void reset();
    
private slots:
    void updateDisplay();
    
private:
    QLabel* messagesPerMinLabel;
    QLabel* topUsersLabel;
    QLabel* topEmotesLabel;
    
    QTimer* updateTimer;
    QList<qint64> messageTimes;
    QMap<QString, int> userCounts;
    QMap<QString, int> emoteCounts;
};

#endif
