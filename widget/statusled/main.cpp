#include "StatusLEDWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    StatusLEDWindow window;
    window.show();
    app.exec();
}