// QtWidgets 入门示例 47: QListView Model 驱动列表视图
// 演示：与 QStringListModel 配合
//       setViewMode 列表/图标模式
//       setSpacing / setGridSize 图标布局
//       自定义 ItemDelegate 改变显示样式

#include "MainWindow.h"

#include <QApplication>
#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// MainWindow: QListView 综合演示
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QListView 综合演示 — 颜色列表");
    resize(640, 480);
    initUi();
}

void MainWindow::initUi()
{
    auto *centralWidget = new QWidget;
    auto *mainHLayout = new QHBoxLayout(centralWidget);

    // ================================================================
    // 左侧：QListView
    // ================================================================
    m_listView = new QListView;
    m_listView->setSelectionMode(
        QAbstractItemView::SingleSelection);

    // 创建 Model
    const QStringList colors = {
        "Tomato",   "SteelBlue",  "SeaGreen",   "Gold",
        "Orchid",   "Coral",      "Turquoise",  "Salmon",
        "MediumPurple", "LimeGreen", "DodgerBlue", "HotPink"
    };
    m_model = new QStringListModel(colors, this);
    m_listView->setModel(m_model);

    // 安装自定义 delegate
    m_delegate = new ColorItemDelegate(m_listView);
    m_listView->setItemDelegate(m_delegate);

    // 默认列表模式
    m_listView->setViewMode(QListView::ListMode);
    m_listView->setIconSize(QSize(24, 24));
    m_listView->setUniformItemSizes(true);

    // ================================================================
    // 右侧：控制面板
    // ================================================================
    auto *controlPanel = new QWidget;
    controlPanel->setFixedWidth(200);
    auto *controlLayout = new QVBoxLayout(controlPanel);

    // 视图模式切换
    controlLayout->addWidget(new QLabel("视图模式"));
    auto *listModeBtn = new QPushButton("列表模式");
    auto *iconModeBtn = new QPushButton("图标模式");

    auto *modeLayout = new QHBoxLayout;
    modeLayout->addWidget(listModeBtn);
    modeLayout->addWidget(iconModeBtn);
    controlLayout->addLayout(modeLayout);

    // Spacing 控制
    controlLayout->addWidget(new QLabel("间距 (spacing)"));
    auto *spacingSpin = new QSpinBox;
    spacingSpin->setRange(0, 30);
    spacingSpin->setValue(0);
    controlLayout->addWidget(spacingSpin);

    // GridSize 控制
    controlLayout->addWidget(new QLabel("网格大小 (gridSize)"));
    auto *gridSpin = new QSpinBox;
    gridSpin->setRange(60, 200);
    gridSpin->setValue(100);
    gridSpin->setSingleStep(10);
    gridSpin->setEnabled(false);  // 仅图标模式可用
    controlLayout->addWidget(gridSpin);

    // 添加颜色
    controlLayout->addWidget(new QLabel("添加颜色"));
    m_colorInput = new QLineEdit;
    m_colorInput->setPlaceholderText("颜色名称如 RoyalBlue");
    auto *addBtn = new QPushButton("添加");

    auto *addLayout = new QHBoxLayout;
    addLayout->addWidget(m_colorInput, 1);
    addLayout->addWidget(addBtn);
    controlLayout->addLayout(addLayout);

    // 颜色预览
    m_previewLabel = new QLabel("选中颜色预览");
    m_previewLabel->setFixedHeight(40);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet(
        "border: 1px solid #CCC; border-radius: 4px;");
    controlLayout->addWidget(m_previewLabel);

    controlLayout->addStretch();

    // 状态标签
    m_infoLabel = new QLabel;
    updateInfoLabel();
    controlLayout->addWidget(m_infoLabel);

    // ================================================================
    // 组装水平布局
    // ================================================================
    mainHLayout->addWidget(m_listView, 1);
    mainHLayout->addWidget(controlPanel);

    setCentralWidget(centralWidget);

    // ================================================================
    // 信号连接
    // ================================================================
    // 切换列表模式
    connect(listModeBtn, &QPushButton::clicked, this, [this]() {
        m_listView->setViewMode(QListView::ListMode);
        m_listView->setIconSize(QSize(24, 24));
        m_listView->setGridSize(QSize());
        m_listView->setUniformItemSizes(true);
        static_cast<QSpinBox *>(
            m_colorInput->parentWidget()->findChild<QSpinBox *>()
            )->setEnabled(false);
    });

    // 切换图标模式
    connect(iconModeBtn, &QPushButton::clicked, this, [this, gridSpin]() {
        m_listView->setViewMode(QListView::IconMode);
        m_listView->setIconSize(QSize(64, 64));
        m_listView->setGridSize(QSize(gridSpin->value(),
                                      gridSpin->value()));
        m_listView->setUniformItemSizes(false);
        gridSpin->setEnabled(true);
    });

    // Spacing 变化
    connect(spacingSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) {
        m_listView->setSpacing(value);
    });

    // GridSize 变化
    connect(gridSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) {
        if (m_listView->viewMode() == QListView::IconMode) {
            m_listView->setGridSize(QSize(value, value));
        }
    });

    // 添加颜色
    connect(addBtn, &QPushButton::clicked, this,
            &MainWindow::onAddColor);
    connect(m_colorInput, &QLineEdit::returnPressed, this,
            &MainWindow::onAddColor);

    // 选中变化时更新预览
    connect(m_listView->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this, &MainWindow::onSelectionChanged);
}

void MainWindow::onAddColor()
{
    QString name = m_colorInput->text().trimmed();
    if (name.isEmpty()) return;

    // 检查颜色名称是否有效
    QColor color(name);
    if (!color.isValid()) {
        m_colorInput->setStyleSheet(
            "border: 1px solid #E53935;");
        return;
    }
    m_colorInput->setStyleSheet("");

    // 插入到 Model 末尾
    int row = m_model->rowCount();
    m_model->insertRows(row, 1);
    QModelIndex newIndex = m_model->index(row);
    m_model->setData(newIndex, name, Qt::DisplayRole);

    m_colorInput->clear();
    m_listView->scrollToBottom();
    updateInfoLabel();
}

void MainWindow::onSelectionChanged(const QModelIndex &current)
{
    if (!current.isValid()) {
        m_previewLabel->setText("选中颜色预览");
        m_previewLabel->setStyleSheet(
            "border: 1px solid #CCC; border-radius: 4px;");
        return;
    }

    QString colorName = current.data(Qt::DisplayRole).toString();
    QColor color(colorName);

    if (color.isValid()) {
        m_previewLabel->setText(colorName);
        m_previewLabel->setStyleSheet(
            QString("background-color: %1;"
                    "border: 1px solid #CCC;"
                    "border-radius: 4px;"
                    "color: %2;")
                .arg(colorName)
                .arg(color.lightness() > 128 ? "#333" : "#FFF"));
    }
}

void MainWindow::updateInfoLabel()
{
    QString mode = (m_listView->viewMode() == QListView::ListMode)
                       ? "列表"
                       : "图标";
    m_infoLabel->setText(
        QString("条目数: %1 | 模式: %2")
            .arg(m_model->rowCount())
            .arg(mode));
}
