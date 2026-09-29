/**
 * QtMultimedia 音视频播放基础示例
 *
 * 本示例演示 QtMultimedia 模块的播放管线核心用法：
 * 1. QMediaPlayer + QAudioOutput + QVideoWidget 播放管线组装
 * 2. play() / pause() / stop() / setPosition() 播放控制
 * 3. playbackStateChanged / errorOccurred 状态信号处理
 * 4. QSlider 进度条双向绑定
 * 5. 音量控制与文件选择
 */

#include "PlayerWindow.h"

#include <QAudioOutput>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSlider>
#include <QStyle>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QWidget>

#include <QTime>

/// 将毫秒转换为 "mm:ss" 格式的字符串
static QString formatTime(qint64 milliseconds)
{
    qint64 seconds = milliseconds / 1000;
    int minutes = static_cast<int>(seconds / 60);
    int secs = static_cast<int>(seconds % 60);
    return QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'));
}

// ============================================================================
// 主窗口：简易视频播放器
// ============================================================================
PlayerWindow::PlayerWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QtMultimedia 音视频播放示例");
    resize(800, 600);

    // ---- 播放管线组装 ----
    player_ = new QMediaPlayer(this);
    audio_output_ = new QAudioOutput(this);
    video_widget_ = new QVideoWidget(this);

    player_->setAudioOutput(audio_output_);
    player_->setVideoOutput(video_widget_);
    audio_output_->setVolume(0.8);

    // ---- 控制面板 ----
    auto *open_button = new QPushButton("打开文件", this);
    auto *play_button = new QPushButton("播放", this);
    auto *stop_button = new QPushButton("停止", this);

    progress_slider_ = new QSlider(Qt::Horizontal, this);
    progress_slider_->setRange(0, 0);

    time_label_ = new QLabel("00:00 / 00:00", this);
    time_label_->setMinimumWidth(120);

    auto *volume_label = new QLabel("音量:", this);
    auto *volume_slider = new QSlider(Qt::Horizontal, this);
    volume_slider->setRange(0, 100);
    volume_slider->setValue(80);
    volume_slider->setMaximumWidth(120);

    error_label_ = new QLabel(this);
    error_label_->setStyleSheet("color: red;");
    error_label_->hide();

    // ---- 布局 ----
    auto *central = new QWidget(this);
    auto *main_layout = new QVBoxLayout(central);

    // 视频显示区域
    main_layout->addWidget(video_widget_, 1);

    // 进度条行
    auto *progress_layout = new QHBoxLayout();
    progress_layout->addWidget(progress_slider_, 1);
    progress_layout->addWidget(time_label_);
    main_layout->addLayout(progress_layout);

    // 控制按钮行
    auto *control_layout = new QHBoxLayout();
    control_layout->addWidget(open_button);
    control_layout->addWidget(play_button);
    control_layout->addWidget(stop_button);
    control_layout->addStretch();
    control_layout->addWidget(volume_label);
    control_layout->addWidget(volume_slider);
    main_layout->addLayout(control_layout);

    // 错误提示
    main_layout->addWidget(error_label_);

    setCentralWidget(central);

    // ---- 信号槽连接 ----

    // 打开文件
    connect(open_button, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(
            this,
            "选择媒体文件",
            {},
            "Media Files (*.mp4 *.avi *.mkv *.mp3 *.wav *.flac *.ogg);;"
            "All Files (*)");
        if (!file.isEmpty()) {
            player_->setSource(QUrl::fromLocalFile(file));
            error_label_->hide();
            setWindowTitle("QtMultimedia 音视频播放示例 - "
                           + QFileInfo(file).fileName());
        }
    });

    // 播放/暂停切换
    connect(play_button, &QPushButton::clicked, this, [this, play_button]() {
        if (player_->playbackState() == QMediaPlayer::PlayingState) {
            player_->pause();
        } else {
            player_->play();
        }
    });

    // 停止
    connect(stop_button, &QPushButton::clicked, player_, &QMediaPlayer::stop);

    // 进度条：播放位置 → 滑块
    connect(player_, &QMediaPlayer::positionChanged, this,
            [this](qint64 position) {
                if (!progress_slider_->isSliderDown()) {
                    progress_slider_->blockSignals(true);
                    progress_slider_->setValue(static_cast<int>(position));
                    progress_slider_->blockSignals(false);
                }
                time_label_->setText(
                    formatTime(position) + " / "
                    + formatTime(player_->duration()));
            });

    // 进度条：拖拽 → 跳转位置
    connect(progress_slider_, &QSlider::sliderMoved, player_,
            &QMediaPlayer::setPosition);

    // 总时长变化 → 更新滑块范围
    connect(player_, &QMediaPlayer::durationChanged, this,
            [this](qint64 duration) {
                progress_slider_->setRange(0, static_cast<int>(duration));
            });

    // 音量滑块
    connect(volume_slider, &QSlider::valueChanged, this,
            [this](int value) {
                audio_output_->setVolume(value / 100.0);
            });

    // 播放状态变化 → 更新按钮文字
    connect(player_, &QMediaPlayer::playbackStateChanged, this,
            [play_button](QMediaPlayer::PlaybackState state) {
                switch (state) {
                case QMediaPlayer::PlayingState:
                    play_button->setText("暂停");
                    break;
                case QMediaPlayer::PausedState:
                    play_button->setText("播放");
                    break;
                case QMediaPlayer::StoppedState:
                    play_button->setText("播放");
                    break;
                }
            });

    // 错误处理
    connect(player_, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &error_string) {
                error_label_->setText("播放出错: " + error_string);
                error_label_->show();
            });

    // 播放结束 → 重置状态
    connect(player_, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia) {
                    progress_slider_->setValue(0);
                    time_label_->setText(
                        "00:00 / " + formatTime(player_->duration()));
                }
            });
}
