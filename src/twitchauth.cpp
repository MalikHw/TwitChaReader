#include "twitchauth.h"
#include "settings.h"
#include "constants.h"
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>

TwitchAuth::TwitchAuth(QObject* parent) : QObject(parent) {
    nam = new QNetworkAccessManager(this);
    localServer = nullptr;
    clientId = TWITCH_APP_CLIENT_ID;
    localPort = 3000;
}

TwitchAuth::~TwitchAuth() {
    if (localServer) {
        localServer->close();
        delete localServer;
    }
}

void TwitchAuth::startAuth() {
    if (localServer) {
        localServer->close();
        delete localServer;
    }
    
    localServer = new QTcpServer(this);
    if (!localServer->listen(QHostAddress::LocalHost, localPort)) {
        emit authenticationFailed("Failed to start local server");
        return;
    }
    
    connect(localServer, &QTcpServer::newConnection, this, &TwitchAuth::handleIncomingConnection);
    
    QString redirectUri = QString("http://localhost:%1").arg(localPort);
    QUrl authUrl("https://id.twitch.tv/oauth2/authorize");
    QUrlQuery query;
    query.addQueryItem("client_id", clientId);
    query.addQueryItem("redirect_uri", redirectUri);
    query.addQueryItem("response_type", "token");
    query.addQueryItem("scope", "chat:read chat:edit user:read:email user:read:follows");
    authUrl.setQuery(query);
    
    QDesktopServices::openUrl(authUrl);
}

void TwitchAuth::handleIncomingConnection() {
    QTcpSocket* socket = localServer->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        QString request = socket->readAll();
        
        QString response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "\r\n"
                          "<html><body><h1>Authentication successful!</h1>"
                          "<p>You can close this window now.</p>"
                          "<script>window.close();</script></body></html>";
        
        socket->write(response.toUtf8());
        socket->flush();
        socket->disconnectFromHost();
        
        int hashPos = request.indexOf("#access_token=");
        if (hashPos != -1) {
            int endPos = request.indexOf("&", hashPos);
            if (endPos == -1) endPos = request.indexOf(" ", hashPos);
            QString token = request.mid(hashPos + 14, endPos - hashPos - 14);
            
            Settings::instance().accessToken = token;
            Settings::instance().save();
            
            fetchUserInfo(token);
        }
    });
}

void TwitchAuth::fetchUserInfo(const QString& token) {
    QUrl url("https://api.twitch.tv/helix/users");
    QNetworkRequest req(url);
    req.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    req.setRawHeader("Client-Id", clientId.toUtf8());
    
    QNetworkReply* reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, &TwitchAuth::handleUserInfoResponse);
}

void TwitchAuth::handleUserInfoResponse(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit authenticationFailed(reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    
    if (doc.isNull() || !doc.isObject()) {
        emit authenticationFailed("Invalid response");
        return;
    }
    
    QJsonObject obj = doc.object();
    QJsonArray data = obj["data"].toArray();
    
    if (data.isEmpty()) {
        emit authenticationFailed("No user data");
        return;
    }
    
    QJsonObject user = data[0].toObject();
    QString username = user["login"].toString();
    QString userId = user["id"].toString();
    
    Settings::instance().username = username;
    Settings::instance().userId = userId;
    Settings::instance().save();
    
    emit authenticated(username, userId);
}

void TwitchAuth::validateToken() {
    if (Settings::instance().accessToken.isEmpty()) {
        emit tokenValidated(false);
        return;
    }
    
    QUrl url("https://id.twitch.tv/oauth2/validate");
    QNetworkRequest req(url);
    req.setRawHeader("Authorization", QString("OAuth %1").arg(Settings::instance().accessToken).toUtf8());
    
    QNetworkReply* reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, &TwitchAuth::handleValidateResponse);
}

void TwitchAuth::handleValidateResponse(QNetworkReply* reply) {
    bool valid = (reply->error() == QNetworkReply::NoError);
    emit tokenValidated(valid);
    reply->deleteLater();
}

void TwitchAuth::logout() {
    Settings::instance().accessToken.clear();
    Settings::instance().refreshToken.clear();
    Settings::instance().username.clear();
    Settings::instance().userId.clear();
    Settings::instance().save();
}

bool TwitchAuth::isAuthenticated() {
    return !Settings::instance().accessToken.isEmpty();
}

QString TwitchAuth::getAccessToken() {
    return Settings::instance().accessToken;
}

QString TwitchAuth::getUsername() {
    return Settings::instance().username;
}

QString TwitchAuth::getUserId() {
    return Settings::instance().userId;
}

void TwitchAuth::refreshAccessToken() {
}
