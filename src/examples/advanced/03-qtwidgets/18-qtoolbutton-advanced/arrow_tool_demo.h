/// @file    arrow_tool_demo.h
/// @brief   演示 QToolButton 的 ArrowType、PopupMode 以及 QToolBar 集成。
///
/// 对应教程：进阶层 03-QtWidgets/18-QToolButton 进阶。

#pragma once

#include <QMainWindow>
#include <QWidget>

class QLabel;
class QMenu;
class QTextEdit;
class QToolBar;
class QToolButton;

/// QToolButton 进阶用法演示主窗口。
///
/// 展示四个核心知识点：
/// - QToolButton::ArrowType 四种方向箭头按钮
/// - ToolButtonPopupMode 三种弹出模式的时序差异
/// - QToolBar 集成：addWidget vs addAction 的行为区别
/// - DelayedPopup / InstantPopup / MenuButtonPopup 的交互对比
class ArrowToolDemo : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化工具栏、中央演示区与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit ArrowToolDemo(QWidget* parent = nullptr);

private:
    /// @brief 创建顶部工具栏，包含 ArrowType 方向按钮和弹出模式按钮。
    void createToolBar();

    /// @brief 创建中央演示区，展示三种 PopupMode 的独立 QToolButton。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createPopupModeSection();

    /// @brief 在日志区追加一行文本。
    /// @param[in] message 日志消息内容。
    void appendLog(const QString& message);

    /// @brief 处理 ArrowType 方向按钮点击，在状态栏和日志中显示导航方向。
    /// @param[in] direction 方向名称（上/下/左/右）。
    void onArrowClicked(const QString& direction);

    /// @brief 处理历史记录按钮的快速点击（撤销操作）。
    void onHistoryClicked();

    /// @brief 在历史菜单弹出前动态更新最近操作记录。
    void onHistoryMenuAboutToShow();

    /// @brief 处理导出按钮主体区域的默认导出操作。
    void onExportClicked();

    /// @brief 处理菜单中"导出为 PDF"动作。
    void onExportPdf();

    /// @brief 处理菜单中"导出为 PNG"动作。
    void onExportPng();

    /// @brief 处理菜单中"导出为 SVG"动作。
    void onExportSvg();

    /// @brief 处理工具栏风格切换按钮，在 IconOnly 和 TextBesideIcon 之间切换。
    void onToggleStyle();

    // --- 工具栏成员 ---
    QToolBar* m_toolBar;                    // 顶部工具栏
    QToolButton* m_upArrowBtn;              // 上方向箭头按钮
    QToolButton* m_downArrowBtn;            // 下方向箭头按钮
    QToolButton* m_leftArrowBtn;            // 左方向箭头按钮
    QToolButton* m_rightArrowBtn;           // 右方向箭头按钮
    QToolButton* m_historyBtn;              // 历史记录按钮（DelayedPopup）
    QMenu* m_historyMenu;                   // 历史记录下拉菜单
    QToolButton* m_exportBtn;               // 导出按钮（MenuButtonPopup）
    QMenu* m_exportMenu;                    // 导出格式下拉菜单

    // --- 中央区域成员 ---
    QTextEdit* m_logEdit;                   // 操作日志显示区

    // --- 独立弹出模式演示成员 ---
    QToolButton* m_delayedBtn;              // DelayedPopup 独立演示按钮
    QToolButton* m_instantBtn;              // InstantPopup 独立演示按钮
    QToolButton* m_menuBtn;                 // MenuButtonPopup 独立演示按钮
    QLabel* m_popupInfoLabel;               // 弹出模式行为说明标签

    // --- 状态 ---
    int m_historyCount;                     // 历史记录条数计数
    bool m_iconOnlyMode;                    // 当前工具栏是否为 IconOnly 模式
};
