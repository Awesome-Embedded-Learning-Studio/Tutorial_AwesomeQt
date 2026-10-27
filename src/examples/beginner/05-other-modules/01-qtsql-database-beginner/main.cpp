#include <QCoreApplication>

#include "Demo.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Demo demo;
    demo.run();

    return 0;
}
