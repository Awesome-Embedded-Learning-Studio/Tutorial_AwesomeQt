/// @file    main.cpp
/// @brief   Entry point for the Qt Remote Objects advanced example.
///
/// Demonstrates in-process (local) communication between a source node and a
/// replica node. A SensorSource is hosted on a QRemoteObjectHostNode; a
/// ReplicaMonitor connects via "local:registry" and prints live updates.
/// The application auto-quits after approximately 5 seconds of simulation.

#include "replica_monitor.h"
#include "sensor_source.h"

#include <QCoreApplication>
#include <QRemoteObjectHost>
#include <QTimer>

#include <QDebug>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // -- 1. Create the source node with a local registry ----------------------
    // "local:registry" creates an in-process registry backed by QLocalSocket.
    // No TCP port or external network is needed.
    QRemoteObjectHost sourceNode(QUrl(QStringLiteral("local:registry")));

    // -- 2. Create and host the sensor source ---------------------------------
    auto* sensor = new SensorSource(&app);
    // The non-template overload introspects the QObject's Q_PROPERTY set
    // and registers it with the registry so replicas can discover it.
    sourceNode.enableRemoting(sensor);

    // -- 3. Create the replica monitor and connect ----------------------------
    ReplicaMonitor monitor(&app);
    monitor.connectToLocal();

    // -- 4. Start the sensor simulation ---------------------------------------
    // Delay start slightly so the replica has time to initialize.
    QTimer::singleShot(200, sensor, &SensorSource::start);

    // -- 5. Auto-quit after 5 seconds of simulated data -----------------------
    constexpr int kRunDurationMs = 5200;
    QTimer::singleShot(kRunDurationMs, &app, [&app]() {
        qDebug() << "";
        qDebug() << "[Main] Simulation complete — shutting down";
        app.quit();
    });

    qDebug() << "[Main] Qt Remote Objects — in-process source/replica demo";
    qDebug() << "[Main] Source node:  local:registry";
    qDebug() << "";

    return app.exec();
}
