// QtWidgets 入门示例 04: QSS 样式表基础
// 演示：类选择器 / ID 选择器 / 后代选择器 / 伪状态(:hover :pressed :checked :disabled)
//       外部 QSS 文件加载 / 浅色-深色主题动态切换

#include "mainwindow.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QTextEdit>
#include <QGroupBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSpacerItem>

// ============================================================================
// QSS 工具函数（文件局部）
// ============================================================================
namespace {

/// @brief 从资源或文件系统加载 QSS 样式表
/// @param filepath QSS 文件路径（支持 qrc 路径或磁盘路径）
/// @return 文件内容字符串
QString loadQss(const QString &filepath)
{
    QFile file(filepath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "无法打开 QSS 文件:" << filepath;
        return {};
    }
    return QTextStream(&file).readAll();
}

} // anonymous namespace

// ============================================================================
// 浅色主题 QSS（内嵌为 C++ 常量，避免依赖外部文件方便演示）
// ============================================================================
static const char *kLightQss = R"(
/* ===== 浅色主题 ===== */

/* 全局基础样式 */
QWidget {
    background-color: #FFFFFF;
    color: #2C3E50;
    font-family: "Segoe UI", "Helvetica Neue", sans-serif;
    font-size: 13px;
}

/* ---- 类选择器：QPushButton 全局样式 ---- */
QPushButton {
    background-color: #3498DB;
    color: white;
    border: none;
    border-radius: 4px;
    padding: 8px 20px;
    font-weight: bold;
}

QPushButton:hover {
    background-color: #2980B9;
}

QPushButton:pressed {
    background-color: #21618C;
}

QPushButton:disabled {
    background-color: #BDC3C7;
    color: #95A5A6;
}

/* ---- ID 选择器：特殊按钮单独配色 ---- */
QPushButton#dangerButton {
    background-color: #E74C3C;
}

QPushButton#dangerButton:hover {
    background-color: #C0392B;
}

QPushButton#successButton {
    background-color: #27AE60;
}

QPushButton#successButton:hover {
    background-color: #1E8449;
}

QPushButton#themeToggleButton {
    background-color: #8E44AD;
    padding: 6px 14px;
    font-size: 12px;
}

QPushButton#themeToggleButton:hover {
    background-color: #7D3C98;
}

/* ---- 后代选择器：QGroupBox 内部的 QLabel 加粗 ---- */
QGroupBox QLabel {
    font-weight: bold;
    color: #34495E;
}

/* ---- 输入控件样式 ---- */
QLineEdit, QComboBox, QTextEdit {
    border: 1px solid #BDC3C7;
    border-radius: 4px;
    padding: 6px 10px;
    background-color: #FAFAFA;
    selection-background-color: #3498DB;
    selection-color: white;
}

QLineEdit:focus, QComboBox:focus, QTextEdit:focus {
    border-color: #3498DB;
}

/* ---- QComboBox 下拉部分 ---- */
QComboBox::drop-down {
    border: none;
    width: 24px;
}

QComboBox::down-arrow {
    image: none;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-top: 6px solid #7F8C8D;
}

/* ---- QCheckBox 自定义指示器 ---- */
QCheckBox {
    spacing: 8px;
}

QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border: 2px solid #BDC3C7;
    border-radius: 3px;
    background-color: white;
}

QCheckBox::indicator:hover {
    border-color: #3498DB;
}

QCheckBox::indicator:checked {
    background-color: #3498DB;
    border-color: #3498DB;
}

/* ---- QSlider ---- */
QSlider::groove:horizontal {
    height: 6px;
    background: #ECF0F1;
    border-radius: 3px;
}

QSlider::handle:horizontal {
    width: 16px;
    height: 16px;
    margin: -5px 0;
    background-color: #3498DB;
    border-radius: 8px;
}

QSlider::handle:horizontal:hover {
    background-color: #2980B9;
}

QSlider::sub-page:horizontal {
    background: #3498DB;
    border-radius: 3px;
}

/* ---- QGroupBox ---- */
QGroupBox {
    font-weight: bold;
    border: 1px solid #E0E0E0;
    border-radius: 6px;
    margin-top: 12px;
    padding: 16px 12px 12px 12px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 6px;
    color: #2C3E50;
}
)";

// ============================================================================
// 深色主题 QSS
// ============================================================================
static const char *kDarkQss = R"(
/* ===== 深色主题 ===== */

/* 全局基础样式 */
QWidget {
    background-color: #1E1E2E;
    color: #CDD6F4;
    font-family: "Segoe UI", "Helvetica Neue", sans-serif;
    font-size: 13px;
}

/* ---- QPushButton ---- */
QPushButton {
    background-color: #89B4FA;
    color: #1E1E2E;
    border: none;
    border-radius: 4px;
    padding: 8px 20px;
    font-weight: bold;
}

QPushButton:hover {
    background-color: #74C7EC;
}

QPushButton:pressed {
    background-color: #89DCEB;
}

QPushButton:disabled {
    background-color: #45475A;
    color: #6C7086;
}

/* ---- ID 选择器 ---- */
QPushButton#dangerButton {
    background-color: #F38BA8;
}

QPushButton#dangerButton:hover {
    background-color: #EBA0AC;
}

QPushButton#successButton {
    background-color: #A6E3A1;
}

QPushButton#successButton:hover {
    background-color: #94E2D5;
}

QPushButton#themeToggleButton {
    background-color: #CBA6F7;
    color: #1E1E2E;
    padding: 6px 14px;
    font-size: 12px;
}

QPushButton#themeToggleButton:hover {
    background-color: #F5C2E7;
}

/* ---- 后代选择器：QGroupBox 内部的 QLabel ---- */
QGroupBox QLabel {
    font-weight: bold;
    color: #BAC2DE;
}

/* ---- 输入控件 ---- */
QLineEdit, QComboBox, QTextEdit {
    border: 1px solid #45475A;
    border-radius: 4px;
    padding: 6px 10px;
    background-color: #313244;
    color: #CDD6F4;
    selection-background-color: #89B4FA;
    selection-color: #1E1E2E;
}

QLineEdit:focus, QComboBox:focus, QTextEdit:focus {
    border-color: #89B4FA;
}

/* ---- QComboBox 下拉 ---- */
QComboBox::drop-down {
    border: none;
    width: 24px;
}

QComboBox::down-arrow {
    image: none;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-top: 6px solid #6C7086;
}

/* ---- QCheckBox ---- */
QCheckBox {
    spacing: 8px;
}

QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border: 2px solid #45475A;
    border-radius: 3px;
    background-color: #313244;
}

QCheckBox::indicator:hover {
    border-color: #89B4FA;
}

QCheckBox::indicator:checked {
    background-color: #89B4FA;
    border-color: #89B4FA;
}

/* ---- QSlider ---- */
QSlider::groove:horizontal {
    height: 6px;
    background: #45475A;
    border-radius: 3px;
}

QSlider::handle:horizontal {
    width: 16px;
    height: 16px;
    margin: -5px 0;
    background-color: #89B4FA;
    border-radius: 8px;
}

QSlider::handle:horizontal:hover {
    background-color: #74C7EC;
}

QSlider::sub-page:horizontal {
    background: #89B4FA;
    border-radius: 3px;
}

/* ---- QGroupBox ---- */
QGroupBox {
    font-weight: bold;
    border: 1px solid #45475A;
    border-radius: 6px;
    margin-top: 12px;
    padding: 16px 12px 12px 12px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 6px;
    color: #BAC2DE;
}
)";

// ============================================================================
// MainWindow 实现
// ============================================================================
MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QSS 样式表演示 — 浅色/深色主题切换");
    resize(680, 520);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // ---- 顶部标题栏 + 主题切换按钮 ----
    auto *headerLayout = new QHBoxLayout;

    auto *titleLabel = new QLabel("QSS 样式表演示");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();

    m_themeButton = new QPushButton("切换深色主题");
    m_themeButton->setObjectName("themeToggleButton");
    connect(m_themeButton, &QPushButton::clicked, this,
            &MainWindow::toggleTheme);
    headerLayout->addWidget(m_themeButton);

    mainLayout->addLayout(headerLayout);

    // ---- 表单区 ----
    auto *formGroup = new QGroupBox("信息输入");
    auto *formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(12);

    auto *nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("请输入姓名");
    formLayout->addRow("姓名:", nameEdit);

    auto *emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("example@email.com");
    formLayout->addRow("邮箱:", emailEdit);

    auto *roleCombo = new QComboBox;
    roleCombo->addItems({"开发者", "设计师", "产品经理", "测试工程师"});
    formLayout->addRow("角色:", roleCombo);

    mainLayout->addWidget(formGroup);

    // ---- 控件展示区 ----
    auto *demoGroup = new QGroupBox("控件展示");
    auto *demoGrid = new QGridLayout(demoGroup);
    demoGrid->setSpacing(12);

    // 不同 ID 的按钮：演示 ID 选择器
    auto *saveBtn = new QPushButton("保存");
    saveBtn->setObjectName("successButton");

    auto *deleteBtn = new QPushButton("删除");
    deleteBtn->setObjectName("dangerButton");

    auto *disabledBtn = new QPushButton("已禁用");
    disabledBtn->setObjectName("disabledButton");
    disabledBtn->setEnabled(false);

    demoGrid->addWidget(saveBtn, 0, 0);
    demoGrid->addWidget(deleteBtn, 0, 1);
    demoGrid->addWidget(disabledBtn, 0, 2);

    // QCheckBox：演示 :checked 伪状态
    auto *checkLayout = new QHBoxLayout;
    auto *check1 = new QCheckBox("启用通知");
    check1->setChecked(true);
    auto *check2 = new QCheckBox("自动保存");
    check2->setChecked(true);
    auto *check3 = new QCheckBox("调试模式");
    checkLayout->addWidget(check1);
    checkLayout->addWidget(check2);
    checkLayout->addWidget(check3);
    checkLayout->addStretch();

    demoGrid->addLayout(checkLayout, 1, 0, 1, 3);

    // QSlider：演示子控件样式
    auto *sliderLabel = new QLabel("音量:");
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(60);
    slider->setFixedHeight(30);

    demoGrid->addWidget(sliderLabel, 2, 0);
    demoGrid->addWidget(slider, 2, 1, 1, 2);

    mainLayout->addWidget(demoGroup);

    // ---- 文本编辑区 ----
    auto *textGroup = new QGroupBox("备注");
    auto *textLayout = new QVBoxLayout(textGroup);

    auto *textEdit = new QTextEdit;
    textEdit->setPlaceholderText("在这里输入任何内容...");
    textEdit->setMaximumHeight(100);
    textLayout->addWidget(textEdit);

    mainLayout->addWidget(textGroup);

    // ---- 底部状态 ----
    auto *statusLayout = new QHBoxLayout;
    m_statusLabel = new QLabel("当前主题: 浅色");
    m_statusLabel->setStyleSheet("font-size: 11px; color: #888;");
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    mainLayout->addLayout(statusLayout);

    // 默认应用浅色主题
    m_isDark = false;
    applyTheme(false);
}

void MainWindow::toggleTheme()
{
    m_isDark = !m_isDark;
    applyTheme(m_isDark);
}

void MainWindow::applyTheme(bool dark)
{
    // 调用 QApplication::setStyleSheet() 替换全局样式
    if (dark) {
        qApp->setStyleSheet(kDarkQss);
        m_themeButton->setText("切换浅色主题");
        m_statusLabel->setText("当前主题: 深色");
    } else {
        qApp->setStyleSheet(kLightQss);
        m_themeButton->setText("切换深色主题");
        m_statusLabel->setText("当前主题: 浅色");
    }
}
