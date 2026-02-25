#include "userprofile.h"

UserProfile::UserProfile(const QString& username, QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_DeleteOnClose);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    usernameLabel = new QLabel(username, this);
    usernameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    
    avatarLabel = new QLabel(this);
    avatarLabel->setFixedSize(64, 64);
    
    infoLabel = new QLabel("Loading...", this);
    
    layout->addWidget(usernameLabel);
    layout->addWidget(avatarLabel);
    layout->addWidget(infoLabel);
    
    setFixedSize(200, 150);
}
