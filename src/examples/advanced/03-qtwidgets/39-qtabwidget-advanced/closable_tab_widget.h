/// @file    closable_tab_widget.h
/// @brief   演示 QTabWidget 可关闭标签页、QTabBar+QStackedWidget 协作机制。
///
/// 对应教程：进阶层 03-QtWidgets/39-QTabWidget 进阶。

#pragma once

#include <QTabWidget>

/// QTabWidget 进阶用法演示控件。
///
/// 展示三个核心知识点：
/// - setTabsClosable + tabCloseRequested 可关闭标签页的完整链路
/// - tabBar() 访问内部 QTabBar 进行细粒度定制（setTabButton、setIconSize）
/// - documentMode 配合 QSS 实现跨平台一致的现代标签页风格
class ClosableTabWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit ClosableTabWidget(QWidget* parent = nullptr);

private:
    /// @brief 创建包含 QTextEdit 的新标签页并添加到 QTabWidget。
    /// @note 每个新标签页标题为"未命名 N"，N 从计数器递增。
    void addNewTab();

    /// @brief 处理 tabCloseRequested 信号，安全移除指定标签页。
    /// @param[in] index 请求关闭的标签页索引。
    /// @note 先 widget(index) 取出页面指针，再 removeTab，最后 deleteLater。
    void handleCloseRequested(int index);

    /// @brief 响应标签切换，在底部状态标签显示当前文档字数统计。
    void updateStatusBar();

    /// @brief 在当前标签的 RightSide 嵌入一个 QProgressBar 演示 setTabButton。
    /// @note 展示如何通过 tabBar() 访问 QTabWidget 未直接暴露的 QTabBar 接口。
    void attachProgressButton(int index);

private:
    QTabWidget* m_tabWidget;         // 核心标签页控件
    int m_tabCounter;                // 标签命名计数器
};
