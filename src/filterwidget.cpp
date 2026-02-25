#include "filterwidget.h"
#include "settings.h"
#include <QHBoxLayout>
#include <QGroupBox>

FilterWidget::FilterWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QGroupBox* mutedBox = new QGroupBox("Muted Users", this);
    QVBoxLayout* mutedLayout = new QVBoxLayout(mutedBox);
    mutedUsersList = new QListWidget(this);
    mutedUserInput = new QLineEdit(this);
    QPushButton* addMutedBtn = new QPushButton("Add", this);
    QPushButton* removeMutedBtn = new QPushButton("Remove", this);
    
    QHBoxLayout* mutedBtnLayout = new QHBoxLayout();
    mutedBtnLayout->addWidget(mutedUserInput);
    mutedBtnLayout->addWidget(addMutedBtn);
    mutedBtnLayout->addWidget(removeMutedBtn);
    
    mutedLayout->addWidget(mutedUsersList);
    mutedLayout->addLayout(mutedBtnLayout);
    
    for (const QString& user : Settings::instance().mutedUsers) {
        mutedUsersList->addItem(user);
    }
    
    connect(addMutedBtn, &QPushButton::clicked, this, &FilterWidget::addMutedUser);
    connect(removeMutedBtn, &QPushButton::clicked, this, &FilterWidget::removeMutedUser);
    
    QGroupBox* highlightBox = new QGroupBox("Highlighted Users", this);
    QVBoxLayout* highlightLayout = new QVBoxLayout(highlightBox);
    highlightedUsersList = new QListWidget(this);
    highlightedUserInput = new QLineEdit(this);
    QPushButton* addHighlightBtn = new QPushButton("Add", this);
    QPushButton* removeHighlightBtn = new QPushButton("Remove", this);
    
    QHBoxLayout* highlightBtnLayout = new QHBoxLayout();
    highlightBtnLayout->addWidget(highlightedUserInput);
    highlightBtnLayout->addWidget(addHighlightBtn);
    highlightBtnLayout->addWidget(removeHighlightBtn);
    
    highlightLayout->addWidget(highlightedUsersList);
    highlightLayout->addLayout(highlightBtnLayout);
    
    for (const QString& user : Settings::instance().highlightedUsers) {
        highlightedUsersList->addItem(user);
    }
    
    connect(addHighlightBtn, &QPushButton::clicked, this, &FilterWidget::addHighlightedUser);
    connect(removeHighlightBtn, &QPushButton::clicked, this, &FilterWidget::removeHighlightedUser);
    
    QGroupBox* keywordBox = new QGroupBox("Highlight Keywords", this);
    QVBoxLayout* keywordLayout = new QVBoxLayout(keywordBox);
    keywordsList = new QListWidget(this);
    keywordInput = new QLineEdit(this);
    QPushButton* addKeywordBtn = new QPushButton("Add", this);
    QPushButton* removeKeywordBtn = new QPushButton("Remove", this);
    
    QHBoxLayout* keywordBtnLayout = new QHBoxLayout();
    keywordBtnLayout->addWidget(keywordInput);
    keywordBtnLayout->addWidget(addKeywordBtn);
    keywordBtnLayout->addWidget(removeKeywordBtn);
    
    keywordLayout->addWidget(keywordsList);
    keywordLayout->addLayout(keywordBtnLayout);
    
    for (const QString& keyword : Settings::instance().highlightKeywords) {
        keywordsList->addItem(keyword);
    }
    
    connect(addKeywordBtn, &QPushButton::clicked, this, &FilterWidget::addKeyword);
    connect(removeKeywordBtn, &QPushButton::clicked, this, &FilterWidget::removeKeyword);
    
    mainLayout->addWidget(mutedBox);
    mainLayout->addWidget(highlightBox);
    mainLayout->addWidget(keywordBox);
    
    QPushButton* saveBtn = new QPushButton("Save Filters", this);
    connect(saveBtn, &QPushButton::clicked, this, &FilterWidget::saveFilters);
    mainLayout->addWidget(saveBtn);
}

void FilterWidget::addMutedUser() {
    QString user = mutedUserInput->text().trimmed().toLower();
    if (!user.isEmpty()) {
        mutedUsersList->addItem(user);
        mutedUserInput->clear();
    }
}

void FilterWidget::removeMutedUser() {
    qDeleteAll(mutedUsersList->selectedItems());
}

void FilterWidget::addHighlightedUser() {
    QString user = highlightedUserInput->text().trimmed().toLower();
    if (!user.isEmpty()) {
        highlightedUsersList->addItem(user);
        highlightedUserInput->clear();
    }
}

void FilterWidget::removeHighlightedUser() {
    qDeleteAll(highlightedUsersList->selectedItems());
}

void FilterWidget::addKeyword() {
    QString keyword = keywordInput->text().trimmed();
    if (!keyword.isEmpty()) {
        keywordsList->addItem(keyword);
        keywordInput->clear();
    }
}

void FilterWidget::removeKeyword() {
    qDeleteAll(keywordsList->selectedItems());
}

void FilterWidget::saveFilters() {
    Settings::instance().mutedUsers.clear();
    for (int i = 0; i < mutedUsersList->count(); ++i) {
        Settings::instance().mutedUsers.append(mutedUsersList->item(i)->text());
    }
    
    Settings::instance().highlightedUsers.clear();
    for (int i = 0; i < highlightedUsersList->count(); ++i) {
        Settings::instance().highlightedUsers.append(highlightedUsersList->item(i)->text());
    }
    
    Settings::instance().highlightKeywords.clear();
    for (int i = 0; i < keywordsList->count(); ++i) {
        Settings::instance().highlightKeywords.append(keywordsList->item(i)->text());
    }
    
    Settings::instance().save();
}
