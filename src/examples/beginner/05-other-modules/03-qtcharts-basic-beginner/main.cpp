#include <QApplication>

#include "Demo.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Demo demo;
    demo.run();

    return 0;
}
