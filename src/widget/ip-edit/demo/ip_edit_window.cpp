/**
 * @file ip_edit_window.cpp
 * @brief IpEdit 演示主窗口实现
 * @copyright Copyright (c) 2026
 */

#include "ip_edit_window.h"

#include "ip_edit.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

IpEditWindow::IpEditWindow(QWidget* parent) : QMainWindow(parent) {
    setup_ui();
}

QWidget* IpEditWindow::setup_input_layout() {
    auto* group = new QGroupBox("IPv4 输入（满 3 位自动跳焦 / 按 . 跳段 / 段首退格回上段）");
    auto* layout = new QVBoxLayout(group);

    auto* ip_edit = new AwesomeQt::IpEdit(group);
    layout->addWidget(ip_edit);

    // —— 显示值：回显 text() + isValid() ——
    auto* echo_row = new QHBoxLayout();
    auto* echo_label = new QLabel("当前值：(未填写)", group);
    echo_label->setMinimumWidth(280);
    auto* show_btn = new QPushButton("显示值", group);

    connect(show_btn, &QPushButton::clicked, group, [ip_edit, echo_label]() {
        const QString val = ip_edit->text();
        const QString valid = ip_edit->isValid() ? "合法" : "非法";
        echo_label->setText(QString("当前值：%1   [%2]").arg(val, valid));
    });

    // —— textChanged 实时联动（输入即更新回显）——
    connect(ip_edit, &AwesomeQt::IpEdit::textChanged, group, [echo_label](const QString& addr) {
        echo_label->setText(QString("当前值：%1").arg(addr));
    });

    // —— editingFinished（末段回车/失焦）提示 ——
    auto* finished_label = new QLabel("（editingFinished 未触发）", group);
    connect(ip_edit, &AwesomeQt::IpEdit::editingFinished, group, [finished_label]() {
        finished_label->setText("editingFinished 触发！（末段回车/失焦）");
    });

    echo_row->addWidget(echo_label);
    echo_row->addWidget(show_btn);
    echo_row->addStretch();
    layout->addLayout(echo_row);
    layout->addWidget(finished_label);

    return group;
}

QWidget* IpEditWindow::setup_presets_layout() {
    auto* group = new QGroupBox("预设地址 + 清空");
    auto* layout = new QHBoxLayout(group);

    // 预设按钮直接 new 一个独立 IpEdit 演示 setText
    auto* ip_edit = new AwesomeQt::IpEdit(group);
    layout->addWidget(ip_edit);

    auto* b1 = new QPushButton("192.168.1.1", group);
    auto* b2 = new QPushButton("10.0.0.1", group);
    auto* clear_btn = new QPushButton("清空", group);

    connect(b1, &QPushButton::clicked, group,
            [ip_edit]() { ip_edit->setText(QStringLiteral("192.168.1.1")); });
    connect(b2, &QPushButton::clicked, group,
            [ip_edit]() { ip_edit->setText(QStringLiteral("10.0.0.1")); });
    connect(clear_btn, &QPushButton::clicked, group, [ip_edit]() { ip_edit->clear(); });

    layout->addWidget(b1);
    layout->addWidget(b2);
    layout->addWidget(clear_btn);
    layout->addStretch();

    return group;
}

QWidget* IpEditWindow::setup_edgecases_layout() {
    auto* group = new QGroupBox("边界校验（点按钮模拟程序化 setText）");
    auto* layout = new QVBoxLayout(group);

    auto* hint = new QLabel("用预设按钮塞越界/不足段值，观察 clamp 与补 0：", group);
    layout->addWidget(hint);

    auto* ip_edit = new AwesomeQt::IpEdit(group);
    layout->addWidget(ip_edit);

    auto* result_label = new QLabel("结果：—", group);
    layout->addWidget(result_label);

    auto* row = new QHBoxLayout();
    auto* b999 = new QPushButton("setText(\"999.1.1.1\")", group); // 越界 → 夹 255
    auto* b3 = new QPushButton("setText(\"1.2.3\")", group);       // 不足段 → 补 0
    auto* babc = new QPushButton("setText(\"a.b.c\")", group);     // 非数字 → 补 0
    auto* bempty = new QPushButton("setText(\"\")", group);        // 空串 → 全清
    auto* bshow = new QPushButton("显示值", group);

    connect(b999, &QPushButton::clicked, group,
            [ip_edit]() { ip_edit->setText(QStringLiteral("999.1.1.1")); });
    connect(b3, &QPushButton::clicked, group,
            [ip_edit]() { ip_edit->setText(QStringLiteral("1.2.3")); });
    connect(babc, &QPushButton::clicked, group,
            [ip_edit]() { ip_edit->setText(QStringLiteral("a.b.c")); });
    connect(bempty, &QPushButton::clicked, group, [ip_edit]() { ip_edit->setText(QString()); });
    connect(bshow, &QPushButton::clicked, group, [ip_edit, result_label]() {
        const QString valid = ip_edit->isValid() ? "合法" : "非法";
        result_label->setText(QString("结果：%1   [%2]").arg(ip_edit->text(), valid));
    });

    row->addWidget(b999);
    row->addWidget(b3);
    row->addWidget(babc);
    row->addWidget(bempty);
    row->addWidget(bshow);
    row->addStretch();
    layout->addLayout(row);

    // 提示：子 QLineEdit 直接输 abc 会被 QIntValidator 拒绝（非数字根本输不进）
    auto* note = new QLabel(
        "注：直接在段里敲字母会被 QIntValidator 拒绝（输不进）；这里只测程序化 setText。", group);
    note->setStyleSheet("color: gray;");
    layout->addWidget(note);

    return group;
}

void IpEditWindow::setup_ui() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* layout = new QVBoxLayout(central);
    layout->addWidget(setup_input_layout());
    layout->addWidget(setup_presets_layout());
    layout->addWidget(setup_edgecases_layout());
    layout->addStretch();

    setWindowTitle("IpEdit Widget Demo");
    resize(520, 320);
}
