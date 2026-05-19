/// @file    checkable_group_box.h
/// @brief   演示 QGroupBox 的 paintEvent 重写、checkable 子树禁用传播与 changeEvent 拦截。
///
/// 对应教程：进阶层 03-QtWidgets/38-QGroupBox 进阶。

#pragma once

#include <QGroupBox>

class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;

/// 自定义风格的 checkable QGroupBox。
///
/// 展示四个核心知识点：
/// - 重写 paintEvent 实现自定义标题绘制（彩色标题 + 自定义对齐）
/// - setCheckable(true) 与子控件禁用级联机制
/// - 重写 changeEvent 拦截 QEvent::EnabledChange
/// - toggled 信号槽处理焦点转移与子控件状态保留
class CheckableGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit CheckableGroupBox(QWidget* parent = nullptr);

protected:
    /// @brief 自定义绘制——渐变边框 + 彩色标题背景条覆盖边框线。
    /// @param[in] event 绘制事件。
    /// @note 重写 paintEvent 后 QSS 的 border 属性不再生效，需自行绘制所有视觉元素。
    void paintEvent(QPaintEvent* event) override;

    /// @brief 拦截 EnabledChange 事件，在启用状态变化时输出调试信息。
    /// @param[in] event 状态变更事件。
    /// @note changeEvent 在 setEnabled 调用后触发，可在此拦截多种状态变化。
    void changeEvent(QEvent* event) override;

private:
    /// @brief 创建一组子控件用于演示 checkable 禁用级联。
    void setupChildWidgets();

    /// @brief 处理 toggled 信号——未勾选时转移焦点，保留子控件内容。
    /// @param[in] checked 当前是否勾选。
    /// @note 取消勾选时必须主动 setFocus，否则子控件在 disabled 状态下仍可能持有焦点。
    void onToggled(bool checked);

private:
    QLineEdit* m_nameEdit;       // 名称输入框
    QSpinBox* m_portSpin;        // 端口号微调框
    QComboBox* m_protocolCombo;  // 协议选择下拉框
    QLabel* m_statusLabel;       // 状态显示标签
};
