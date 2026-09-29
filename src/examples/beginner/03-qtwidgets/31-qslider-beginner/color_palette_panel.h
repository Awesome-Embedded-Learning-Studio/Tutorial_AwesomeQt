// QtWidgets 入门示例 31: QSlider 滑动条
// 演示：水平/垂直方向
//       setRange / setValue / setSingleStep / setPageStep
//       valueChanged / sliderMoved / sliderReleased 信号区别
//       QSS 自定义滑块外观（handle / groove）

#pragma once

#include <QWidget>

class QSlider;
class QLabel;

// ============================================================================
// ColorPalettePanel: 颜色调色板
// 覆盖 QSlider 的核心用法和 QSS 自定义外观
// ============================================================================

class ColorPalettePanel : public QWidget
{
    Q_OBJECT

public:
    explicit ColorPalettePanel(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief RGB 通道值变化时更新预览
    void onColorChanged();

    /// @brief 参数变化时更新显示
    void onParamChanged();

private:
    QSlider *m_redSlider = nullptr;
    QSlider *m_greenSlider = nullptr;
    QSlider *m_blueSlider = nullptr;
    QLabel *m_redLabel = nullptr;
    QLabel *m_greenLabel = nullptr;
    QLabel *m_blueLabel = nullptr;

    QSlider *m_opacitySlider = nullptr;
    QSlider *m_brushSlider = nullptr;
    QLabel *m_opacityLabel = nullptr;
    QLabel *m_brushLabel = nullptr;

    QLabel *m_colorPreview = nullptr;
    QLabel *m_infoLabel = nullptr;
    QLabel *m_logLabel = nullptr;
};
