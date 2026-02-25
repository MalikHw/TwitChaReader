#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class UserProfile : public QWidget {
    Q_OBJECT
    
public:
    explicit UserProfile(const QString& username, QWidget* parent = nullptr);
    
private:
    QLabel* usernameLabel;
    QLabel* avatarLabel;
    QLabel* infoLabel;
};

#endif
