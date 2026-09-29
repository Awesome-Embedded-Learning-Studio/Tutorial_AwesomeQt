/// @file    drag_reorder_tab_bar.h
/// @brief   演示 QTabBar 拖拽排序、setTabButton 自定义关闭按钮、标签溢出管理。
///
/// 对应教程：进阶层 03-QtWidgets/40-QTabBar 进阶。

#pragma once

#include <QTabBar>

class QStackedWidget;
class QLabel;

/// QTabBar 进阶用法演示控件。
///
/// 展示三个核心知识点：
/// - 通过重写鼠标事件实现标签拖拽重排（mousePressEvent/mouseMoveEvent）
/// - setTabButton 在标签上嵌入自定义关闭按钮与状态指示器
/// - selectionBehaviorOnRemove 配置删除标签后的自动选择策略
class DragReorderTabBar : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit DragReorderTabBar(QWidget* parent = nullptr);

protected:
    /// @brief 鼠标按下事件，记录拖拽起始位置和标签索引。
    /// @param[in] event 鼠标事件对象。
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief 鼠标移动事件，计算目标索引并调用 moveTab 实现实时重排。
    /// @param[in] event 鼠标事件对象。
    /// @note 拖拽超过 QApplication::startDragDistance() 后才开始重排。
    void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief 鼠标释放事件，结束拖拽状态。
    /// @param[in] event 鼠标事件对象。
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    /// @brief 创建初始标签页（5 个文档标签）。
    void setupInitialTabs();

    /// @brief 为指定标签添加自定义关闭按钮（右侧）和状态指示点（左侧）。
    /// @param[in] index 目标标签索引。
    void attachTabButtons(int index);

    /// @brief 移除指定标签并同步 QStackedWidget 页面。
    /// @param[in] index 要移除的标签索引。
    void removeTabAtIndex(int index);

    /// @brief 响应标签移动信号，同步 QStackedWidget 的页面顺序。
    /// @param[in] from 移动前的索引。
    /// @param[in] to   移动后的索引。
    void syncStackedWidget(int from, int to);

    /// @brief 更新底部信息标签，显示当前标签索引和总数。
    void updateInfoLabel();

private:
    QTabBar* m_tabBar;              // 独立标签栏（非 QTabWidget 内部的）
    QStackedWidget* m_stackedWidget;// 配合标签栏的堆叠页面容器
    QLabel* m_infoLabel;            // 底部状态信息

    // 拖拽状态
    bool m_dragging;                // 是否处于拖拽中
    int m_dragStartIndex;           // 拖拽起始标签索引
    QPoint m_dragStartPos;          // 拖拽起始鼠标位置
};
