/// @file    main.cpp
/// @brief   Application entry point — starts the server and test client.
///
/// Corresponds to tutorial: advanced 05-other-modules/20-qtwebchannel.
///
/// This program:
///   1. Starts a ChannelServer on port 12345.
///   2. Connects to the backend directly via C++ signals for logging.
///   3. Creates a TestClient (WebSocket) that performs the WebChannel
///      handshake and invokes methods on the published backend object.
///   4. Auto-quits after the demo completes.

#include "channel_server.h"
#include "chat_backend.h"
#include "test_client.h"

#include <QCoreApplication>
#include <QDebug>
#include <QUrl>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // Start the WebChannel server on the designated port.
    ChannelServer server;
    if (!server.start(12345)) {
        qCritical() << "Failed to start server, exiting";
        return 1;
    }

    // Demonstrate direct C++ signal connection alongside the WebChannel path.
    // This shows that the same backend object can serve both C++ local
    // connections and remote WebSocket clients simultaneously.
    QObject::connect(server.backend(), &ChatBackend::messageReceived,
                     [](const QString& from, const QString& text) {
                         qDebug() << "[Direct C++ signal]" << from << ":" << text;
                     });

    // Launch the test client that speaks the WebChannel protocol over
    // WebSocket. It runs in the same event loop — sufficient for a demo.
    TestClient client(QUrl("ws://localhost:12345"));

    return app.exec();
}
