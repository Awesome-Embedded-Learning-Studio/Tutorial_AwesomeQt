// QtWidgets 入门示例 12: QAbstractButton 按钮基类机制
// 演示：setCheckable / setChecked / setAutoRepeat 核心属性
//       clicked / toggled / pressed / released 四个信号
//       QButtonGroup 单选互斥管理
//       继承 QAbstractButton 自定义圆形按钮

#pragma once

#include <QWidget>

class QPushButton;
class QLabel;
class QButtonGroup;

// ============================================================================
// DemoWindow: 主演示窗口
// ============================================================================
class DemoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DemoWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 更新预览标签的字体
    void updatePreviewFont();

    /// @brief 更新计数器标签
    void updateCounterLabel();

private:
    QPushButton *m_boldBtn = nullptr;
    QPushButton *m_italicBtn = nullptr;
    QPushButton *m_underlineBtn = nullptr;
    QLabel *m_previewLabel = nullptr;
    QButtonGroup *m_buttonGroup = nullptr;
    QLabel *m_counterLabel = nullptr;
    QLabel *m_signalLog = nullptr;

    int m_counter = 0;
    int m_currentFontSize = 16;
};
