// QtGui 入门示例 03: 图片显示子控件 —— 自适应缩放与绘制
#pragma once

#include <QWidget>

class ImageDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageDisplayWidget(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);
    void setFitMode(bool fit);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    /// @brief 生成默认的演示图片（棋盘格 + 渐变）
    void generateDefaultImage();

    /// @brief 在 resize 时缓存缩放结果，避免 paintEvent 中重复计算
    void updateScaledCache();

    QPixmap m_original;
    QPixmap m_scaled;
    bool m_fitMode = true;
};
