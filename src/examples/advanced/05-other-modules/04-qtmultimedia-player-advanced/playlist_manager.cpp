/// @file    playlist_manager.cpp
/// @brief   PlaylistManager 类实现 —— 播放列表管理与播放模式控制。
///
/// 对应教程：进阶层 05-其他模块/04-QtMultimedia 高级。

#include "playlist_manager.h"

#include <QRandomGenerator>

PlaylistManager::PlaylistManager(QObject* parent)
    : QObject(parent)
    , m_currentIndex(-1)
    , m_playMode(PlayMode::kSequential)
{
}

int PlaylistManager::addTrack(const QUrl& url)
{
    m_tracks.append(url);
    // 新添加的曲目索引
    int newIndex = m_tracks.size() - 1;

    // 如果是第一首曲目，自动设为当前曲目
    if (m_currentIndex == -1) {
        m_currentIndex = 0;
        emit currentTrackChanged(m_currentIndex, m_tracks.at(0));
    }

    return newIndex;
}

void PlaylistManager::next()
{
    if (m_tracks.isEmpty()) {
        return;
    }

    int nextIdx = -1;

    switch (m_playMode) {
        case PlayMode::kSequential:
            // 顺序模式：递增索引，到末尾时停在最后一首
            nextIdx = (m_currentIndex < m_tracks.size() - 1)
                          ? m_currentIndex + 1
                          : m_currentIndex;
            break;

        case PlayMode::kLoop:
            // 循环模式：到末尾后回到第一首
            nextIdx = (m_currentIndex + 1) % m_tracks.size();
            break;

        case PlayMode::kShuffle:
            // 随机模式：随机选取一首（避免连续相同）
            nextIdx = pickShuffleIndex();
            break;
    }

    m_currentIndex = nextIdx;
    emit currentTrackChanged(m_currentIndex, m_tracks.at(m_currentIndex));
}

void PlaylistManager::previous()
{
    if (m_tracks.isEmpty()) {
        return;
    }

    int prevIdx = -1;

    switch (m_playMode) {
        case PlayMode::kSequential:
            // 顺序模式：递减索引，已在第一首时不变
            prevIdx = (m_currentIndex > 0) ? m_currentIndex - 1 : 0;
            break;

        case PlayMode::kLoop:
            // 循环模式：在第一首时回到最后一首
            prevIdx = (m_currentIndex > 0)
                          ? m_currentIndex - 1
                          : m_tracks.size() - 1;
            break;

        case PlayMode::kShuffle:
            // 随机模式：上一曲同样随机选取
            prevIdx = pickShuffleIndex();
            break;
    }

    m_currentIndex = prevIdx;
    emit currentTrackChanged(m_currentIndex, m_tracks.at(m_currentIndex));
}

void PlaylistManager::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_tracks.size()) {
        return;
    }

    m_currentIndex = index;
    emit currentTrackChanged(m_currentIndex, m_tracks.at(m_currentIndex));
}

void PlaylistManager::setPlayMode(PlayMode mode)
{
    m_playMode = mode;
}

int PlaylistManager::currentIndex() const
{
    return m_currentIndex;
}

QUrl PlaylistManager::currentUrl() const
{
    if (m_currentIndex >= 0 && m_currentIndex < m_tracks.size()) {
        return m_tracks.at(m_currentIndex);
    }
    return {};
}

int PlaylistManager::trackCount() const
{
    return m_tracks.size();
}

PlayMode PlaylistManager::playMode() const
{
    return m_playMode;
}

int PlaylistManager::pickShuffleIndex() const
{
    // 只有一首或没有曲目时无法随机选取不同的索引
    if (m_tracks.size() <= 1) {
        return 0;
    }

    // 反复随机直到选到不同于当前的索引
    // 对于小列表（典型播放列表），这比 Fisher-Yates 洗牌更简单高效
    int candidate = m_currentIndex;
    while (candidate == m_currentIndex) {
        candidate = QRandomGenerator::global()->bounded(m_tracks.size());
    }

    return candidate;
}
