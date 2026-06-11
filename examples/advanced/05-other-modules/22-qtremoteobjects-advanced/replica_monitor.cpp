/// @file    replica_monitor.cpp
/// @brief   Implementation of the ReplicaMonitor that subscribes to a remote
///          SensorSource and prints incoming updates.

#include "replica_monitor.h"

#include <QRemoteObjectReplica>

#include <QDebug>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ReplicaMonitor::ReplicaMonitor(QObject* parent)
    : QObject(parent)
    , m_replica(nullptr)
{
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void ReplicaMonitor::connectToLocal()
{
    // "local:" scheme uses QLocalSocket under the hood — no TCP needed.
    // The registry URL must match the one used by QRemoteObjectHostNode.
    m_node.connectToNode(QUrl(QStringLiteral("local:registry")));

    // acquire() returns a replica pointer managed by the node.
    // Do NOT call delete on it.
    m_replica = m_node.acquire<SensorSourceReplica>();

    // Wait until the replica has received the full initial state from the
    // source before wiring up property-change listeners. Connecting earlier
    // is safe but would fire with default-constructed values.
    QObject::connect(
        m_replica, &SensorSourceReplica::initialized, this,
        [this]() {
            qDebug() << "[Replica] Initialized — monitoring sensor updates";

            // Wire property-change signals from the generated replica
            connect(m_replica, &SensorSourceReplica::temperatureChanged,
                    this, &ReplicaMonitor::onTemperatureChanged);
            connect(m_replica, &SensorSourceReplica::humidityChanged,
                    this, &ReplicaMonitor::onHumidityChanged);
            connect(m_replica, &SensorSourceReplica::statusChanged,
                    this, &ReplicaMonitor::onStatusChanged);

            emit replicaInitialized();
        });
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void ReplicaMonitor::onTemperatureChanged(double value)
{
    qDebug() << "[Replica] Temperature:" << value << "C";
}

void ReplicaMonitor::onHumidityChanged(double value)
{
    qDebug() << "[Replica] Humidity:   " << value << "%";
}

void ReplicaMonitor::onStatusChanged(const QString& value)
{
    qDebug() << "[Replica] Status:     " << value;
}
