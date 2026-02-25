#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class FilterWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit FilterWidget(QWidget* parent = nullptr);
    
private slots:
    void addMutedUser();
    void removeMutedUser();
    void addHighlightedUser();
    void removeHighlightedUser();
    void addKeyword();
    void removeKeyword();
    void saveFilters();
    
private:
    QListWidget* mutedUsersList;
    QLineEdit* mutedUserInput;
    QListWidget* highlightedUsersList;
    QLineEdit* highlightedUserInput;
    QListWidget* keywordsList;
    QLineEdit* keywordInput;
};

#endif
