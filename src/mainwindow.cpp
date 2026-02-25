#include "mainwindow.h"
#include "settings.h"
#include "emotemanager.h"
#include "notificationmanager.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QLabel>
#include <QStatusBar>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QFontDialog>
#include <QColorDialog>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("TwitChaReader");
    resize(1200, 800);
    
    auth = new TwitchAuth(this);
    chat = new TwitchChat(this);
    
    connect(auth, &TwitchAuth::authenticated, this, &MainWindow::onAuthenticated);
    connect(auth, &TwitchAuth::authenticationFailed, this, &MainWindow::onAuthFailed);
    connect(auth, &TwitchAuth::tokenValidated, this, &MainWindow::onTokenValidated);
    connect(chat, &TwitchChat::connected, this, &MainWindow::onChatConnected);
    connect(chat, &TwitchChat::disconnected, this, &MainWindow::onChatDisconnected);
    
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    chatTabs = new QTabWidget(this);
    chatTabs->setTabsClosable(true);
    connect(chatTabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        ChatWidget* widget = qobject_cast<ChatWidget*>(chatTabs->widget(index));
        if (widget) {
            chat->leaveChannel(widget->getChannel());
            chatWidgets.remove(widget->getChannel());
            chatTabs->removeTab(index);
            delete widget;
        }
    });
    
    mainSplitter->addWidget(chatTabs);
    
    statsWidget = new StatsWidget(this);
    statsWidget->setVisible(false);
    mainSplitter->addWidget(statsWidget);
    
    setCentralWidget(mainSplitter);
    
    createMenus();
    createTrayIcon();
    
    statusBar()->showMessage("Ready");
    
    Settings::instance().load();
    applySettings();
    
    if (auth->isAuthenticated()) {
        auth->validateToken();
    } else {
        showLoginDialog();
    }
}

MainWindow::~MainWindow() {
    Settings::instance().save();
}

void MainWindow::createMenus() {
    QMenu* accountMenu = menuBar()->addMenu("Account");
    QAction* loginAction = accountMenu->addAction("Login");
    connect(loginAction, &QAction::triggered, this, &MainWindow::showLoginDialog);
    QAction* logoutAction = accountMenu->addAction("Logout");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);
    
    QMenu* channelMenu = menuBar()->addMenu("Channel");
    QAction* joinAction = channelMenu->addAction("Join Channel...");
    joinAction->setShortcut(QKeySequence("Ctrl+J"));
    connect(joinAction, &QAction::triggered, this, &MainWindow::joinChannelDialog);
    QAction* leaveAction = channelMenu->addAction("Leave Channel");
    connect(leaveAction, &QAction::triggered, this, &MainWindow::leaveCurrentChannel);
    channelMenu->addSeparator();
    QAction* exportAction = channelMenu->addAction("Export Chat Log...");
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportCurrentChat);
    QAction* popOutAction = channelMenu->addAction("Pop Out Chat");
    connect(popOutAction, &QAction::triggered, this, &MainWindow::popOutCurrentChat);
    
    QMenu* viewMenu = menuBar()->addMenu("View");
    QAction* darkModeAction = viewMenu->addAction("Toggle Dark Mode");
    connect(darkModeAction, &QAction::triggered, this, &MainWindow::toggleDarkMode);
    QAction* compactAction = viewMenu->addAction("Toggle Compact Mode");
    connect(compactAction, &QAction::triggered, this, &MainWindow::toggleCompactMode);
    QAction* minimalAction = viewMenu->addAction("Toggle Minimal Mode");
    connect(minimalAction, &QAction::triggered, this, &MainWindow::toggleMinimalMode);
    QAction* transparentAction = viewMenu->addAction("Toggle Transparent Background");
    connect(transparentAction, &QAction::triggered, this, &MainWindow::toggleTransparentBg);
    viewMenu->addSeparator();
    QAction* increaseFontAction = viewMenu->addAction("Increase Font Size");
    increaseFontAction->setShortcut(QKeySequence("Ctrl++"));
    connect(increaseFontAction, &QAction::triggered, this, &MainWindow::increaseFontSize);
    QAction* decreaseFontAction = viewMenu->addAction("Decrease Font Size");
    decreaseFontAction->setShortcut(QKeySequence("Ctrl+-"));
    connect(decreaseFontAction, &QAction::triggered, this, &MainWindow::decreaseFontSize);
    viewMenu->addSeparator();
    QAction* splitViewAction = viewMenu->addAction("Toggle Split View");
    connect(splitViewAction, &QAction::triggered, this, &MainWindow::splitViewToggle);
    QAction* statsAction = viewMenu->addAction("Toggle Stats Panel");
    connect(statsAction, &QAction::triggered, this, &MainWindow::showStats);
    
    QMenu* windowMenu = menuBar()->addMenu("Window");
    QAction* alwaysOnTopAction = windowMenu->addAction("Always On Top");
    alwaysOnTopAction->setCheckable(true);
    connect(alwaysOnTopAction, &QAction::triggered, this, &MainWindow::toggleAlwaysOnTop);
    
    QMenu* toolsMenu = menuBar()->addMenu("Tools");
    QAction* filtersAction = toolsMenu->addAction("Filters & Highlights...");
    connect(filtersAction, &QAction::triggered, this, &MainWindow::showFilters);
    QAction* settingsAction = toolsMenu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
}

void MainWindow::createTrayIcon() {
    if (!Settings::instance().systemTray) {
        return;
    }
    
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icon.png"));
    
    trayMenu = new QMenu(this);
    QAction* showAction = trayMenu->addAction("Show");
    connect(showAction, &QAction::triggered, this, &MainWindow::show);
    QAction* quitAction = trayMenu->addAction("Quit");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);
    
    trayIcon->show();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        show();
        raise();
        activateWindow();
    }
}

void MainWindow::showLoginDialog() {
    auth->startAuth();
}

void MainWindow::onAuthenticated(const QString& username, const QString& userId) {
    statusBar()->showMessage(QString("Logged in as %1").arg(username));
    
    chat->connectToChat(auth->getAccessToken(), username);
    
    loadEmotes();
}

void MainWindow::onAuthFailed(const QString& error) {
    QMessageBox::critical(this, "Authentication Failed", error);
}

void MainWindow::onTokenValidated(bool valid) {
    if (valid) {
        chat->connectToChat(auth->getAccessToken(), auth->getUsername());
        loadEmotes();
    } else {
        showLoginDialog();
    }
}

void MainWindow::logout() {
    chat->disconnect();
    auth->logout();
    
    for (int i = chatTabs->count() - 1; i >= 0; --i) {
        ChatWidget* widget = qobject_cast<ChatWidget*>(chatTabs->widget(i));
        if (widget) {
            chatWidgets.remove(widget->getChannel());
            delete widget;
        }
    }
    chatTabs->clear();
    
    statusBar()->showMessage("Logged out");
}

void MainWindow::onChatConnected() {
    statusBar()->showMessage("Connected to Twitch chat");
}

void MainWindow::onChatDisconnected() {
    statusBar()->showMessage("Disconnected from Twitch chat");
}

void MainWindow::joinChannelDialog() {
    QString channel = QInputDialog::getText(this, "Join Channel", "Enter channel name:");
    
    if (channel.isEmpty()) {
        return;
    }
    
    channel = channel.toLower();
    
    if (chatWidgets.contains(channel)) {
        for (int i = 0; i < chatTabs->count(); ++i) {
            ChatWidget* widget = qobject_cast<ChatWidget*>(chatTabs->widget(i));
            if (widget && widget->getChannel() == channel) {
                chatTabs->setCurrentIndex(i);
                break;
            }
        }
        return;
    }
    
    ChatWidget* chatWidget = new ChatWidget(channel, chat, this);
    connect(chat, &TwitchChat::messageReceived, chatWidget, &ChatWidget::onMessageReceived);
    connect(chat, &TwitchChat::channelInfoUpdated, chatWidget, &ChatWidget::onChannelInfoUpdated);
    
    chatWidgets[channel] = chatWidget;
    chatTabs->addTab(chatWidget, channel);
    chatTabs->setCurrentWidget(chatWidget);
    
    chat->joinChannel(channel);
    
    EmoteManager::instance().loadBTTVEmotes(channel);
    EmoteManager::instance().loadFFZEmotes(channel);
    EmoteManager::instance().load7TVEmotes(channel);
}

void MainWindow::leaveCurrentChannel() {
    int index = chatTabs->currentIndex();
    if (index >= 0) {
        ChatWidget* widget = qobject_cast<ChatWidget*>(chatTabs->widget(index));
        if (widget) {
            chat->leaveChannel(widget->getChannel());
            chatWidgets.remove(widget->getChannel());
            chatTabs->removeTab(index);
            delete widget;
        }
    }
}

void MainWindow::loadEmotes() {
    EmoteManager::instance().loadTwitchGlobalEmotes(auth->getAccessToken());
}

void MainWindow::toggleDarkMode() {
    Settings::instance().darkMode = !Settings::instance().darkMode;
    Settings::instance().save();
    updateTheme();
}

void MainWindow::updateTheme() {
    QString bg = Settings::instance().darkMode ? "#2b2b2b" : "#ffffff";
    QString fg = Settings::instance().darkMode ? "#e0e0e0" : "#000000";
    
    setStyleSheet(QString(
        "QMainWindow { background-color: %1; color: %2; }"
        "QMenuBar { background-color: %1; color: %2; }"
        "QMenuBar::item:selected { background-color: #404040; }"
        "QMenu { background-color: %1; color: %2; }"
        "QMenu::item:selected { background-color: #404040; }"
        "QTabWidget::pane { border: 1px solid #333; }"
        "QTabBar::tab { background-color: %1; color: %2; padding: 8px; }"
        "QTabBar::tab:selected { background-color: #404040; }"
        "QStatusBar { background-color: %1; color: %2; }"
    ).arg(bg).arg(fg));
    
    for (ChatWidget* widget : chatWidgets) {
        widget->updateTheme();
    }
}

void MainWindow::applySettings() {
    updateTheme();
    
    if (Settings::instance().alwaysOnTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        show();
    }
    
    if (Settings::instance().startMinimized) {
        showMinimized();
    }
}

void MainWindow::showSettings() {
    QDialog dialog(this);
    dialog.setWindowTitle("Settings");
    QFormLayout* layout = new QFormLayout(&dialog);
    
    QCheckBox* darkModeCheck = new QCheckBox(&dialog);
    darkModeCheck->setChecked(Settings::instance().darkMode);
    layout->addRow("Dark Mode:", darkModeCheck);
    
    QCheckBox* timestampsCheck = new QCheckBox(&dialog);
    timestampsCheck->setChecked(Settings::instance().showTimestamps);
    layout->addRow("Show Timestamps:", timestampsCheck);
    
    QCheckBox* emotesCheck = new QCheckBox(&dialog);
    emotesCheck->setChecked(Settings::instance().showEmotes);
    layout->addRow("Show Emotes:", emotesCheck);
    
    QCheckBox* animatedCheck = new QCheckBox(&dialog);
    animatedCheck->setChecked(Settings::instance().animatedEmotes);
    layout->addRow("Animated Emotes:", animatedCheck);
    
    QSpinBox* fontSizeSpin = new QSpinBox(&dialog);
    fontSizeSpin->setRange(8, 32);
    fontSizeSpin->setValue(Settings::instance().fontSize);
    layout->addRow("Font Size:", fontSizeSpin);
    
    QSpinBox* emoteScaleSpin = new QSpinBox(&dialog);
    emoteScaleSpin->setRange(50, 200);
    emoteScaleSpin->setValue(Settings::instance().emoteScale);
    layout->addRow("Emote Scale (%):", emoteScaleSpin);
    
    QCheckBox* notifyMentionsCheck = new QCheckBox(&dialog);
    notifyMentionsCheck->setChecked(Settings::instance().notifyMentions);
    layout->addRow("Notify on Mentions:", notifyMentionsCheck);
    
    QCheckBox* soundAlertsCheck = new QCheckBox(&dialog);
    soundAlertsCheck->setChecked(Settings::instance().soundAlerts);
    layout->addRow("Sound Alerts:", soundAlertsCheck);
    
    QCheckBox* autoScrollCheck = new QCheckBox(&dialog);
    autoScrollCheck->setChecked(Settings::instance().autoScroll);
    layout->addRow("Auto-scroll:", autoScrollCheck);
    
    QCheckBox* lowCpuCheck = new QCheckBox(&dialog);
    lowCpuCheck->setChecked(Settings::instance().lowCpuMode);
    layout->addRow("Low CPU Mode:", lowCpuCheck);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addRow(buttons);
    
    if (dialog.exec() == QDialog::Accepted) {
        Settings::instance().darkMode = darkModeCheck->isChecked();
        Settings::instance().showTimestamps = timestampsCheck->isChecked();
        Settings::instance().showEmotes = emotesCheck->isChecked();
        Settings::instance().animatedEmotes = animatedCheck->isChecked();
        Settings::instance().fontSize = fontSizeSpin->value();
        Settings::instance().emoteScale = emoteScaleSpin->value();
        Settings::instance().notifyMentions = notifyMentionsCheck->isChecked();
        Settings::instance().soundAlerts = soundAlertsCheck->isChecked();
        Settings::instance().autoScroll = autoScrollCheck->isChecked();
        Settings::instance().lowCpuMode = lowCpuCheck->isChecked();
        Settings::instance().save();
        
        applySettings();
    }
}

void MainWindow::showFilters() {
    QDialog dialog(this);
    dialog.setWindowTitle("Filters & Highlights");
    dialog.resize(400, 500);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    FilterWidget* filterWidget = new FilterWidget(&dialog);
    layout->addWidget(filterWidget);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttons);
    
    dialog.exec();
}

void MainWindow::showStats() {
    statsWidget->setVisible(!statsWidget->isVisible());
}

void MainWindow::exportCurrentChat() {
    int index = chatTabs->currentIndex();
    if (index < 0) {
        return;
    }
    
    ChatWidget* widget = qobject_cast<ChatWidget*>(chatTabs->widget(index));
    if (!widget) {
        return;
    }
    
    QString filename = QFileDialog::getSaveFileName(
        this, "Export Chat Log", 
        QString("%1_chat.txt").arg(widget->getChannel()),
        "Text Files (*.txt)");
    
    if (!filename.isEmpty()) {
        widget->exportLog(filename);
    }
}

void MainWindow::toggleAlwaysOnTop() {
    Settings::instance().alwaysOnTop = !Settings::instance().alwaysOnTop;
    Settings::instance().save();
    
    Qt::WindowFlags flags = windowFlags();
    if (Settings::instance().alwaysOnTop) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    
    setWindowFlags(flags);
    show();
}

void MainWindow::toggleCompactMode() {
    Settings::instance().compactMode = !Settings::instance().compactMode;
    Settings::instance().save();
    updateTheme();
}

void MainWindow::toggleMinimalMode() {
    Settings::instance().minimalMode = !Settings::instance().minimalMode;
    Settings::instance().save();
    
    if (Settings::instance().minimalMode) {
        menuBar()->hide();
        statusBar()->hide();
    } else {
        menuBar()->show();
        statusBar()->show();
    }
}

void MainWindow::toggleTransparentBg() {
    Settings::instance().transparentBg = !Settings::instance().transparentBg;
    Settings::instance().save();
    
    if (Settings::instance().transparentBg) {
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    } else {
        setAttribute(Qt::WA_TranslucentBackground, false);
        setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);
    }
    
    show();
    updateTheme();
}

void MainWindow::increaseFontSize() {
    Settings::instance().fontSize += 2;
    if (Settings::instance().fontSize > 32) {
        Settings::instance().fontSize = 32;
    }
    Settings::instance().save();
    updateTheme();
}

void MainWindow::decreaseFontSize() {
    Settings::instance().fontSize -= 2;
    if (Settings::instance().fontSize < 8) {
        Settings::instance().fontSize = 8;
    }
    Settings::instance().save();
    updateTheme();
}

void MainWindow::popOutCurrentChat() {
    int index = chatTabs->currentIndex();
    if (index < 0) {
        return;
    }
    
    ChatWidget* widget = qobject_cast<ChatWidget*>(chatTabs->widget(index));
    if (!widget) {
        return;
    }
    
    QMainWindow* popOut = new QMainWindow();
    popOut->setWindowTitle(QString("TwitChaReader - %1").arg(widget->getChannel()));
    popOut->setCentralWidget(widget);
    popOut->resize(600, 800);
    popOut->show();
    
    chatTabs->removeTab(index);
}

void MainWindow::splitViewToggle() {
    splitViewMode = !splitViewMode;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (Settings::instance().systemTray && trayIcon && trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}
