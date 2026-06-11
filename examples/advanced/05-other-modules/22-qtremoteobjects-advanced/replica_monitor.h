/// @file    replica_monitor.h
/// @brief   Replica-side monitor that receives sensor data from a remote source.
///
/// Demonstrates how a QRemoteObjectReplica connects to a source node,
/// acquires a replica, and reacts to property-change signals as data
/// arrives from the source side.

#pragma once

// The repc-generated replica header provides SensorSourceReplica
#include "rep_sensor_source_replica.h"

#include <QObject>
#include <QRemoteObjectNode>

/// @brief Connects to a SensorSource via Qt Remote Objects and prints
///        every property update received from the source node.
class ReplicaMonitor : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the monitor with a parent for ownership.
    /// @param[in] parent Parent QObject for Qt object-tree lifecycle.
    explicit ReplicaMonitor(QObject* parent = nullptr);

    /// @brief Connects to the source node through a local (in-process) registry.
    /// @note  Uses "local:registry" which avoids any network socket — ideal
    ///        for in-process demos and unit tests.
    void connectToLocal();

signals:
    /// @brief Emitted once the replica is fully initialized and synchronized.
    void replicaInitialized();

private:
    /// @brief Callback wired to the replica's temperatureChanged signal.
    /// @param[in] value The updated temperature reading.
    void onTemperatureChanged(double value);

    /// @brief Callback wired to the replica's humidityChanged signal.
    /// @param[in] value The updated humidity reading.
    void onHumidityChanged(double value);

    /// @brief Callback wired to the replica's statusChanged signal.
    /// @param[in] value The updated status string.
    void onStatusChanged(const QString& value);

    QRemoteObjectNode m_node;              ///< Replica-side node for receiving.
    SensorSourceReplica* m_replica;        ///< Pointer held for signal wiring.
};
