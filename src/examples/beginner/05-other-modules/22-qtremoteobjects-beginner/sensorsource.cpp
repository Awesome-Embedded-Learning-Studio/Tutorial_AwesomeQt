#include "sensorsource.h"

#include <QDebug>

SensorSource::SensorSource(QObject *parent)
    : SensorDataSimpleSource(parent) {}

void SensorSource::reset()
{
    qDebug() << "[Source] 收到远程 reset 请求，重置数据";
    setTemperature(22.0);
    setHumidity(50.0);
    setUpdateCount(0);
    setStatus(QStringLiteral("Reset"));
}
