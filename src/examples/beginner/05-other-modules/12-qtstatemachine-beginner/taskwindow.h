/**
 * Qt StateMachine 基础示例
 *
 * 本示例演示 QtStateMachine 模块的核心功能：
 * 1. QStateMachine + QState + QFinalState 构建状态机
 * 2. addTransition() 信号触发状态转换
 * 3. assignProperty() 状态进入时自动修改属性
 * 4. 层次状态机（子状态）组织复杂状态逻辑
 *
 * 核心要点：
 * - 状态机替代 if-else 手动管理 UI 状态
 * - assignProperty 自动管理控件属性，无需手动 setEnabled
 * - 层次状态机通过父状态统管子状态的转换
 * - QFinalState 用于标记状态机流程结束
 */

#include <QWidget>

class QLabel;
class QProgressBar;
class QPushButton;
class QStateMachine;
class QTimer;

// ========================================
// 状态机驱动的任务管理器窗口
// ========================================

class TaskWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TaskWindow(QWidget *parent = nullptr);
    ~TaskWindow() override = default;

signals:

    /// 初始化完成信号（从初始化子状态转到处理子状态）
    void initFinished();

    /// 任务完成信号（从运行状态转到完成状态）
    void taskFinished();

    /// 任务停止信号（从任意运行子状态转到空闲）
    void taskStopped();

private:

    /// 构建状态机
    void setupStateMachine();

    /// 模拟初始化阶段
    void simulateInit();

    /// 模拟处理阶段（更新进度条）
    void simulateProcessing();

private:
    QLabel *status_label_;
    QProgressBar *progress_bar_;
    QPushButton *start_btn_;
    QPushButton *pause_btn_;
    QPushButton *resume_btn_;
    QPushButton *stop_btn_;

    QStateMachine *machine_ = nullptr;
    QTimer *progress_timer_ = nullptr;
};
