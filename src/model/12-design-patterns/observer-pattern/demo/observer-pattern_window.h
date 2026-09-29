/**
 * @file observer-pattern_window.h
 * @brief Observer Pattern 演示主窗口——温度 Subject + 多 Observer 面板 + 纯 C++ 对照
 *
 * 演示库类全部能力：
 * - 一对多广播：一个 valueChanged 连数值显示 / 进度条 / 历史记录三个槽；
 * - DirectConnection(同步) vs QueuedConnection(异步) 切换；
 * - disconnect 的几种写法 + lambda 连接的生命周期坑；
 * - 纯 C++ ClassicSubject + Observer 并行运行作对照。
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include "observer-pattern.h"

#include <QMainWindow>

#include <memory>

class QLabel;
class QProgressBar;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;
class QButtonGroup;

/// @brief 纯 C++ 观察者实现：把收到的温度塞进一个文本面板。
/// 不继承 QObject，纯虚函数回调——对照 Qt 槽函数。
class TextObserver : public AwesomeQt::Observer {
  public:
    explicit TextObserver(QPlainTextEdit* log) : log_(log) {}

    /// @brief ClassicSubject::notify 回调入口。
    /// @param[in] temperature 新温度。
    void onUpdate(double temperature) override;

  private:
    QPlainTextEdit* log_; // 不持有所有权
};

/// @brief 演示主窗口。
class ObserverPatternWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit ObserverPatternWindow(QWidget* parent = nullptr);
    ~ObserverPatternWindow() override;

  private:
    void setupUi();
    void connectPanels();
    void pushRandomTemperature();
    void toggleConnectionType();
    void disconnectHistoryPanel();
    void demonstrateLambdaTrap();
    void pumpClassicPath();

  private:
    // Qt 信号槽路径
    AwesomeQt::QtSubject* subject_{nullptr};
    QLabel* value_label_{nullptr};
    QProgressBar* progress_bar_{nullptr};
    QPlainTextEdit* history_view_{nullptr};
    QPlainTextEdit* classic_log_{nullptr};
    QPlainTextEdit* trap_log_{nullptr};

    // 连接类型切换
    QButtonGroup* flavor_group_{nullptr};
    QRadioButton* sync_radio_{nullptr};
    QRadioButton* async_radio_{nullptr};
    AwesomeQt::QtSubject::ConnectionFlavor current_flavor_{
        AwesomeQt::QtSubject::ConnectionFlavor::kSynchronous};

    // 历史 panel 的连接，便于演示 disconnect（按信号+槽断开）
    QMetaObject::Connection history_conn_;
    bool history_connected_{false};

    // 纯 C++ 路径
    std::unique_ptr<AwesomeQt::ClassicSubject> classic_subject_;
    std::unique_ptr<TextObserver> classic_observer_;
    double classic_value_{0.0};
};
