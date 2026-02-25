#ifndef TWITCHAUTH_H
#define TWITCHAUTH_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QTcpServer>

class TwitchAuth : public QObject {
    Q_OBJECT
    
public:
    explicit TwitchAuth(QObject* parent = nullptr);
    ~TwitchAuth();
    
    void startAuth();
    void logout();
    bool isAuthenticated();
    QString getAccessToken();
    QString getUsername();
    QString getUserId();
    void validateToken();
    void refreshAccessToken();
    
signals:
    void authenticated(const QString& username, const QString& userId);
    void authenticationFailed(const QString& error);
    void tokenValidated(bool valid);
    
private slots:
    void handleIncomingConnection();
    void handleAuthResponse(QNetworkReply* reply);
    void handleValidateResponse(QNetworkReply* reply);
    void handleRefreshResponse(QNetworkReply* reply);
    void handleUserInfoResponse(QNetworkReply* reply);
    
private:
    QNetworkAccessManager* nam;
    QTcpServer* localServer;
    QString clientId;
    QString clientSecret;
    int localPort;
    
    void fetchUserInfo(const QString& token);
};

#endif
