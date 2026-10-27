#include <QApplication>

#include "CameraWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    CameraWindow window;
    window.show();

    return app.exec();
}
