/// @file    splash_demo.cpp
/// @brief   SplashDemo 类的实现。
///
/// 对应教程：进阶层 03-QtWidgets/70-QSplashScreen 进阶。

#include "splash_demo.h"

#include <QApplication>
#include <QFont>
#include <QGradient>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QPainter>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSplashScreen>
#include <QTimer>
#include <QVBoxLayout>

SplashDemo::SplashDemo(QWidget* parent)
    : QWidget(parent)
    , m_infoLabel(nullptr)
{
    setupUI();
}

void SplashDemo::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    m_infoLabel = new QLabel(tr("Application loaded. Click the button to replay splash screen."), this);
    m_infoLabel->setWordWrap(true);

    auto* restartBtn = new QPushButton(tr("Replay Splash Screen"), this);

    layout->addWidget(m_infoLabel);
    layout->addWidget(restartBtn);
    layout->addStretch();

    connect(restartBtn, &QPushButton::clicked,
            this, &SplashDemo::restartSplash);
}

void SplashDemo::restartSplash()
{
    QPixmap pixmap = generateSplashPixmap(500, 300);

    // @note QSplashScreen 的父对象设为 nullptr，因为我们会在动画完成后手动删除。
    auto* splash = new QSplashScreen(pixmap);
    splash->show();

    // @note 在启动画面显示期间阻塞主窗口输入，防止用户提前操作。
    splash->setWindowModality(Qt::ApplicationModal);

    runSplashSequence(splash);
}

void SplashDemo::runSplashSequence(QSplashScreen* splash)
{
    // @note QElapsedTimer 用于追踪从启动画面显示到加载完成的总耗时，
    //       确保启动画面至少展示 kMinimumSplashMs 毫秒。
    QElapsedTimer elapsed;
    elapsed.start();

    // 模拟加载步骤的消息列表
    const QStringList loadingSteps = {
        tr("Loading core modules..."),
        tr("Initializing plugins..."),
        tr("Setting up workspace..."),
        tr("Finalizing..."),
    };

    // @note 使用 QPointer 防止 splash 在延迟回调期间被提前删除。
    // 这里不用 QPointer 因为 splash 是 new 创建的裸指针，我们手动管理。
    // 通过 QTimer::singleShot 链式调度每一步加载。
    for (int i = 0; i < loadingSteps.size(); ++i) {
        QTimer::singleShot(kSplashStepIntervalMs * (i + 1), splash, [splash, text = loadingSteps[i]]() {
            if (splash) {
                splash->showMessage(text, Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
            }
        });
    }

    // 加载步骤完成后的总耗时（所有步骤的累计时间）
    const int loadingTotalMs = kSplashStepIntervalMs * (loadingSteps.size() + 1);

    // @note 如果加载步骤总耗时 < kMinimumSplashMs，需要额外等待差值，
    //       保证用户能看到启动画面足够长时间。
    const int remainingMs = std::max(0, kMinimumSplashMs - loadingTotalMs);
    const int fadeStartMs = loadingTotalMs + remainingMs;

    // 开始渐变消隐
    QTimer::singleShot(fadeStartMs, splash, [splash, this]() {
        if (!splash) {
            return;
        }

        // @note QPropertyAnimation 对 windowOpacity 做从 1.0 到 0.0 的渐变，
        //       产生平滑的淡出效果。
        auto* fadeAnim = new QPropertyAnimation(splash, "windowOpacity");
        fadeAnim->setDuration(800);  // 淡出持续 800ms
        fadeAnim->setStartValue(1.0);
        fadeAnim->setEndValue(0.0);

        // 动画结束后清理 splash 并激活主窗口
        connect(fadeAnim, &QPropertyAnimation::finished, splash, [splash, this]() {
            splash->close();
            splash->deleteLater();
            this->activateWindow();
            this->raise();
        });

        fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

QPixmap SplashDemo::generateSplashPixmap(int width, int height)
{
    QPixmap pixmap(width, height);

    // @note 使用 QPainter 在 QPixmap 上绘制，无需外部图片文件。
    QPainter painter(&pixmap);

    // 绘制渐变背景
    QLinearGradient gradient(0, 0, 0, height);
    gradient.setColorAt(0.0, QColor(30, 60, 120));
    gradient.setColorAt(1.0, QColor(15, 30, 60));
    painter.fillRect(0, 0, width, height, gradient);

    // 绘制应用标题
    QFont titleFont("Sans Serif", 28, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(Qt::white);
    painter.drawText(QRect(0, 0, width, height / 2),
                     Qt::AlignCenter, tr("AwesomeQt App"));

    // 绘制副标题
    QFont subFont("Sans Serif", 12);
    painter.setFont(subFont);
    painter.setPen(QColor(180, 200, 230));
    painter.drawText(QRect(0, height / 2, width, height / 3),
                     Qt::AlignCenter, tr("QSplashScreen Advanced Demo"));

    painter.end();
    return pixmap;
}
