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

#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QMainWindow>

class QMediaPlayer;
class QAudioOutput;
class QVideoWidget;
class QSlider;
class QLabel;

class PlayerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlayerWindow(QWidget *parent = nullptr);

private:
    QMediaPlayer *player_;
    QAudioOutput *audio_output_;
    QVideoWidget *video_widget_;
    QSlider *progress_slider_;
    QLabel *time_label_;
    QLabel *error_label_;
};

#endif // PLAYERWINDOW_H
