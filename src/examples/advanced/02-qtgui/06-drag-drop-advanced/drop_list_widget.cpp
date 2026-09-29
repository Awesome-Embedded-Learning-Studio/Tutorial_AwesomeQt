/// @file    drop_list_widget.cpp
/// @brief   DropListWidget 类实现——自定义 MIME 放下目标。
///
/// 对应教程：进阶层 02-QtGui/06-拖放系统高级用法。
/// 核心知识点：
/// - dragEnterEvent 中检查 hasFormat() 决定是否接受拖入
/// - dragMoveEvent 持续确认接受状态（防止闪烁）
/// - dropEvent 中通过 QDataStream 反序列化自定义数据

#include "drop_list_widget.h"

#include <QByteArray>
#include <QDataStream>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QIODevice>
#include <QMimeData>

static const char* kCustomMimeType = "application/x-custom-item"; ///< 与 DragListWidget 一致的 MIME 类型

DropListWidget::DropListWidget(QWidget* parent)
    : QListWidget(parent)
{
    // 接受放下操作
    setAcceptDrops(true);

    // 设置为只接收模式（不允许从本控件拖出）
    setDragDropMode(QAbstractItemView::DropOnly);

    // QListWidget 无 setPlaceholderText，通过背景提示文字引导用户
    addItem(QStringLiteral("将左侧项目拖放到此处..."));
}

void DropListWidget::dragEnterEvent(QDragEnterEvent* event)
{
    // 检查拖入的数据是否包含我们的自定义 MIME 类型
    if (event->mimeData()->hasFormat(QLatin1String(kCustomMimeType))) {
        event->acceptProposedAction();
    }
    // 不匹配则忽略 — Qt 会自动显示"禁止放下"的光标
}

void DropListWidget::dragMoveEvent(QDragMoveEvent* event)
{
    // @note 必须持续 acceptProposedAction，否则拖拽过程中
    // 控件会反复切换接受/拒绝状态，导致光标闪烁
    if (event->mimeData()->hasFormat(QLatin1String(kCustomMimeType))) {
        event->acceptProposedAction();
    }
}

void DropListWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();
    if (!mime->hasFormat(QLatin1String(kCustomMimeType))) {
        return;
    }

    // 从 MIME 数据中取出原始字节
    QByteArray encodedData = mime->data(QLatin1String(kCustomMimeType));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    // 按写入顺序反序列化：先 QString 再 int
    QString itemText;
    int     rowIndex = 0;
    stream >> itemText >> rowIndex;

    // 将解码后的数据显示到列表，附带原始行号信息
    QString displayText = QStringLiteral("[来源行 %1] %2")
                              .arg(rowIndex)
                              .arg(itemText);
    addItem(displayText);

    event->acceptProposedAction();
    emit itemDropped(itemText);
}
