// QtWidgets 入门示例 08: 图形视图框架基础
// MainWindow 实现

#include "mainwindow.h"
#include "interactivescene.h"
#include "graphicsview.h"

#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("图形视图框架基础演示 — 简易图形画板");
    resize(1000, 700);

    // ---- 场景与视图 ----
    m_scene = new InteractiveScene(this);
    m_view = new GraphicsView(m_scene, this);
    setCentralWidget(m_view);

    // ---- 工具栏 ----
    setupToolBar();

    // ---- 状态栏 ----
    setupStatusBar();

    // ---- 信号连接 ----
    connect(m_scene, &InteractiveScene::mouseScenePosChanged,
            this, &MainWindow::onMousePosChanged);
    connect(m_scene, &InteractiveScene::selectionCountChanged,
            this, &MainWindow::onSelectionChanged);
    // selectionChanged 是 QGraphicsScene 内置信号
    connect(m_scene, &QGraphicsScene::selectionChanged,
            this, [this]() {
                m_selectedLabel->setText(
                    "选中: " +
                    QString::number(m_scene->selectedItems().size()));
            });

    // 初始状态
    m_selectedLabel->setText("选中: 0");
    m_posLabel->setText("场景坐标: (-, -)");
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("工具");
    toolBar->setMovable(false);

    auto *addRectAction = toolBar->addAction("添加矩形");
    connect(addRectAction, &QAction::triggered,
            m_scene, &InteractiveScene::addRandomRect);

    auto *addEllipseAction = toolBar->addAction("添加椭圆");
    connect(addEllipseAction, &QAction::triggered,
            m_scene, &InteractiveScene::addRandomEllipse);

    toolBar->addSeparator();

    auto *deleteAction = toolBar->addAction("删除选中");
    connect(deleteAction, &QAction::triggered,
            m_scene, &InteractiveScene::deleteSelectedItems);

    toolBar->addSeparator();

    auto *resetViewAction = toolBar->addAction("重置视图");
    connect(resetViewAction, &QAction::triggered, this, [this]() {
        m_view->resetTransform();
    });

    // 显示图元总数
    toolBar->addSeparator();
    m_itemCountLabel = new QLabel("图元: 0");
    m_itemCountLabel->setStyleSheet("padding: 0 8px; color: #666;");
    toolBar->addWidget(m_itemCountLabel);

    // 监听场景变化来更新图元计数
    connect(m_scene, &QGraphicsScene::changed,
            this, [this]() {
                m_itemCountLabel->setText(
                    "图元: " +
                    QString::number(m_scene->items().size()));
            });
}

void MainWindow::setupStatusBar()
{
    m_posLabel = new QLabel("场景坐标: (-, -)");
    m_posLabel->setMinimumWidth(220);
    statusBar()->addWidget(m_posLabel, 1);

    m_selectedLabel = new QLabel("选中: 0");
    m_selectedLabel->setStyleSheet("padding: 0 12px;");
    statusBar()->addPermanentWidget(m_selectedLabel);

    statusBar()->showMessage("拖拽移动图元 | 橡皮带框选 | "
                              "滚轮缩放 | 双击空白处添加矩形", 5000);
}

void MainWindow::onMousePosChanged(const QPointF &scenePos)
{
    m_posLabel->setText(QString("场景坐标: (%1, %2)")
        .arg(scenePos.x(), 0, 'f', 1)
        .arg(scenePos.y(), 0, 'f', 1));
}

void MainWindow::onSelectionChanged(int count)
{
    m_selectedLabel->setText("选中: " + QString::number(count));
}
