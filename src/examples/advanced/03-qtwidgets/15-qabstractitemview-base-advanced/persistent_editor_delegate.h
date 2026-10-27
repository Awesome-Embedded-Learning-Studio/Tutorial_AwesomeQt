/// @file    persistent_editor_delegate.h
/// @brief   为列表项提供持久进度条编辑器的委托类。
///
/// 演示 QStyledItemDelegate 的 createEditor / setEditorData / setModelData
/// 三件套配合 openPersistentEditor 的完整工作流程。
///
/// 对应教程：进阶层 03-QtWidgets/15-QAbstractItemView 基类进阶。

#pragma once

#include <QStyledItemDelegate>

/// 持久进度条委托。
///
/// createEditor 创建一个 QProgressBar 作为编辑器控件，
/// setEditorData 从模型读取进度值并更新进度条，
/// setModelData 将进度条当前值写回模型。
///
/// 配合视图的 openPersistentEditor 使用，进度条将始终显示在对应行上，
/// 不会在编辑完成后被销毁。
class PersistentEditorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit PersistentEditorDelegate(QObject* parent = nullptr);

    /// @brief 创建进度条编辑器控件。
    /// @param[in] parent 编辑器的父控件。
    /// @param[in] option 风格选项（未使用）。
    /// @param[in] index 对应的模型索引。
    /// @return 指向新建 QProgressBar 的指针。
    /// @note 返回的进度条范围为 0~100，文本显示百分比。
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    /// @brief 将模型数据同步到进度条编辑器。
    /// @param[in] editor 编辑器控件。
    /// @param[in] index 模型索引。
    /// @note 从 index 的 Qt::DisplayRole 读取整数值并设置到进度条。
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    /// @brief 将进度条当前值写回模型。
    /// @param[in] editor 编辑器控件。
    /// @param[in] model 目标模型。
    /// @param[in] index 模型索引。
    /// @note 将进度条的 value() 以 Qt::EditRole 写入模型。
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;

    /// @brief 更新编辑器的几何位置，使其填满整个单元格。
    /// @param[in] editor 编辑器控件。
    /// @param[in] option 风格选项，包含可用矩形区域。
    /// @param[in] index 模型索引（未使用）。
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;
};
