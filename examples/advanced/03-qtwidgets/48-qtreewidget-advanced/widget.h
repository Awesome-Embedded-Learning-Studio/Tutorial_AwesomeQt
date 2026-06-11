/// @file    widget.h
/// @brief   演示 QTreeWidget 延迟加载子节点（懒加载）。
///
/// 本示例展示：
/// - 模拟文件系统目录树，顶层节点代表目录
/// - 使用 itemExpanded 信号在首次展开时动态填充子节点
/// - 占位 "Loading..." 子项指示节点可展开
/// - 避免一次性构造整棵树，减少启动时间和内存占用
///
/// 对应教程：进阶层 03-QtWidgets/48-qtreewidget-advanced。

#pragma once

#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

/// @brief 主窗口，包含一个模拟文件系统目录树的 QTreeWidget。
///
/// 核心演示点：
/// - 顶层目录项带有占位 "Loading..." 子项以显示展开箭头
/// - itemExpanded 信号触发时检测是否首次展开，动态加载真实子节点
/// - 不同目录类型（文件夹、文档等）使用不同图标标记
class Widget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化目录树和信号连接。
    /// @param[in] parent 父控件指针。
    explicit Widget(QWidget* parent = nullptr);

private:
    /// @brief 创建顶层目录项并附加 "Loading..." 占位子项。
    /// @param[in] name 目录名称。
    /// @param[in] type 目录类型标记（用于生成子节点内容）。
    /// @return 创建的顶层 QTreeWidgetItem 指针。
    /// @note 占位子项使得顶层项显示展开箭头，即使真实子项尚未加载。
    QTreeWidgetItem* createTopLevelItem(const QString& name, const QString& type);

    /// @brief 为指定目录项动态加载子节点。
    /// @param[in] item 被展开的目录项。
    /// @note 检查第一个子项是否为占位项（text=kPlaceholder），
    ///       若是则移除占位项并填充真实子节点。
    void loadChildren(QTreeWidgetItem* item);

    /// @brief 模拟为目录生成子文件/子文件夹列表。
    /// @param[in] dirName 目录名。
    /// @param[in] dirType 目录类型。
    /// @return 子项名称列表。
    /// @note 在真实应用中，此处应替换为实际的文件系统遍历。
    QStringList simulateChildEntries(const QString& dirName,
                                     const QString& dirType) const;

    /// @brief 响应 itemExpanded 信号。
    /// @param[in] item 被展开的项。
    void onItemExpanded(QTreeWidgetItem* item);

    /// @brief 响应 currentItemChanged 信号，更新状态标签。
    /// @param[in] current 当前选中项。
    /// @param[in] previous 之前选中项。
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    QTreeWidget* m_treeWidget;   ///< 目录树控件
    QLabel* m_statusLabel;       ///< 底部状态标签

    static constexpr const char* kPlaceholder = "__loading__";  ///< 占位子项的文本标记
};
