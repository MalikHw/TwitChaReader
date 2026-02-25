#include "notificationmanager.h"
#include "settings.h"
#include <QApplication>
#include <QSound>

NotificationManager& NotificationManager::instance() {
    static NotificationManager inst;
    return inst;
}

NotificationManager::NotificationManager() {
    trayIcon = new QSystemTrayIcon(qApp);
    trayIcon->setIcon(QIcon(":/icon.png"));
}

void NotificationManager::showNotification(const QString& title, const QString& message) {
    if (trayIcon) {
        trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 3000);
    }
}

void NotificationManager::playSound() {
    if (Settings::instance().soundAlerts) {
    }
}
