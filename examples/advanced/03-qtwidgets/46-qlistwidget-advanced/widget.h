/// @file    widget.h
/// @brief   演示 QListWidget 拖放排序与自定义 ItemWidget。
///
/// 本示例展示：
/// - dragDropMode=InternalMove 实现拖放重新排序
/// - setItemWidget() 为每一项挂载自定义迷你卡片控件
/// - setMovement(Free) vs setMovement(Snap) 的区别
/// - currentRowChanged 信号展示选中反馈
///
/// 对应教程：进阶层 03-QtWidgets/46-qlistwidget-advanced。

#pragma once

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

/// @brief 自定义迷你卡片控件，通过 setItemWidget 挂载到 QListWidgetItem 上。
///
/// 每张卡片包含一个图标标签、标题文本和一个操作按钮，
/// 演示如何在列表项中嵌入任意 QWidget。
class CardWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建卡片布局。
    /// @param[in] icon       图标字符（如 Unicode emoji）。
    /// @param[in] title      卡片标题文本。
    /// @param[in] listWidget 所属列表控件，用于在按钮点击时定位当前行。
    /// @param[in] parent     父控件指针。
    /// @note 使用 QHBoxLayout 水平排列图标、标题、按钮，保持紧凑。
    explicit CardWidget(const QString& icon, const QString& title,
                        QListWidget* listWidget, QWidget* parent = nullptr);

private:
    /// @brief "移除"按钮点击的回调。
    /// @note 通过 sender()->parent() 向上找到 CardWidget 所在的列表行并移除。
    void onRemoveClicked();

    QListWidget* m_listWidget;  ///< 所属列表控件，用于定位行索引
    QLabel* m_titleLabel;       ///< 标题标签，用于在移除时读取名称
};

/// @brief 主窗口，包含一个支持拖放排序的 QListWidget 和状态栏。
///
/// 核心演示点：
/// - dragDropMode = InternalMove 让用户拖动列表项重新排序
/// - setItemWidget 为每个项挂载自定义 CardWidget
/// - 通过 currentRowChanged 信号在底部标签显示当前选中行信息
class Widget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化 UI 和信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit Widget(QWidget* parent = nullptr);

private:
    /// @brief 添加一张卡片到列表中。
    /// @param[in] icon  图标字符。
    /// @param[in] title 卡片标题。
    /// @note 先 addItem 创建 QListWidgetItem，再 setItemWidget 挂载自定义控件。
    void addCard(const QString& icon, const QString& title);

    /// @brief 切换 Movement 模式（Free / Snap）。
    /// @note Free 允许自由拖放位置，Snap 限制到网格对齐——对比两种行为差异。
    void toggleMovementMode();

    /// @brief 响应当前行变化，更新状态标签。
    /// @param[in] row 新选中的行索引，-1 表示无选中。
    void onCurrentRowChanged(int row);

    QListWidget* m_listWidget;   ///< 主列表控件
    QLabel* m_statusLabel;       ///< 底部状态标签，显示当前选中行
    QPushButton* m_toggleBtn;    ///< 切换 Movement 模式的按钮
    bool m_freeMovement;         ///< 当前是否为 Free 模式
};
