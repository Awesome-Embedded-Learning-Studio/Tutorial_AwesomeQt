/// @file    main.cpp
/// @brief   QAbstractItemView 基类进阶演示程序入口。
///
/// 展示拖拽排序（DragDropListView）和持久编辑器（PersistentEditorDelegate）
/// 两个核心知识点。窗口分为上下两个区域：上方为可拖拽排序的任务列表，
/// 下方为带持久进度条的下载管理列表。
///
/// 对应教程：进阶层 03-QtWidgets/15-QAbstractItemView 基类进阶。

#include "drag_drop_list_view.h"
#include "persistent_editor_delegate.h"

#include <QApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

/// @brief 创建"下载管理器"演示面板，展示 openPersistentEditor + 自定义委托。
/// @return 包含完整演示控件的 QWidget 指针。
static QWidget* createDownloadPanel()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    // 标题
    auto* title = new QLabel(
        QStringLiteral("Persistent Editor - openPersistentEditor + QProgressBar delegate"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 13px;"));

    // 进度列表
    auto* listView = new QListView;
    auto* model = new QStandardItemModel(listView);

    // 模拟 6 个下载任务，初始进度各不相同
    const int kProgressValues[] = {100, 75, 42, 18, 5, 0};
    const QString kFileNames[] = {
        QStringLiteral("qt-base-6.9.1.tar.xz"),
        QStringLiteral("awesome-library.dll"),
        QStringLiteral("tutorial-video.mp4"),
        QStringLiteral("dataset-v2.csv"),
        QStringLiteral("plugin-pack.zip"),
        QStringLiteral("backup-2026-05-18.tar.gz"),
    };

    for (int i = 0; i < 6; ++i) {
        auto* item = new QStandardItem(kFileNames[i]);
        item->setData(kProgressValues[i], Qt::DisplayRole);
        item->setEditable(false);
        model->appendRow(item);
    }

    listView->setModel(model);

    // 安装自定义委托——将单元格渲染为进度条
    auto* delegate = new PersistentEditorDelegate(listView);
    listView->setItemDelegate(delegate);

    // 为每一行打开持久编辑器，进度条将始终可见
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex idx = model->index(row, 0);
        listView->openPersistentEditor(idx);
    }

    // "模拟下载进度"按钮——随机推进一个任务的进度
    auto* advanceBtn = new QPushButton(QStringLiteral("Simulate Download Progress"));

    QObject::connect(advanceBtn, &QPushButton::clicked, listView, [model, listView]() {
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex idx = model->index(row, 0);
            int current = idx.data(Qt::DisplayRole).toInt();
            if (current < 100) {
                // 随机增加 5~20 的进度
                int increment = 5 + (std::rand() % 16);
                int newVal = qMin(current + increment, 100);
                model->setData(idx, newVal, Qt::DisplayRole);
                break;  // 每次只推进一个任务
            }
        }
    });

    layout->addWidget(title);
    layout->addWidget(listView, 1);
    layout->addWidget(advanceBtn);

    return container;
}

/// @brief 创建"拖拽排序"演示面板。
/// @return 包含 DragDropListView 的 QWidget 指针。
static QWidget* createDragDropPanel()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* title = new QLabel(
        QStringLiteral("Drag & Drop Reorder - InternalMove mode"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 13px;"));

    auto* hint = new QLabel(
        QStringLiteral("Drag items to reorder. Uses setDragEnabled + setAcceptDrops "
                       "+ InternalMove + MoveAction."));
    hint->setWordWrap(true);

    auto* listView = new DragDropListView;

    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addWidget(listView, 1);

    return container;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 主窗口用 QSplitter 上下分割两个演示面板
    auto* splitter = new QSplitter(Qt::Vertical);

    splitter->addWidget(createDragDropPanel());
    splitter->addWidget(createDownloadPanel());

    splitter->setWindowTitle(QStringLiteral("QAbstractItemView Advanced Demo"));
    splitter->resize(500, 700);
    splitter->show();

    return app.exec();
}
