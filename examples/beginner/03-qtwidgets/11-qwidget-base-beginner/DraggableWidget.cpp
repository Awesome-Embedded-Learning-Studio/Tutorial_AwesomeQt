// QtWidgets 入门示例 11: QWidget 基类基础
// 演示：窗口属性 resize / move / setWindowTitle / setWindowIcon
//       show() / hide() / setVisible() / raise() / lower()
//       尺寸策略 setSizePolicy / setFixedSize / setMinimumSize
//       窗口标志 Qt::WindowFlags（无边框、置顶、工具窗口）

#include "DraggableWidget.h"

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPainter>
#include <QMouseEvent>

#include <utility>

// ============================================================================
// DraggableWidget: 支持无边框拖拽的自定义 QWidget
// ============================================================================
DraggableWidget::DraggableWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QWidget 基类功能演示");
    resize(720, 520);

    // 标记是否处于自定义拖拽模式（无边框时使用）
    m_dragging = false;

    initUi();
}

void DraggableWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆角背景
    painter.setBrush(QColor("#FAFAFA"));
    painter.setPen(QPen(QColor("#CCCCCC"), 1));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}

void DraggableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton &&
        windowFlags() & Qt::FramelessWindowHint) {
        m_dragPosition = event->globalPosition().toPoint() - pos();
        m_dragging = true;
    }
    QWidget::mousePressEvent(event);
}

void DraggableWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging &&
        (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
    }
    QWidget::mouseMoveEvent(event);
}

void DraggableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}

void DraggableWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("QWidget 基类功能演示");
    titleLabel->setFont(QFont("Arial", 16, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ---- 窗口属性组 ----
    auto *propGroup = new QGroupBox("窗口属性");
    auto *propLayout = new QHBoxLayout(propGroup);

    auto *resizeBtn = new QPushButton("切换大小");
    resizeBtn->setToolTip("在 720x520 和 500x350 之间切换");
    connect(resizeBtn, &QPushButton::clicked, this, [this]() {
        if (width() > 600) {
            resize(500, 350);
        } else {
            resize(720, 520);
        }
        updateSizeLabel();
    });

    auto *moveBtn = new QPushButton("移动窗口");
    moveBtn->setToolTip("在两个预设位置之间移动");
    connect(moveBtn, &QPushButton::clicked, this, [this]() {
        static bool toggle = false;
        toggle = !toggle;
        move(toggle ? QPoint(200, 100) : QPoint(500, 250));
    });

    auto *titleBtn = new QPushButton("修改标题");
    titleBtn->setToolTip("循环切换不同的窗口标题");
    connect(titleBtn, &QPushButton::clicked, this, [this]() {
        static int idx = 0;
        QStringList titles = {
            "QWidget 基类功能演示",
            "窗口标题已修改!",
            "Hello QWidget!",
            "Qt 6.9.1 基础控件"
        };
        idx = (idx + 1) % titles.size();
        setWindowTitle(titles[idx]);
    });

    propLayout->addWidget(resizeBtn);
    propLayout->addWidget(moveBtn);
    propLayout->addWidget(titleBtn);
    mainLayout->addWidget(propGroup);

    // ---- 可见性控制组 ----
    auto *visGroup = new QGroupBox("可见性控制");
    auto *visLayout = new QHBoxLayout(visGroup);

    auto *togglePanelBtn = new QPushButton("显示/隐藏信息面板");
    connect(togglePanelBtn, &QPushButton::clicked, this, [this]() {
        // setVisible 是 show/hide 的统一接口
        m_infoPanel->setVisible(!m_infoPanel->isVisible());
    });

    auto *raiseBtn = new QPushButton("raise() 提升");
    raiseBtn->setToolTip("将红色标签提升到最顶层");
    connect(raiseBtn, &QPushButton::clicked, this, [this]() {
        m_backLabel->raise();
    });

    auto *lowerBtn = new QPushButton("lower() 降低");
    lowerBtn->setToolTip("将红色标签降到最底层");
    connect(lowerBtn, &QPushButton::clicked, this, [this]() {
        m_backLabel->lower();
    });

    visLayout->addWidget(togglePanelBtn);
    visLayout->addWidget(raiseBtn);
    visLayout->addWidget(lowerBtn);
    mainLayout->addWidget(visGroup);

    // ---- 信息面板（可显示/隐藏）----
    m_infoPanel = new QLabel(
        "这是一个信息面板 — 点击\"显示/隐藏信息面板\"可以切换它的可见性。\n"
        "面板被隐藏后内存不会被释放，再次显示时内容不变。\n"
        "isVisible() = true 表示控件在屏幕上可见。");
    m_infoPanel->setStyleSheet(
        "background-color: #E8F5E9;"
        "border: 1px solid #81C784;"
        "border-radius: 4px;"
        "padding: 10px;"
        "color: #2E7D32;"
        "font-size: 12px;");
    m_infoPanel->setWordWrap(true);
    mainLayout->addWidget(m_infoPanel);

    // ---- Z 序演示区域（raise / lower）----
    auto *zContainer = new QWidget();
    zContainer->setFixedHeight(60);
    zContainer->setStyleSheet("position: relative;");

    // 蓝色标签（前）
    auto *frontLabel = new QLabel("蓝色标签（前）", zContainer);
    frontLabel->setGeometry(30, 10, 200, 40);
    frontLabel->setStyleSheet(
        "background-color: #BBDEFB; border: 1px solid #64B5F6;"
        "border-radius: 4px; padding: 5px; color: #1565C0;");
    frontLabel->raise();

    // 红色标签（后）— raise/lower 的操作对象
    m_backLabel = new QLabel("红色标签（后）— 操作目标", zContainer);
    m_backLabel->setGeometry(80, 10, 240, 40);
    m_backLabel->setStyleSheet(
        "background-color: #FFCDD2; border: 1px solid #EF9A9A;"
        "border-radius: 4px; padding: 5px; color: #C62828;");

    mainLayout->addWidget(zContainer);

    // ---- 窗口标志组 ----
    auto *flagGroup = new QGroupBox("窗口标志");
    auto *flagLayout = new QHBoxLayout(flagGroup);

    auto *topmostBtn = new QPushButton("置顶切换");
    topmostBtn->setToolTip("切换 WindowStaysOnTopHint");
    connect(topmostBtn, &QPushButton::clicked, this, [this]() {
        Qt::WindowFlags flags = windowFlags();
        flags ^= Qt::WindowStaysOnTopHint;
        setWindowFlags(flags);
        show();  // setWindowFlags 后需要重新 show
    });

    auto *framelessBtn = new QPushButton("无边框切换");
    framelessBtn->setToolTip("切换 FramelessWindowHint（会闪烁）");
    connect(framelessBtn, &QPushButton::clicked, this, [this]() {
        Qt::WindowFlags flags = windowFlags();
        flags ^= Qt::FramelessWindowHint;
        setWindowFlags(flags);
        show();
    });

    auto *toolBtn = new QPushButton("工具窗口切换");
    toolBtn->setToolTip("切换 Qt::Tool 窗口类型");
    connect(toolBtn, &QPushButton::clicked, this, [this]() {
        Qt::WindowFlags flags = windowFlags();
        flags ^= Qt::Tool;
        setWindowFlags(flags);
        show();
    });

    flagLayout->addWidget(topmostBtn);
    flagLayout->addWidget(framelessBtn);
    flagLayout->addWidget(toolBtn);
    mainLayout->addWidget(flagGroup);

    // ---- 尺寸策略组 ----
    auto *sizeGroup = new QGroupBox("尺寸策略");
    auto *sizeLayout = new QHBoxLayout(sizeGroup);

    auto *minSizeBtn = new QPushButton("设置最小尺寸 500x350");
    connect(minSizeBtn, &QPushButton::clicked, this, [this]() {
        static bool set = false;
        set = !set;
        if (set) {
            setMinimumSize(500, 350);
        } else {
            setMinimumSize(0, 0);
        }
        updateSizeLabel();
    });

    auto *fixedSizeBtn = new QPushButton("固定尺寸 600x400");
    connect(fixedSizeBtn, &QPushButton::clicked, this, [this]() {
        static bool set = false;
        set = !set;
        if (set) {
            setFixedSize(600, 400);
        } else {
            setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            setMinimumSize(0, 0);
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            resize(720, 520);
        }
        updateSizeLabel();
    });

    m_sizeLabel = new QLabel();
    m_sizeLabel->setStyleSheet("color: #666; font-size: 12px;");
    updateSizeLabel();

    sizeLayout->addWidget(minSizeBtn);
    sizeLayout->addWidget(fixedSizeBtn);
    sizeLayout->addWidget(m_sizeLabel);
    mainLayout->addWidget(sizeGroup);

    // ---- 底部状态 ----
    auto *statusLabel = new QLabel(
        "提示: 窗口属性和标志的修改会即时生效 | "
        "无边框模式下可拖拽窗口移动");
    statusLabel->setStyleSheet("color: #999; font-size: 11px;");
    mainLayout->addWidget(statusLabel);
}

void DraggableWidget::updateSizeLabel()
{
    QString info = QString("当前: %1 x %2").arg(width()).arg(height());
    QSize minSz = minimumSize();
    QSize maxSz = maximumSize();
    if (minSz.width() > 0 || minSz.height() > 0) {
        info += QString(" | 最小: %1 x %2")
                    .arg(minSz.width()).arg(minSz.height());
    }
    if (maxSz.width() < QWIDGETSIZE_MAX || maxSz.height() < QWIDGETSIZE_MAX) {
        info += QString(" | 最大: %1 x %2")
                    .arg(maxSz.width()).arg(maxSz.height());
    }
    m_sizeLabel->setText(info);
}
