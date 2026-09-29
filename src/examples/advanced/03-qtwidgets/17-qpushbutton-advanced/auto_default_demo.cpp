/// @file    auto_default_demo.cpp
/// @brief   AutoDefaultDemo 类实现——autoDefault 拦截、setMenu 信号抑制、flat 焦点框演示。
///
/// 对应教程：进阶层 03-QtWidgets/17-QPushButton 进阶。

#include "auto_default_demo.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QVBoxLayout>

// ── 构造函数 ──────────────────────────────────────────────────────────────────

AutoDefaultDemo::AutoDefaultDemo(QWidget* parent)
    : QDialog(parent)
    , m_logOutput(new QTextEdit(this))
{
    setWindowTitle(QStringLiteral("QPushButton Advanced Demo"));
    resize(560, 480);

    // m_logOutput 只读，用于显示信号触发日志
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumHeight(160);

    auto* mainLayout = new QVBoxLayout(this);

    // 三个演示分区垂直排列
    mainLayout->addWidget(createAutoDefaultSection(this));
    mainLayout->addWidget(createMenuSection(this));
    mainLayout->addWidget(createFlatSection(this));
    mainLayout->addWidget(m_logOutput, /*stretch=*/1);

    appendLog(QStringLiteral("提示：在输入框中按 Enter 观察信号流向"));
}

// ── 分区 1：autoDefault 拦截演示 ──────────────────────────────────────────────

QWidget* AutoDefaultDemo::createAutoDefaultSection(QWidget* parent)
{
    auto* box = new QWidget(parent);
    auto* layout = new QVBoxLayout(box);

    auto* title = new QLabel(
        QStringLiteral("1. autoDefault 拦截（勾选关闭 autoDefault 以解除拦截）"));
    title->setStyleSheet(QStringLiteral("font-weight: bold;"));

    auto* input = new QLineEdit;
    input->setPlaceholderText(
        QStringLiteral("在输入框中按 Enter，观察日志输出"));

    // autoDefault 在 QDialog 中默认为 true，会拦截 Enter 键
    auto* submitBtn = new QPushButton(QStringLiteral("提交"));
    auto* toggleCheck = new QCheckBox(QStringLiteral("autoDefault = true"));

    // 初始状态：autoDefault 开启（QDialog 中默认行为）
    toggleCheck->setChecked(true);

    layout->addWidget(title);
    layout->addWidget(input);
    layout->addWidget(submitBtn);
    layout->addWidget(toggleCheck);

    // QLineEdit 的 returnPressed——如果被按钮拦截则不会触发
    connect(input, &QLineEdit::returnPressed, this, [this]() {
        appendLog(QStringLiteral("[returnPressed] 输入框触发"));
    });

    // QPushButton 的 clicked——autoDefault 开启时按 Enter 会走这里
    connect(submitBtn, &QPushButton::clicked, this, [this]() {
        appendLog(QStringLiteral("[clicked] 提交按钮触发"));
    });

    // 复选框切换 autoDefault 状态
    connect(toggleCheck, &QCheckBox::toggled, this, [submitBtn, this](bool checked) {
        submitBtn->setAutoDefault(checked);
        appendLog(QStringLiteral("autoDefault 已设为 %1")
                      .arg(checked ? QStringLiteral("true")
                                   : QStringLiteral("false")));
    });

    return box;
}

// ── 分区 2：setMenu() 信号抑制演示 ───────────────────────────────────────────

QWidget* AutoDefaultDemo::createMenuSection(QWidget* parent)
{
    auto* box = new QWidget(parent);
    auto* layout = new QVBoxLayout(box);

    auto* title = new QLabel(
        QStringLiteral("2. setMenu() 后 clicked() 信号被抑制"));
    title->setStyleSheet(QStringLiteral("font-weight: bold;"));

    auto* row = new QHBoxLayout;

    // 带菜单的按钮——clicked 不会被触发
    auto* menuBtn = new QPushButton(QStringLiteral("带菜单按钮"));
    auto* menu = new QMenu(menuBtn);
    menu->addAction(QStringLiteral("选项 A"), this, [this]() {
        appendLog(QStringLiteral("[menu] 选项 A 被选中"));
    });
    menu->addAction(QStringLiteral("选项 B"), this, [this]() {
        appendLog(QStringLiteral("[menu] 选项 B 被选中"));
    });
    menuBtn->setMenu(menu);

    connect(menuBtn, &QPushButton::clicked, this, [this]() {
        appendLog(QStringLiteral("[clicked] 带菜单按钮——这条不会出现"));
    });

    connect(menuBtn, &QPushButton::pressed, this, [this]() {
        appendLog(QStringLiteral("[pressed] 带菜单按钮——这条也不会出现"));
    });

    // 普通按钮作为对照
    auto* normalBtn = new QPushButton(QStringLiteral("普通按钮（对照）"));
    connect(normalBtn, &QPushButton::clicked, this, [this]() {
        appendLog(QStringLiteral("[clicked] 普通按钮正常触发"));
    });

    layout->addWidget(title);
    row->addWidget(menuBtn);
    row->addWidget(normalBtn);
    layout->addLayout(row);

    return box;
}

// ── 分区 3：flat 按钮焦点框演示 ──────────────────────────────────────────────

QWidget* AutoDefaultDemo::createFlatSection(QWidget* parent)
{
    auto* box = new QWidget(parent);
    auto* layout = new QVBoxLayout(box);

    auto* title = new QLabel(
        QStringLiteral("3. flat 按钮焦点框（勾选关闭焦点策略）"));
    title->setStyleSheet(QStringLiteral("font-weight: bold;"));

    auto* row = new QHBoxLayout;

    // flat 按钮——点击后可能残留焦点框
    auto* flatBtn = new QPushButton(QStringLiteral("flat 按钮（可获焦）"));
    flatBtn->setFlat(true);

    auto* flatNoFocusBtn = new QPushButton(
        QStringLiteral("flat 按钮（NoFocus）"));
    flatNoFocusBtn->setFlat(true);
    flatNoFocusBtn->setFocusPolicy(Qt::NoFocus);

    auto* toggleCheck = new QCheckBox(QStringLiteral("左侧按钮启用 NoFocus"));

    connect(flatBtn, &QPushButton::clicked, this, [this]() {
        appendLog(QStringLiteral("[clicked] 可获焦 flat 按钮被点击"));
    });
    connect(flatNoFocusBtn, &QPushButton::clicked, this, [this]() {
        appendLog(QStringLiteral("[clicked] NoFocus flat 按钮被点击"));
    });
    connect(toggleCheck, &QCheckBox::toggled, this,
            [flatBtn, this](bool checked) {
                flatBtn->setFocusPolicy(checked ? Qt::NoFocus
                                                : Qt::TabFocus);
                appendLog(QStringLiteral("可获焦 flat 按钮的 focusPolicy 已设为 %1")
                              .arg(checked ? QStringLiteral("NoFocus")
                                           : QStringLiteral("TabFocus")));
            });

    layout->addWidget(title);
    row->addWidget(flatBtn);
    row->addWidget(flatNoFocusBtn);
    row->addWidget(toggleCheck);
    layout->addLayout(row);

    return box;
}

// ── 日志输出 ──────────────────────────────────────────────────────────────────

void AutoDefaultDemo::appendLog(const QString& message)
{
    m_logOutput->append(QStringLiteral("[%1] %2").arg(
        QTime::currentTime().toString(QStringLiteral("HH:mm:ss.zzz")),
        message));
}
