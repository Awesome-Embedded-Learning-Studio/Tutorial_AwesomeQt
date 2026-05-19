/// @file    dynamic_radio_group.h
/// @brief   演示 QButtonGroup 的 exclusive 模式、动态增删按钮与信号差异。
///
/// 对应教程：进阶层 03-QtWidgets/19-QRadioButton 进阶。

#pragma once

#include <QWidget>

class QButtonGroup;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;

/// QRadioButton 进阶用法演示控件。
///
/// 展示四个核心知识点：
/// - QButtonGroup::setExclusive(true/false) 与 QRadioButton::autoExclusive 的独立性
/// - 运行时动态添加/移除 QRadioButton 的状态管理
/// - idClicked(int) vs buttonClicked(QAbstractButton*) 的触发条件差异
/// - 两个独立的 exclusive 组互不干扰
class DynamicRadioGroup : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit DynamicRadioGroup(QWidget* parent = nullptr);

private:
    /// @brief 创建"设备选择"动态单选组演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createDeviceSection();

    /// @brief 创建两个独立的 exclusive 组互不干扰演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createDualGroupSection();

    /// @brief 创建 idClicked vs buttonClicked 对比演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createSignalCompareSection();

    /// @brief 动态添加一个设备选项按钮。
    void addDevice();

    /// @brief 动态移除当前选中的设备选项按钮。
    void removeDevice();

    /// @brief 切换设备组的 exclusive 属性。
    void toggleExclusive();

    /// @brief 更新设备信息显示标签。
    void updateDeviceInfo();

    // --- 设备选择区成员 ---
    QVBoxLayout* m_deviceLayout;           // 设备按钮所在布局
    QButtonGroup* m_deviceGroup;           // 设备单选组
    QLabel* m_deviceInfoLabel;             // 当前选中设备信息
    QLineEdit* m_deviceNameInput;          // 新设备名称输入框
    QPushButton* m_addDeviceBtn;           // 添加设备按钮
    QPushButton* m_removeDeviceBtn;        // 移除设备按钮
    QPushButton* m_toggleExclusiveBtn;     // 切换 exclusive 按钮
    int m_nextDeviceId;                    // 下一个设备按钮的 ID
    bool m_deviceGroupExclusive;           // 设备组当前是否 exclusive

    // --- 双组演示成员 ---
    QButtonGroup* m_groupA;                // 独立单选组 A
    QButtonGroup* m_groupB;                // 独立单选组 B
    QLabel* m_groupALabel;                 // 组 A 选中状态标签
    QLabel* m_groupBLabel;                 // 组 B 选中状态标签

    // --- 信号对比成员 ---
    QButtonGroup* m_signalGroup;           // 信号对比用按钮组
    QLabel* m_idClickedLog;                // idClicked 信号日志
    QLabel* m_buttonClickedLog;            // buttonClicked 信号日志
    int m_idClickedCount;                  // idClicked 触发次数
    int m_buttonClickedCount;              // buttonClicked 触发次数
};
