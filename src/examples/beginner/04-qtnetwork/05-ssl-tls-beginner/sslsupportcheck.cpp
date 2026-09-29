#include "sslsupportcheck.h"

#include <QDebug>
#include <QSslSocket>

void checkSslSupport()
{
    qDebug() << "\n=== Demo 1: SSL Support Check ===";

    qDebug() << "  SSL supported:"
             << QSslSocket::supportsSsl();
    qDebug() << "  SSL runtime version:"
             << QSslSocket::sslLibraryVersionString();
    qDebug() << "  SSL build version:"
             << QSslSocket::sslLibraryBuildVersionString();

    if (!QSslSocket::supportsSsl()) {
        qDebug() << "\n  WARNING: SSL is NOT supported on this system!";
        qDebug() << "  On Linux, install OpenSSL runtime libraries.";
        qDebug() << "  On Windows, Qt uses Schannel (built-in).";
    }
}
