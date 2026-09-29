/// @file    sensor_source.cpp
/// @brief   Implementation of the SensorSource remote object source.

#include "sensor_source.h"

#include <QRandomGenerator>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SensorSource::SensorSource(QObject* parent)
    : SensorSourceSimpleSource(parent)
    , m_updateCount(0)
{
    // One-second interval simulates real sensor polling
    m_updateTimer.setInterval(1000);
    connect(&m_updateTimer, &QTimer::timeout, this,
            &SensorSource::updateSensorData);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void SensorSource::updateSensorData()
{
    ++m_updateCount;

    // Simulate realistic sensor drift around baseline values
    const double newTemp = generateReading(23.5, 3.0);
    const double newHum  = generateReading(50.0, 15.0);

    // Setter methods are provided by SensorSourceSimpleSource; they
    // compare old vs new and emit the corresponding NOTIFY signal.
    setTemperature(newTemp);
    setHumidity(newHum);

    // Build a descriptive status string for each update cycle.
    // status is a READ-only property on the replica side, so we use
    // pushStatus() to propagate it from source to replica.
    const QString newStatus =
        QStringLiteral("Update #%1 — temp: %2 C, humidity: %3%")
            .arg(m_updateCount)
            .arg(newTemp, 0, 'f', 1)
            .arg(newHum, 0, 'f', 1);
    pushStatus(newStatus);
}

void SensorSource::start()
{
    // Fire the first update immediately so the replica sees initial data
    updateSensorData();
    m_updateTimer.start();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

double SensorSource::generateReading(double center, double spread)
{
    // QRandomGenerator::bounded produces uniform noise; adding to center
    // gives a simple but effective sensor simulation.
    const double offset =
        QRandomGenerator::global()->bounded(static_cast<quint32>(spread * 2))
            - spread;
    return center + offset;
}
