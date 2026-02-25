#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>

class NotificationManager : public QObject {
    Q_OBJECT
    
public:
    static NotificationManager& instance();
    
    void showNotification(const QString& title, const QString& message);
    void playSound();
    
private:
    NotificationManager();
    QSystemTrayIcon* trayIcon;
};

#endif
