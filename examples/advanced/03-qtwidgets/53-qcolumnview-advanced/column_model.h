/// @file    column_model.h
/// @brief   演示 QColumnView 自定义列宽与预览组件。
///
/// 对应教程：进阶层 03-QtWidgets/53-QColumnView 高级用法。
/// 本示例展示如何用 QColumnView 展示层级数据（文件系统分类），
/// 设置自定义列宽以及通过 setPreviewWidget 显示选中项的详细信息。

#pragma once

#include <QColumnView>
#include <QLabel>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWidget>

/// @brief 预览面板，在 QColumnView 最右列显示选中项的详细描述。
///
/// 当用户在 QColumnView 中选中某个条目时，此面板会更新显示该条目的
/// 名称、类型和描述信息。
class PreviewPanel : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化标签布局。
    /// @param[in] parent 父控件
    explicit PreviewPanel(QWidget* parent = nullptr);

public slots:
    /// @brief 当选中项变化时更新预览内容。
    /// @param[in] index 当前选中的模型索引
    void onSelectionChanged(const QModelIndex& index);

private:
    /// @brief 根据类型返回对应的描述文本。
    /// @param[in] type 分类类型字符串
    /// @return 描述文本
    QString descriptionForType(const QString& type) const;

    QLabel* m_titleLabel;  ///< 标题标签
    QLabel* m_typeLabel;   ///< 类型标签
    QLabel* m_descLabel;   ///< 描述标签
};

/// @brief 自定义 QColumnView，展示文件系统分类的层级结构。
class ColumnViewWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，组装模型、视图和预览面板。
    /// @param[in] parent 父控件
    explicit ColumnViewWindow(QWidget* parent = nullptr);

private:
    /// @brief 构建文件系统分类的层级数据模型。
    void buildModel();

    /// @brief 递归添加子节点到模型。
    /// @param[in] parent 父节点
    /// @param[in] children 子节点信息列表
    void addChildren(QStandardItem* parent,
                     const std::vector<std::pair<QString, QString>>& children);

    QStandardItemModel* m_model; ///< 数据模型
    PreviewPanel* m_preview;     ///< 预览面板
};
