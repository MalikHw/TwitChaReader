#include "mainwindow.h"
#include "settings.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setOrganizationName("TwitChaReader");
    app.setApplicationName("TwitChaReader");
    app.setApplicationVersion("1.0.0");
    
    Settings::instance().load();
    
    MainWindow window;
    
    if (!Settings::instance().startMinimized) {
        window.show();
    }
    
    return app.exec();
}
