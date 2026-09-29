// QtWidgets 入门示例 63: QInputDialog 输入对话框
// 演示：getText / getInt / getDouble / getItem 四种静态方法
//       自定义 QDialog 多字段输入
//       重写 accept() 做校验，阻止无效提交

#include "mainwindow.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 演示四种 QInputDialog + 自定义登录对话框
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QInputDialog 输入对话框演示");
    resize(550, 500);

    auto *central = new QWidget;
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    // ---- 按钮区域 ----
    auto *btnLayout = new QHBoxLayout;

    auto *textBtn = new QPushButton("输入文件名");
    auto *intBtn = new QPushButton("设置字号");
    auto *doubleBtn = new QPushButton("缩放比例");
    auto *itemBtn = new QPushButton("选择编码");
    auto *loginBtn = new QPushButton("用户登录");

    for (auto *btn : {textBtn, intBtn, doubleBtn,
                      itemBtn, loginBtn}) {
        btn->setMinimumHeight(36);
    }

    btnLayout->addWidget(textBtn);
    btnLayout->addWidget(intBtn);
    btnLayout->addWidget(doubleBtn);
    btnLayout->addWidget(itemBtn);
    btnLayout->addWidget(loginBtn);
    mainLayout->addLayout(btnLayout);

    // ---- 结果显示 ----
    auto *resultLabel = new QLabel("操作结果:");
    mainLayout->addWidget(resultLabel);

    m_resultEdit = new QTextEdit;
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setPlaceholderText(
        "点击上方按钮触发不同的输入对话框...");
    mainLayout->addWidget(m_resultEdit);

    // ---- 信号连接 ----
    connect(textBtn, &QPushButton::clicked,
            this, &MainWindow::onGetText);
    connect(intBtn, &QPushButton::clicked,
            this, &MainWindow::onGetInt);
    connect(doubleBtn, &QPushButton::clicked,
            this, &MainWindow::onGetDouble);
    connect(itemBtn, &QPushButton::clicked,
            this, &MainWindow::onGetItem);
    connect(loginBtn, &QPushButton::clicked,
            this, &MainWindow::onLogin);
}

// ====================================================================
// getText: 文本输入
// ====================================================================
void MainWindow::onGetText()
{
    bool ok = false;
    QString text = QInputDialog::getText(
        this,
        "重命名文件",
        "请输入新文件名:",
        QLineEdit::Normal,
        "untitled.txt",
        &ok);

    if (ok && !text.isEmpty()) {
        m_resultEdit->append(
            QString("[文本] 文件名: %1").arg(text));
    } else if (ok) {
        m_resultEdit->append(
            "[文本] 用户未输入任何内容");
    } else {
        m_resultEdit->append("[文本] 用户取消了输入");
    }
}

// ====================================================================
// getInt: 整数输入 (1 ~ 72)
// ====================================================================
void MainWindow::onGetInt()
{
    bool ok = false;
    int fontSize = QInputDialog::getInt(
        this,
        "设置字号",
        "字号大小:",
        12,     // 默认值
        1,      // 最小值
        72,     // 最大值
        1,      // 步长
        &ok);

    if (ok) {
        m_resultEdit->append(
            QString("[整数] 字号: %1 pt").arg(fontSize));
    } else {
        m_resultEdit->append("[整数] 用户取消了设置");
    }
}

// ====================================================================
// getDouble: 浮点数输入 (0.1 ~ 5.0)
// ====================================================================
void MainWindow::onGetDouble()
{
    bool ok = false;
    double scale = QInputDialog::getDouble(
        this,
        "缩放比例",
        "请输入缩放比例 (0.1 ~ 5.0):",
        1.0,    // 默认值
        0.1,    // 最小值
        5.0,    // 最大值
        2,      // 小数位数
        &ok);

    if (ok) {
        m_resultEdit->append(
            QString("[浮点] 缩放比例: %1")
                .arg(scale, 0, 'f', 2));
    } else {
        m_resultEdit->append("[浮点] 用户取消了输入");
    }
}

// ====================================================================
// getItem: 列表选择
// ====================================================================
void MainWindow::onGetItem()
{
    const QStringList encodings = {
        "UTF-8", "GBK", "ISO-8859-1", "ASCII"
    };

    bool ok = false;
    QString encoding = QInputDialog::getItem(
        this,
        "选择编码",
        "请选择字符编码:",
        encodings,
        0,      // 默认选中 UTF-8
        false,  // 不可编辑
        &ok);

    if (ok) {
        m_resultEdit->append(
            QString("[列表] 编码: %1").arg(encoding));
    } else {
        m_resultEdit->append("[列表] 用户取消了选择");
    }
}

// ====================================================================
// 自定义登录对话框: 多字段输入 + accept() 校验
// ====================================================================
void MainWindow::onLogin()
{
    UserLoginDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_resultEdit->append(
            QString("[登录] 用户名: %1 | 记住密码: %2")
                .arg(dialog.username())
                .arg(dialog.rememberPassword()
                         ? "是" : "否"));
    } else {
        m_resultEdit->append("[登录] 用户取消了登录");
    }
}
