/// @file    drag_list_widget.cpp
/// @brief   DragListWidget 类实现——自定义 MIME 拖拽源。
///
/// 对应教程：进阶层 02-QtGui/06-拖放系统高级用法。
/// 核心知识点：
/// - startDrag() 中创建 QDrag 并设置 QMimeData
/// - 自定义 MIME 类型 "application/x-custom-item"
/// - 使用 QDataStream 对结构化数据进行序列化

#include "drag_list_widget.h"

#include <QByteArray>
#include <QDataStream>
#include <QDrag>
#include <QIODevice>
#include <QMimeData>

static const char* kCustomMimeType = "application/x-custom-item"; ///< 自定义 MIME 类型标识

DragListWidget::DragListWidget(QWidget* parent)
    : QListWidget(parent)
{
    // 启用拖拽模式：仅允许从本控件拖出
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);

    // 只允许复制动作（不改变源列表）
    setDefaultDropAction(Qt::CopyAction);

    // 添加示例数据供用户拖拽
    for (int i = 0; i < 8; ++i) {
        addItem(QStringLiteral("可拖拽项目 %1").arg(i + 1));
    }
}

void DragListWidget::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions)

    QListWidgetItem* item = currentItem();
    if (!item) {
        return;
    }

    // 使用 QDataStream 将结构化数据写入 QByteArray
    // 格式：QString(文本) + int(行号) — 演示多字段自定义协议
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << item->text();
    stream << row(item);

    // 构建 MIME 数据对象，设置自定义类型
    auto* mimeData = new QMimeData;
    mimeData->setData(QLatin1String(kCustomMimeType), encodedData);

    // 同时设置 text/plain，使控件也能被普通文本接收者接受
    mimeData->setText(item->text());

    // QDrag 是拖拽操作的核心对象，必须在堆上创建，
    // 其生命周期由 Qt 事件循环管理
    auto* drag = new QDrag(this);
    drag->setMimeData(mimeData);

    // exec() 阻塞直到拖拽操作完成（放下或取消）
    // 返回值指示最终执行的动作类型
    drag->exec(Qt::CopyAction);
}
