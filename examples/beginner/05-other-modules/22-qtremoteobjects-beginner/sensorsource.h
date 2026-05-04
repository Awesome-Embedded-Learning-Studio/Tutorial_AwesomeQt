#include <QObject>

// repc 生成的头文件，位于构建目录中
#include "rep_sensordata_source.h"

// 自定义 Source 类，实现 reset() 纯虚槽函数
class SensorSource : public SensorDataSimpleSource
{
    Q_OBJECT
public:
    explicit SensorSource(QObject *parent = nullptr);

public Q_SLOTS:
    void reset() override;
};
