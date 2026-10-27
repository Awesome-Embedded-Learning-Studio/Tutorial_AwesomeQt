#include <QApplication>

#include "Database.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!initDatabase()) {
        return 1;
    }

    MainWindow window;
    window.show();

    return app.exec();
}
