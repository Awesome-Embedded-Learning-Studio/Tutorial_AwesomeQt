#include "trafficlightwindow.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScxmlStateMachine>
#include <QVBoxLayout>
#include <QWidget>

// ========================================
// SCXML 驱动的交通灯模拟器窗口
// ========================================

TrafficLightWindow::TrafficLightWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Qt SCXML 交通灯模拟器");
    resize(400, 500);

    // 从资源文件加载 SCXML 状态图
    machine_ = QScxmlStateMachine::fromFile(
        QStringLiteral(":/trafficlight.scxml"));

    if (!machine_) {
        qCritical() << "SCXML 文件加载失败，请检查文件路径和格式";
        return;
    }

    setupUi();
    connectStateMachine();

    // 启动状态机（进入 initial="red" 指定的初始状态）
    machine_->start();
    qDebug() << "SCXML 状态机已启动";
}

/// 构建 UI 界面
void TrafficLightWindow::setupUi()
{
    auto *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(20);
    main_layout->setContentsMargins(30, 30, 30, 30);

    // 标题
    auto *title = new QLabel("交通灯模拟器", this);
    QFont title_font;
    title_font.setPointSize(16);
    title_font.setBold(true);
    title->setFont(title_font);
    title->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(title);

    // 灯壳容器（深灰背景）
    auto *shell = new QWidget(this);
    shell->setStyleSheet(
        "background-color: #333333; "
        "border-radius: 20px; "
        "padding: 20px;");
    shell->setFixedHeight(280);

    auto *shell_layout = new QHBoxLayout(shell);
    shell_layout->setSpacing(15);
    shell_layout->setContentsMargins(20, 20, 20, 20);

    // 三个灯（红、黄、绿），默认灰色
    red_light_ = createLight("#555555");
    yellow_light_ = createLight("#555555");
    green_light_ = createLight("#555555");

    shell_layout->addWidget(red_light_);
    shell_layout->addWidget(yellow_light_);
    shell_layout->addWidget(green_light_);

    main_layout->addWidget(shell);

    // 状态文字标签
    status_label_ = new QLabel("初始化中...", this);
    QFont status_font;
    status_font.setPointSize(14);
    status_label_->setFont(status_font);
    status_label_->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(status_label_);

    // 控制按钮区域
    auto *btn_layout = new QHBoxLayout();

    next_btn_ = new QPushButton("下一步", this);
    next_btn_->setMinimumHeight(45);
    next_btn_->setStyleSheet(
        "QPushButton { font-size: 14px; "
        "background-color: #2196F3; color: white; "
        "border-radius: 8px; }"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:pressed { background-color: #0D47A1; }");

    btn_layout->addWidget(next_btn_);
    main_layout->addLayout(btn_layout);

    // 说明文字
    auto *hint = new QLabel(
        "点击 \"下一步\" 按钮切换灯色\n"
        "状态切换逻辑由 trafficlight.scxml 定义", this);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: gray; font-size: 11px;");
    main_layout->addWidget(hint);
}

/// 创建一个圆形灯控件
QWidget *TrafficLightWindow::createLight(const QColor &color)
{
    auto *light = new QWidget(this);
    light->setFixedSize(70, 70);
    light->setStyleSheet(QString(
        "background-color: %1; border-radius: 35px;").arg(color.name()));
    return light;
}

/// 连接状态机信号和按钮事件
void TrafficLightWindow::connectStateMachine()
{
    // 监控状态变化：根据活跃状态更新灯色和文字
    connect(machine_, &QScxmlStateMachine::reachedStableState,
            this, [this]() {
        QStringList activeStates = machine_->activeStateNames();
        onStateChanged(QSet<QString>(
            activeStates.begin(), activeStates.end()));
    });

    // "下一步"按钮提交 next 事件
    connect(next_btn_, &QPushButton::clicked, this, [this]() {
        machine_->submitEvent(QStringLiteral("next"));
    });
}

/// 状态变化回调：更新 UI 显示
void TrafficLightWindow::onStateChanged(const QSet<QString> &states)
{
    // 先全部熄灭（灰色）
    red_light_->setStyleSheet(
        "background-color: #555555; border-radius: 35px;");
    yellow_light_->setStyleSheet(
        "background-color: #555555; border-radius: 35px;");
    green_light_->setStyleSheet(
        "background-color: #555555; border-radius: 35px;");

    // 点亮当前活跃状态对应的灯
    if (states.contains("red")) {
        red_light_->setStyleSheet(
            "background-color: #FF0000; border-radius: 35px;");
        status_label_->setText("红灯 - 停车");
        status_label_->setStyleSheet("color: red;");
    } else if (states.contains("green")) {
        green_light_->setStyleSheet(
            "background-color: #00CC00; border-radius: 35px;");
        status_label_->setText("绿灯 - 通行");
        status_label_->setStyleSheet("color: green;");
    } else if (states.contains("yellow")) {
        yellow_light_->setStyleSheet(
            "background-color: #FFCC00; border-radius: 35px;");
        status_label_->setText("黄灯 - 注意");
        status_label_->setStyleSheet("color: #B8860B;");
    }

    qDebug() << "当前状态:" << states;
}
