/// @file    splash_demo.h
/// @brief   演示 QSplashScreen 渐变消隐动画与最小显示时长保证。
///
/// 对应教程：进阶层 03-QtWidgets/70-QSplashScreen 进阶。
/// 核心知识点：QPainter 绘制启动画面、QPropertyAnimation 透明度渐变消隐、
///             QElapsedTimer 保证最小展示时长。

#pragma once

#include <QElapsedTimer>
#include <QWidget>

class QLabel;
class QPushButton;
class QSplashScreen;

/// @brief 主窗口，配合 QSplashScreen 演示启动画面与渐变消隐。
class SplashDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit SplashDemo(QWidget* parent = nullptr);

public slots:
    /// @brief 重新展示启动画面，模拟完整加载流程。
    /// @note 此槽函数用于演示目的，允许用户反复查看启动画面效果。
    ///       同时被 main.cpp 中的 QTimer::singleShot 调用以在启动时展示。
    void restartSplash();

private:
    /// @brief 初始化界面布局。
    void setupUI();

    /// @brief 生成启动画面的 QPixmap。
    /// @param[in] width  画面宽度。
    /// @param[in] height 画面高度。
    /// @return 绘制完成的 QPixmap。
    /// @note 使用 QPainter 在 QPixmap 上绘制文字和渐变背景，
    ///       避免依赖外部图片文件。
    static QPixmap generateSplashPixmap(int width, int height);

    /// @brief 执行启动画面的显示、加载步骤模拟和渐变消隐。
    /// @param[in] splash 指向已创建的 QSplashScreen 对象。
    /// @note 使用 QTimer 分步模拟加载，完成后通过 QPropertyAnimation
    ///       对窗口 opacity 做渐变消隐，并保证总时长不少于 kMinimumMs。
    void runSplashSequence(QSplashScreen* splash);

    QLabel* m_infoLabel;  ///< 信息展示标签
};

/// @brief 启动画面最小展示时长（毫秒）。
constexpr int kMinimumSplashMs = 3000;

/// @brief 启动画面每步加载间隔（毫秒）。
constexpr int kSplashStepIntervalMs = 600;
