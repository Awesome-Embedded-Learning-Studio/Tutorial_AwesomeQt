/// @file    transform_widget.h
/// @brief   演示 QTransform 坐标变换的控件声明。
///
/// 展示 translate / rotate / scale 三种基本变换，并在屏幕上绘制
/// 对应的 3x3 仿射矩阵数值。使用 QPainter::save/restore 隔离
/// 各变换区域，防止状态泄漏。
///
/// 对应教程：进阶层 02-QtGui/02-坐标变换进阶。

#pragma once

#include <QMatrix3x3>
#include <QStringList>
#include <QWidget>

/// @brief 坐标变换演示控件。
///
/// 左侧绘制原始坐标系网格，右侧三个面板分别演示平移、旋转、缩放。
/// 底部显示当前变换的 3x3 矩阵数值，帮助理解变换的数学表示。
class TransformWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit TransformWidget(QWidget* parent = nullptr);

protected:
    /// @brief 重写绘制事件，完成所有自定义绘制。
    /// @param[in] event 绘制事件指针，由 Qt 事件循环分发。
    void paintEvent(QPaintEvent* event) override;

private:
    /// @brief 绘制坐标网格背景。
    /// @param[in] painter 已配置好的画笔对象。
    /// @param[in] rect    网格绘制区域。
    /// @note 使用浅灰线条绘制网格，帮助可视化坐标系原点与轴向。
    void drawGrid(QPainter& painter, const QRectF& rect);

    /// @brief 绘制一个示例图形（矩形 + 对角线 + 中心圆点）。
    /// @param[in] painter 已配置好的画笔对象。
    /// @param[in] size    图形尺寸（宽高相同）。
    /// @note 所有图形以 (0,0) 为中心绘制，方便观察变换效果。
    void drawShape(QPainter& painter, double size);

    /// @brief 从 QTransform 提取 3x3 矩阵并格式化为字符串列表。
    /// @param[in] transform 要提取的变换矩阵。
    /// @return 包含 3 行文本的列表，每行显示矩阵的一行。
    /// @note 仿射变换第三列前两个元素始终为 0，第三行为 [0, 0, 1]。
    QStringList formatMatrix(const QTransform& transform);

    /// @brief 绘制矩阵数值文本。
    /// @param[in] painter   已配置好的画笔对象。
    /// @param[in] transform 要显示的变换矩阵。
    /// @param[in] position  文本绘制位置（左上角）。
    void drawMatrixText(QPainter& painter, const QTransform& transform,
                        const QPointF& position);

    /// @brief 绘制原始参考图形（无变换），作为对比基准。
    /// @param[in] painter 已配置好的画笔对象。
    /// @param[in] rect    分配给原始图形的绘制区域。
    void drawOriginal(QPainter& painter, const QRectF& rect);

    /// @brief 演示平移变换。
    /// @param[in] painter 已配置好的画笔对象。
    /// @param[in] rect    分配给平移演示的绘制区域。
    /// @note 平移是最基本的刚体变换，只改变位置不改变形状和朝向。
    void drawTranslate(QPainter& painter, const QRectF& rect);

    /// @brief 演示旋转变换。
    /// @param[in] painter 已配置好的画笔对象。
    /// @param[in] rect    分配给旋转演示的绘制区域。
    /// @note 旋转以图形中心为原点，正值角度为顺时针方向。
    void drawRotate(QPainter& painter, const QRectF& rect);

    /// @brief 演示缩放变换。
    /// @param[in] painter 已配置好的画笔对象。
    /// @param[in] rect    分配给缩放演示的绘制区域。
    /// @note 非等比缩放会改变图形的长宽比，负值会产生镜像效果。
    void drawScale(QPainter& painter, const QRectF& rect);

    /// @brief 绘制面板标题和分隔线。
    /// @param[in] painter 已配置好的画笔对象。
    /// @param[in] rect    面板区域。
    /// @param[in] title   标题文本。
    void drawPanelHeader(QPainter& painter, const QRectF& rect,
                         const QString& title);

    static constexpr double kShapeSize = 40.0;   ///< 示例图形的基础尺寸
    static constexpr double kGridSpacing = 20.0;  ///< 网格线间距
    static constexpr int kRotationAngle = 30;      ///< 旋转演示的角度（度）
    static constexpr double kScaleX = 1.5;         ///< 缩放演示的 X 比例
    static constexpr double kScaleY = 0.7;         ///< 缩放演示的 Y 比例
    static constexpr int kTranslateX = 50;         ///< 平移的 X 偏移量
    static constexpr int kTranslateY = 30;         ///< 平移的 Y 偏移量
};
