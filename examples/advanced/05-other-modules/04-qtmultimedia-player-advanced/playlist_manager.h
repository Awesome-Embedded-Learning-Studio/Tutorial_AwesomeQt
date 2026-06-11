/// @file    playlist_manager.h
/// @brief   播放列表管理器，支持顺序/循环/随机播放模式。
///
/// 对应教程：进阶层 05-其他模块/04-QtMultimedia 高级。
/// 使用 QList<QUrl> 管理播放列表，支持多种播放模式的切换与曲目导航。

#pragma once

#include <QList>
#include <QObject>
#include <QUrl>

/// @brief 播放列表的播放模式枚举。
enum class PlayMode
{
    kSequential,  ///< 顺序播放，播完最后一首停止
    kLoop,        ///< 列表循环，最后一首之后回到第一首
    kShuffle      ///< 随机播放，每次下一首随机选取
};

/// @brief 管理播放列表的曲目集合与播放顺序。
///
/// 提供曲目增删、上下曲切换、播放模式控制等功能。
/// 内部使用 QList<QUrl> 存储曲目 URL，不依赖 QMediaPlaylist
/// （QMediaPlaylist 在 Qt 6 中被移至 QtMultimediaWidgets 模块），
/// 以保持轻量并演示列表管理的核心逻辑。
class PlaylistManager : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit PlaylistManager(QObject* parent = nullptr);

    /// @brief 向播放列表末尾添加一首曲目。
    /// @param[in] url 曲目的文件 URL。
    /// @return 添加后该曲目在列表中的索引。
    int addTrack(const QUrl& url);

    /// @brief 跳转到下一曲。
    /// @note 行为取决于当前 PlayMode：
    ///       - kSequential：到达末尾后停在最后一首
    ///       - kLoop：到达末尾后回到第一首
    ///       - kShuffle：随机选取下一首
    void next();

    /// @brief 跳转到上一曲。
    /// @note 各模式下均回退到前一索引；索引为 0 时回到最后一首（循环/随机）
    ///       或停在第一首（顺序模式）。
    void previous();

    /// @brief 直接设置当前播放索引。
    /// @param[in] index 目标索引，必须在有效范围内。
    void setCurrentIndex(int index);

    /// @brief 设置播放模式。
    /// @param[in] mode 目标播放模式。
    void setPlayMode(PlayMode mode);

    /// @brief 获取当前播放索引。
    /// @return 当前索引，空列表时返回 -1。
    [[nodiscard]] int currentIndex() const;

    /// @brief 获取当前曲目的 URL。
    /// @return 当前 URL，列表为空时返回空 QUrl。
    [[nodiscard]] QUrl currentUrl() const;

    /// @brief 获取列表中的曲目数量。
    /// @return 曲目总数。
    [[nodiscard]] int trackCount() const;

    /// @brief 获取当前播放模式。
    /// @return 当前 PlayMode 枚举值。
    [[nodiscard]] PlayMode playMode() const;

signals:
    /// @brief 当前曲目发生变化时发射。
    /// @param[out] index 新曲目的索引。
    /// @param[out] url 新曲目的 URL。
    void currentTrackChanged(int index, const QUrl& url);

private:
    /// @brief 在随机模式下选取一个不同于当前的索引。
    /// @return 随机选取的索引；列表不足 2 首时返回当前索引。
    /// @note 使用 QRandomGenerator 生成随机数，避免连续播放同一首。
    int pickShuffleIndex() const;

    QList<QUrl> m_tracks;      ///< 曲目 URL 列表
    int m_currentIndex;         ///< 当前播放索引，-1 表示无曲目
    PlayMode m_playMode;        ///< 当前播放模式
};
