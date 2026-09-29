/// @file    sensor_source.h
/// @brief   Remote object source that simulates a temperature/humidity sensor.
///
/// Inherits from the repc-generated SensorSourceSimpleSource base class.
/// Adds a QTimer-driven simulation loop that updates sensor readings
/// periodically and broadcasts changes to all connected replicas.

#pragma once

// The repc-generated source header provides SensorSourceSimpleSource
#include "rep_sensor_source_source.h"

#include <QTimer>

/// @brief Concrete sensor source that periodically generates simulated
///        temperature and humidity data for remote object replicas.
class SensorSource : public SensorSourceSimpleSource
{
    Q_OBJECT

public:
    /// @brief Constructs the sensor source with a parent for ownership.
    /// @param[in] parent Parent QObject for Qt object-tree lifecycle.
    explicit SensorSource(QObject* parent = nullptr);

    /// @brief Starts the periodic simulation timer.
    /// @note  Must be called after enableRemoting() so replicas are
    ///        already listening when the first update fires.
    void start();

    /// @brief Triggers a single sensor data update cycle.
    /// @note  Overrides the pure virtual from SensorSourceSource so the
    ///        SLOT defined in the .rep file reaches this implementation.
    void updateSensorData() override;

private:
    /// @brief Generates a random sensor reading within a realistic range.
    /// @param[in] center  The center value around which to vary.
    /// @param[in] spread  The maximum deviation from center.
    /// @return A randomized sensor value.
    double generateReading(double center, double spread);

    QTimer m_updateTimer;   ///< Periodic sensor update timer.
    int m_updateCount;      ///< Tracks how many updates have occurred.
};
