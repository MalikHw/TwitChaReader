#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QSystemTrayIcon>
#include <QSplitter>
#include "twitchauth.h"
#include "twitchchat.h"
#include "chatwidget.h"
#include "statswidget.h"
#include "filterwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
protected:
    void closeEvent(QCloseEvent* event) override;
    
private slots:
    void onAuthenticated(const QString& username, const QString& userId);
    void onAuthFailed(const QString& error);
    void onChatConnected();
    void onChatDisconnected();
    void showLoginDialog();
    void logout();
    void joinChannelDialog();
    void leaveCurrentChannel();
    void toggleDarkMode();
    void showSettings();
    void showFilters();
    void showStats();
    void exportCurrentChat();
    void toggleAlwaysOnTop();
    void toggleCompactMode();
    void toggleMinimalMode();
    void toggleTransparentBg();
    void increaseFontSize();
    void decreaseFontSize();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void popOutCurrentChat();
    void splitViewToggle();
    void onTokenValidated(bool valid);
    
private:
    void createMenus();
    void createTrayIcon();
    void loadEmotes();
    void updateTheme();
    void applySettings();
    
    TwitchAuth* auth;
    TwitchChat* chat;
    QTabWidget* chatTabs;
    QSplitter* mainSplitter;
    StatsWidget* statsWidget;
    QSystemTrayIcon* trayIcon;
    QMenu* trayMenu;
    
    QMap<QString, ChatWidget*> chatWidgets;
    
    bool splitViewMode = false;
};

#endif
