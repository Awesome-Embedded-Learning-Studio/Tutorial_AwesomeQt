/**
 * @file observer-pattern_window.cpp
 * @brief Observer Pattern 演示实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "observer-pattern_window.h"

#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <cmath>

// ---------------------------------------------------------------------------
// 纯 C++ 观察者：不继承 QObject，靠虚函数回调
// ---------------------------------------------------------------------------

void TextObserver::onUpdate(double temperature) {
    if (log_ == nullptr) {
        return;
    }
    log_->appendPlainText(QString("[classic] temperature = %1").arg(temperature, 0, 'f', 2));
}

// ---------------------------------------------------------------------------
// 主窗口
// ---------------------------------------------------------------------------

ObserverPatternWindow::ObserverPatternWindow(QWidget* parent) : QMainWindow(parent) {
    subject_ = new AwesomeQt::QtSubject(this);
    classic_subject_ = std::make_unique<AwesomeQt::ClassicSubject>();
    classic_observer_ = std::make_unique<TextObserver>(classic_log_);

    setupUi();
    connectPanels();

    // 把纯 C++ observer 挂到 ClassicSubject——这是 GoF 写法的「注册」。
    classic_subject_->attach(classic_observer_.get());

    setWindowTitle("Observer Pattern Demo (Qt signals & slots vs classic C++)");
    resize(820, 600);
}

ObserverPatternWindow::~ObserverPatternWindow() = default;

void ObserverPatternWindow::setupUi() {
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    // ---- Qt 信号槽路径：三个面板连同一个信号（一对多广播）----
    auto* qt_box = new QGroupBox("Qt signals & slots  (1 signal -> N slots)", central);
    auto* qt_layout = new QVBoxLayout(qt_box);

    value_label_ = new QLabel("temperature: --", qt_box);
    value_label_->setMinimumWidth(220);

    progress_bar_ = new QProgressBar(qt_box);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);

    history_view_ = new QPlainTextEdit(qt_box);
    history_view_->setReadOnly(true);
    history_view_->setMaximumBlockCount(200);

    qt_layout->addWidget(new QLabel("Panel A: numeric display (sync slot)", qt_box));
    qt_layout->addWidget(value_label_);
    qt_layout->addWidget(new QLabel("Panel B: progress bar 0..100 (sync slot)", qt_box));
    qt_layout->addWidget(progress_bar_);
    qt_layout->addWidget(new QLabel("Panel C: history log (slot, can be disconnected)", qt_box));
    qt_layout->addWidget(history_view_);
    root->addWidget(qt_box);

    // ---- 控制区：连接类型 / 断开 / lambda 陷阱 ----
    auto* ctrl_box = new QGroupBox("Controls", central);
    auto* ctrl_layout = new QVBoxLayout(ctrl_box);

    sync_radio_ = new QRadioButton("DirectConnection (synchronous)", ctrl_box);
    async_radio_ = new QRadioButton("QueuedConnection (asynchronous)", ctrl_box);
    sync_radio_->setChecked(true);
    flavor_group_ = new QButtonGroup(ctrl_box);
    flavor_group_->addButton(sync_radio_);
    flavor_group_->addButton(async_radio_);
    ctrl_layout->addWidget(sync_radio_);
    ctrl_layout->addWidget(async_radio_);

    auto* push_btn = new QPushButton("Push random temperature", ctrl_box);
    auto* toggle_btn = new QPushButton("Reconnect panel C with selected type", ctrl_box);
    auto* disc_btn = new QPushButton("Disconnect panel C  (disconnect by signal+slot)", ctrl_box);
    auto* trap_btn = new QPushButton("Demo: lambda trap  (context-guarded)", ctrl_box);

    ctrl_layout->addWidget(push_btn);
    ctrl_layout->addWidget(toggle_btn);
    ctrl_layout->addWidget(disc_btn);
    ctrl_layout->addWidget(trap_btn);
    root->addWidget(ctrl_box);

    // ---- 纯 C++ 对照路径 ----
    auto* classic_box =
        new QGroupBox("Classic C++  (Observer interface + ClassicSubject)", central);
    auto* classic_layout = new QVBoxLayout(classic_box);
    classic_log_ = new QPlainTextEdit(classic_box);
    classic_log_->setReadOnly(true);
    classic_log_->setMaximumBlockCount(200);
    classic_layout->addWidget(new QLabel("Panel D: classic Observer::onUpdate log", classic_box));
    classic_layout->addWidget(classic_log_);
    root->addWidget(classic_box);

    // ---- lambda 陷阱日志 ----
    auto* trap_box = new QGroupBox("Lambda connection log", central);
    auto* trap_layout = new QVBoxLayout(trap_box);
    trap_log_ = new QPlainTextEdit(trap_box);
    trap_log_->setReadOnly(true);
    trap_log_->setMaximumBlockCount(100);
    trap_layout->addWidget(trap_log_);
    root->addWidget(trap_box);

    setCentralWidget(central);

    // 绑定按钮
    connect(push_btn, &QPushButton::clicked, this, &ObserverPatternWindow::pushRandomTemperature);
    connect(toggle_btn, &QPushButton::clicked, this, &ObserverPatternWindow::toggleConnectionType);
    connect(disc_btn, &QPushButton::clicked, this, &ObserverPatternWindow::disconnectHistoryPanel);
    connect(trap_btn, &QPushButton::clicked, this, &ObserverPatternWindow::demonstrateLambdaTrap);
    connect(flavor_group_, &QButtonGroup::idClicked, this, [this](int /*id*/) {
        current_flavor_ = sync_radio_->isChecked()
                              ? AwesomeQt::QtSubject::ConnectionFlavor::kSynchronous
                              : AwesomeQt::QtSubject::ConnectionFlavor::kAsynchronous;
    });
}

void ObserverPatternWindow::connectPanels() {
    // 一对多：同一个 valueChanged 连三个槽——数值显示、进度条、历史。
    // Qt6 函数指针语法：connect(sender, &Cls::sig, receiver, &Cls::slot)
    connect(subject_, &AwesomeQt::QtSubject::valueChanged, this, [this](double temperature) {
        value_label_->setText(QString("temperature: %1 C").arg(temperature, 0, 'f', 2));
    });

    connect(subject_, &AwesomeQt::QtSubject::valueChanged, this, [this](double temperature) {
        // 温度 0..100 直接映射进度条
        int pct = static_cast<int>(std::round(std::clamp(temperature, 0.0, 100.0)));
        progress_bar_->setValue(pct);
    });

    // 历史 panel 用成员记录连接，便于演示 disconnect
    history_conn_ =
        connect(subject_, &AwesomeQt::QtSubject::valueChanged, this, [this](double temperature) {
            history_view_->appendPlainText(
                QString("[qt] temperature = %1").arg(temperature, 0, 'f', 2));
        });
    history_connected_ = true;
}

void ObserverPatternWindow::pushRandomTemperature() {
    // 两条路径并行推进：Qt 信号槽路径 + 纯 C++ ClassicSubject 路径
    const double next = (std::rand() % 10000) / 100.0; // 0.00..99.99
    subject_->setValue(next);                          // emit valueChanged -> 广播

    classic_value_ = next;
    pumpClassicPath(); // 纯 C++ notify
}

void ObserverPatternWindow::toggleConnectionType() {
    // 演示连接类型：先断开历史 panel，再按当前选择重连。
    if (history_connected_) {
        disconnect(history_conn_);
        history_connected_ = false;
    }

    const auto flavor = current_flavor_;
    const bool is_async = (flavor == AwesomeQt::QtSubject::ConnectionFlavor::kAsynchronous);
    const Qt::ConnectionType type =
        is_async ? Qt::QueuedConnection : Qt::ConnectionType::DirectConnection;

    // 同对象跨连接类型：QueuedConnection 会把调用排队到事件循环（本线程 likewise 可见异步效果）。
    history_conn_ = connect(
        subject_, &AwesomeQt::QtSubject::valueChanged, this,
        [this, is_async](double temperature) {
            history_view_->appendPlainText(QString("%1 temperature = %2")
                                               .arg(is_async ? "[qt-queued]" : "[qt-direct]")
                                               .arg(temperature, 0, 'f', 2));
        },
        type);
    history_connected_ = true;

    history_view_->appendPlainText(
        QString("--- reconnected panel C as %1 ---")
            .arg(is_async ? "QueuedConnection (async)" : "DirectConnection (sync)"));
}

void ObserverPatternWindow::disconnectHistoryPanel() {
    // 断开方式一：用保存的 QMetaObject::Connection（最稳，无歧义）。
    // 也可写成 disconnect(subject_, &QtSubject::valueChanged, this, functor) 但 lambda 无 == 运算，
    // 拿不到原 functor 时断不掉——这正是 lambda 连接的生命周期坑（见 demonstrateLambdaTrap）。
    if (!history_connected_) {
        history_view_->appendPlainText("--- panel C already disconnected ---");
        return;
    }
    disconnect(history_conn_);
    history_connected_ = false;
    history_view_->appendPlainText("--- panel C disconnected (no future updates) ---");
}

void ObserverPatternWindow::demonstrateLambdaTrap() {
    // lambda 连接的生命周期坑：捕获了 this / 裸指针，若 receiver 先于连接销毁，
    // 信号再次 emit 会回调到已删除对象。Qt5 旧写法 connect(sender, sig, lambda) 无 context，
    // 连接不跟随任何对象，必须手动 disconnect——极易遗漏。
    //
    // 正路：connect 时给一个 context对象（第 3 参），context 被销毁时连接自动断开。
    // 下面用一个临时 QLabel 作 context：销毁后即使 emit 也安全。
    auto* context_label = new QLabel(); // 临时 context，不挂父对象
    context_label->setText("ephemeral");

    auto conn = connect(subject_, &AwesomeQt::QtSubject::valueChanged, context_label,
                        [context_label, this](double temperature) {
                            // context_label 还活着才会被调；被 delete 后连接自动失效。
                            trap_log_->appendPlainText(QString("[lambda] via context: %1 (temp=%2)")
                                                           .arg(context_label->text())
                                                           .arg(temperature, 0, 'f', 2));
                        });

    // 触发一次：context 还活着，槽会跑。通过公有接口 setValue 发射信号（不在外部直接 emit 信号）。
    subject_->setValue(subject_->value());

    // 现在 delete context——连接随之失效。再 emit 不会回调到已删对象（这就是 context 守护）。
    context_label->deleteLater();
    Q_UNUSED(conn);

    trap_log_->appendPlainText(
        "--- context scheduled for deletion; future emits are safe (auto-disconnect) ---");
}

void ObserverPatternWindow::pumpClassicPath() {
    // 纯 C++ 路径：手动 notify。若此时 classic_observer_ 已销毁但未 detach，这里就崩。
    // 对照：Qt 路径完全不用手写这一步。
    classic_subject_->notify(classic_value_);
}
