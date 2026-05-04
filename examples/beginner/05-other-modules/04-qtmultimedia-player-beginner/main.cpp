#include <QApplication>

#include "PlayerWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PlayerWindow window;
    window.show();

    return app.exec();
}
