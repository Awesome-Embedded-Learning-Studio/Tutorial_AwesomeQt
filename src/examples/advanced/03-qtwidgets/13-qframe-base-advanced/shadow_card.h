/// @file    shadow_card.h
/// @brief   阴影卡片控件声明——QFrame 子类，演示自定义阴影绘制与 QGraphicsDropShadowEffect。
///
/// 对应教程：进阶层 03-QtWidgets/13-QFrame 基类进阶。

#pragma once

#include <QFrame>
#include <QPaintEvent>

/// 阴影卡片，继承 QFrame，支持手动 QPainter 阴影绘制和 QGraphicsDropShadowEffect 两种模式。
///
/// 核心知识点：
/// - Manual 模式：NoFrame 配 QPainter 手绘，背景、边框、阴影全在 paintEvent 里画
/// - DropEffect 模式：StyledPanel 配 QGraphicsDropShadowEffect，边框交还 QStyle
/// - 圆角 + 阴影 + 内容区域的协调布局
class ShadowCard : public QFrame {
    Q_OBJECT

  public:
    /// 阴影绘制模式。
    enum class ShadowMode {
        Manual,    ///< 手动 QPainter 绘制阴影
        DropEffect ///< 使用 QGraphicsDropShadowEffect
    };

    /// @brief 构造函数，配置卡片外观与阴影模式。
    /// @param[in] title    卡片标题。
    /// @param[in] desc     卡片描述文字。
    /// @param[in] mode     阴影绘制模式。
    /// @param[in] parent   父控件指针。
    explicit ShadowCard(const QString& title, const QString& desc,
                        ShadowMode mode = ShadowMode::Manual, QWidget* parent = nullptr);

    /// @brief 获取当前阴影模式。
    /// @return 当前 ShadowMode。
    ShadowMode shadowMode() const;

  protected:
    /// @brief 重写绘制——手动阴影模式下自行绘制阴影和卡片背景。
    /// @param[in] event 绘制事件。
    void paintEvent(QPaintEvent* event) override;

  private:
    /// @brief 初始化界面内容（标题 + 描述）。
    /// @param[in] title 卡片标题。
    /// @param[in] desc  卡片描述。
    void setupContent(const QString& title, const QString& desc);

    /// @brief 配置 QGraphicsDropShadowEffect 阴影。
    void setupDropShadow();

    ShadowMode m_shadowMode; // 当前阴影模式
};
