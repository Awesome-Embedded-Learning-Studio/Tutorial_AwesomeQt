#include "mainwindow.h"
#include "drawingcanvas.h"

#include <QCoreApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// ============================================================================
// 构造函数
// ============================================================================
MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Qt 事件处理演示");
    resize(700, 500);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // ---- 画板 ----
    m_canvas = new DrawingCanvas;
    // 安装事件过滤器：MainWindow 拦截画板的事件
    m_canvas->installEventFilter(this);
    mainLayout->addWidget(m_canvas, 1);

    // ---- 控制栏 ----
    auto *controlLayout = new QHBoxLayout;
    controlLayout->setSpacing(8);

    auto *clearBtn = new QPushButton("清空 (C)");
    auto *undoBtn = new QPushButton("撤销 (Ctrl+Z)");
    auto *colorBtn = new QPushButton("换色 (R)");
    auto *sendEventBtn = new QPushButton("sendEvent 测试");
    auto *postEventBtn = new QPushButton("postEvent 测试");

    controlLayout->addWidget(clearBtn);
    controlLayout->addWidget(undoBtn);
    controlLayout->addWidget(colorBtn);
    controlLayout->addStretch();
    controlLayout->addWidget(sendEventBtn);
    controlLayout->addWidget(postEventBtn);

    mainLayout->addLayout(controlLayout);

    // ---- 状态栏 ----
    m_statusLabel = new QLabel("就绪 | 画板尺寸: -- | 鼠标: -- | 线条: 0");
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "  background-color: #F0F0F0;"
        "  border: 1px solid #DDD;"
        "  border-radius: 3px;"
        "  padding: 4px 8px;"
        "  font-size: 11px;"
        "  color: #555;"
        "}");
    mainLayout->addWidget(m_statusLabel);

    // ---- 连接信号 ----
    connect(m_canvas, &DrawingCanvas::mousePositionChanged,
            this, &MainWindow::updateStatus);
    connect(m_canvas, &DrawingCanvas::canvasResized,
            this, &MainWindow::updateStatus);
    connect(clearBtn, &QPushButton::clicked,
            m_canvas, &DrawingCanvas::clearCanvas);
    connect(undoBtn, &QPushButton::clicked,
            m_canvas, &DrawingCanvas::undoLastLine);
    connect(colorBtn, &QPushButton::clicked, this, [this]() {
        // 通过 sendEvent 发送 R 键事件——同步投递
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_R, Qt::NoModifier);
        QCoreApplication::sendEvent(m_canvas, &keyPress);
    });
    connect(sendEventBtn, &QPushButton::clicked, this, [this]() {
        // sendEvent：同步投递，立即处理
        qDebug() << "[sendEvent] 同步投递 Key_C 到画板...";
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_C, Qt::NoModifier);
        QCoreApplication::sendEvent(m_canvas, &keyPress);
        qDebug() << "[sendEvent] 已完成（同步返回）";
    });
    connect(postEventBtn, &QPushButton::clicked, this, [this]() {
        // postEvent：异步投递，稍后处理
        qDebug() << "[postEvent] 异步投递 Key_C 到画板...";
        auto *keyPress = new QKeyEvent(QEvent::KeyPress, Qt::Key_C,
                                        Qt::NoModifier);
        QCoreApplication::postEvent(m_canvas, keyPress);
        qDebug() << "[postEvent] 已加入队列（异步，稍后处理）";
    });
}

// ============================================================================
// 事件过滤器
// ============================================================================
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_canvas && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        // 拦截 Ctrl+Z
        if ((keyEvent->modifiers() & Qt::ControlModifier)
            && keyEvent->key() == Qt::Key_Z) {
            qDebug() << "[EventFilter] 拦截 Ctrl+Z！执行撤销";
            m_canvas->undoLastLine();
            return true;  // 事件已消费，不传递到画板的 keyPressEvent
        }
    }
    // 其他事件不过滤
    return QWidget::eventFilter(watched, event);
}

// ============================================================================
// 私有方法
// ============================================================================
void MainWindow::updateStatus()
{
    QString sizeInfo = QString("%1 x %2")
                           .arg(m_canvas->width())
                           .arg(m_canvas->height());
    QString posInfo = QString("(%1, %2)")
                          .arg(m_canvas->mapFromGlobal(QCursor::pos()).x())
                          .arg(m_canvas->mapFromGlobal(QCursor::pos()).y());
    m_statusLabel->setText(
        QString("就绪 | 画板尺寸: %1 | 线条: %2 | 颜色: %3")
            .arg(sizeInfo)
            .arg(m_canvas->lineCount())
            .arg(m_canvas->penColor().name()));
}
