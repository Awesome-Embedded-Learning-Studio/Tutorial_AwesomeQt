#pragma once

// QtWidgets 入门示例 13: QFrame 可视框架基类
// 演示：QFrame::Shape（Box / Panel / StyledPanel / HLine / VLine）
//       QFrame::Shadow（Raised / Sunken / Plain）
//       lineWidth / midLineWidth 边框宽度控制

#include <QList>
#include <QWidget>

class QFrame;
class QSpinBox;

// ============================================================================
// DemoWindow: QFrame 边框效果展示窗口
// ============================================================================
class DemoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DemoWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 更新所有演示 QFrame 的边框宽度
    void updateFrameWidths();

private:
    QList<QFrame *> m_demoFrames;
    QSpinBox *m_lineWidthSpin = nullptr;
    QSpinBox *m_midLineWidthSpin = nullptr;
};
