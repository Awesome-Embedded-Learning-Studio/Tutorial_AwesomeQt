/// @file    sync_scroll_widget.h
/// @brief   双面板同步滚动演示容器。
///
/// 包含两个 SyncTextArea，通过信号槽实现垂直滚动同步。
/// 演示 blockSignals 打破信号循环、同步/独立模式切换。
///
/// 对应教程：进阶层 03-QtWidgets/14-QAbstractScrollArea 基类进阶。

#pragma once

#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;
class SyncTextArea;

/// 双面板同步滚动演示控件。
///
/// 两个 SyncTextArea 并排显示，默认同步滚动。
/// 提供控制面板切换同步/独立模式、重置滚动位置、
/// 实时显示视口位置百分比。
class SyncScrollWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit SyncScrollWidget(QWidget* parent = nullptr);

private slots:
    /// @brief 当左侧面板滚动时同步右侧面板。
    /// @param[in] value 左侧面板的滚动条值。
    void onLeftScrolled(int value);

    /// @brief 当右侧面板滚动时同步左侧面板。
    /// @param[in] value 右侧面板的滚动条值。
    void onRightScrolled(int value);

    /// @brief 切换同步/独立滚动模式。
    /// @param[in] checked true 表示同步模式。
    void toggleSyncMode(bool checked);

    /// @brief 重置两个面板的滚动位置到顶部。
    void resetScroll();

private:
    /// @brief 更新底部位置百分比标签。
    void updatePositionLabel();

    SyncTextArea* m_leftArea;       // 左侧文本滚动区域
    SyncTextArea* m_rightArea;      // 右侧文本滚动区域
    QCheckBox* m_syncCheckBox;      // 同步模式复选框
    QPushButton* m_resetButton;     // 重置按钮
    QLabel* m_positionLabel;        // 位置百分比标签
    bool m_syncing;                 // 是否正在同步中（防止嵌套）
};
