/// @file    combo_item_delegate.h
/// @brief   自定义 QStyledItemDelegate，在 QComboBox 弹窗中绘制图标、文本与描述。
///
/// 对应教程：进阶层 03-QtWidgets/27-QComboBox 进阶。

#pragma once

#include <QStyledItemDelegate>

/// QComboBox 弹窗行自定义绘制委托。
///
/// 每一行从 Model 的三列中读取数据：
/// - 列 0（Qt::DisplayRole）：城市名称，左对齐粗体
/// - 列 1（Qt::DisplayRole）：城市编码，中间区域居中
/// - 列 2（Qt::DisplayRole）：地区描述，右侧右对齐
///
/// 通过 setItemDelegate 安装到 QComboBox 后，
/// 弹窗中的 QListView 会调用 paint 进行自定义渲染。
class ComboItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit ComboItemDelegate(QObject* parent = nullptr);

    /// @brief 绘制单行选项：背景 + 三列文本（名称 / 编码 / 地区）。
    /// @param[in] painter 绘图设备。
    /// @param[in] option  绘制选项（含 rect、state 等）。
    /// @param[in] index   当前项的 Model 索引。
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /// @brief 返回每行的推荐尺寸，适当加高以容纳多列内容。
    /// @param[in] option 绘制选项。
    /// @param[in] index  当前项的 Model 索引。
    /// @return 推荐的行尺寸。
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;
};
