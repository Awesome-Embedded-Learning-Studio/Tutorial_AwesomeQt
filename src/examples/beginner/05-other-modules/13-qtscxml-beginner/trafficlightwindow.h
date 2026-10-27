/**
 * Qt SCXML 基础示例
 *
 * 本示例演示 Qt Scxml 模块的核心功能：
 * 1. 从 .scxml 文件加载状态机（QScxmlStateMachine::fromFile）
 * 2. submitEvent() 提交事件触发状态转换
 * 3. 监控 entered 信号跟踪状态变化
 * 4. SCXML 状态图与 C++ UI 代码的协作模式
 *
 * 核心要点：
 * - SCXML 文件定义状态结构，C++ 代码只负责加载和 UI 响应
 * - submitEvent 的事件名对应 .scxml 中 <transition event="...">
 * - entered 信号返回当前所有活跃状态的集合
 * - fromFile 返回 nullptr 表示加载失败，必须检查
 */

#include <QColor>
#include <QSet>
#include <QString>
#include <QWidget>

class QLabel;
class QPushButton;
class QScxmlStateMachine;

// ========================================
// SCXML 驱动的交通灯模拟器窗口
// ========================================

class TrafficLightWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TrafficLightWindow(QWidget *parent = nullptr);
    ~TrafficLightWindow() override = default;

private:
    /// 构建 UI 界面
    void setupUi();

    /// 创建一个圆形灯控件
    QWidget *createLight(const QColor &color);

    /// 连接状态机信号和按钮事件
    void connectStateMachine();

    /// 状态变化回调：更新 UI 显示
    void onStateChanged(const QSet<QString> &states);

private:
    QScxmlStateMachine *machine_ = nullptr;
    QWidget *red_light_ = nullptr;
    QWidget *yellow_light_ = nullptr;
    QWidget *green_light_ = nullptr;
    QLabel *status_label_ = nullptr;
    QPushButton *next_btn_ = nullptr;
};
