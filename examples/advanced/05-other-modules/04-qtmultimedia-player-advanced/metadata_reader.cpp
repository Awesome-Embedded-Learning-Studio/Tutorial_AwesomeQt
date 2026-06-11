/// @file    metadata_reader.cpp
/// @brief   MetadataReader 类实现 —— 音频元数据提取与演示数据生成。
///
/// 对应教程：进阶层 05-其他模块/04-QtMultimedia 高级。

#include "metadata_reader.h"

#include <QMediaFormat>
#include <QMediaPlayer>
#include <QMediaMetaData>

MetadataReader::MetadataReader(QObject* parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
{
    // 播放器状态变化时检查元数据就绪情况
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this,
            &MetadataReader::onMediaStatusChanged);
}

MetadataReader::~MetadataReader() = default;

void MetadataReader::analyzeFile(const QUrl& url)
{
    if (!url.isValid()) {
        emit error(QStringLiteral("Invalid URL: ") + url.toString());
        return;
    }

    // 设置 source 触发异步加载，元数据将在 mediaStatusChanged 中读取
    m_player->setSource(url);
}

void MetadataReader::onMediaStatusChanged(int status)
{
    // LoadedMedia 表示媒体已加载完成，元数据此时可用
    if (status == QMediaPlayer::LoadedMedia) {
        QMap<QString, QVariant> data;

        // 通过 QMediaPlayer::metaData() 获取 QMediaMetaData 对象
        const QMediaMetaData metaData = m_player->metaData();

        // 提取常见音频元数据字段
        data[QStringLiteral("Title")] =
            metaData.value(QMediaMetaData::Title);
        data[QStringLiteral("Author")] =
            metaData.value(QMediaMetaData::Author);
        data[QStringLiteral("Album")] =
            metaData.value(QMediaMetaData::AlbumTitle);
        data[QStringLiteral("Duration")] =
            metaData.value(QMediaMetaData::Duration);
        data[QStringLiteral("TrackNumber")] =
            metaData.value(QMediaMetaData::TrackNumber);
        data[QStringLiteral("AudioBitRate")] =
            metaData.value(QMediaMetaData::AudioBitRate);
        data[QStringLiteral("AudioCodec")] =
            metaData.value(QMediaMetaData::FileFormat);

        emit metadataReady(data);
    }
    else if (status == QMediaPlayer::InvalidMedia) {
        emit error(QStringLiteral("Failed to load media: invalid or corrupt file"));
    }
}

QMap<QString, QVariant> MetadataReader::generateDemoMetadata(int trackIndex)
{
    // 模拟 5 首不同曲目的元数据，覆盖常见字段
    // 教学用：展示 QMediaMetaData 包含哪些典型信息

    static const QMap<int, QMap<QString, QVariant>> kDemoTracks = {
        {0,
         {{QStringLiteral("Title"), QStringLiteral("Clair de Lune")},
          {QStringLiteral("Author"), QStringLiteral("Claude Debussy")},
          {QStringLiteral("Album"), QStringLiteral("Suite bergamasque")},
          {QStringLiteral("Duration"), 312000},
          {QStringLiteral("TrackNumber"), 1},
          {QStringLiteral("AudioBitRate"), 320000},
          {QStringLiteral("AudioCodec"), QStringLiteral("audio/mpeg")}}},
        {1,
         {{QStringLiteral("Title"), QStringLiteral("Four Seasons: Spring")},
          {QStringLiteral("Author"), QStringLiteral("Antonio Vivaldi")},
          {QStringLiteral("Album"), QStringLiteral("The Four Seasons")},
          {QStringLiteral("Duration"), 198000},
          {QStringLiteral("TrackNumber"), 1},
          {QStringLiteral("AudioBitRate"), 256000},
          {QStringLiteral("AudioCodec"), QStringLiteral("audio/flac")}}},
        {2,
         {{QStringLiteral("Title"), QStringLiteral("Moonlight Sonata")},
          {QStringLiteral("Author"), QStringLiteral("Ludwig van Beethoven")},
          {QStringLiteral("Album"), QStringLiteral("Piano Sonata No. 14")},
          {QStringLiteral("Duration"), 423000},
          {QStringLiteral("TrackNumber"), 1},
          {QStringLiteral("AudioBitRate"), 320000},
          {QStringLiteral("AudioCodec"), QStringLiteral("audio/mpeg")}}},
        {3,
         {{QStringLiteral("Title"), QStringLiteral("Nocturne Op.9 No.2")},
          {QStringLiteral("Author"), QStringLiteral("Frederic Chopin")},
          {QStringLiteral("Album"), QStringLiteral("Nocturnes")},
          {QStringLiteral("Duration"), 276000},
          {QStringLiteral("TrackNumber"), 2},
          {QStringLiteral("AudioBitRate"), 256000},
          {QStringLiteral("AudioCodec"), QStringLiteral("audio/flac")}}},
        {4,
         {{QStringLiteral("Title"), QStringLiteral("Swan Lake")},
          {QStringLiteral("Author"), QStringLiteral("Pyotr Tchaikovsky")},
          {QStringLiteral("Album"), QStringLiteral("Swan Lake Suite")},
          {QStringLiteral("Duration"), 345000},
          {QStringLiteral("TrackNumber"), 1},
          {QStringLiteral("AudioBitRate"), 320000},
          {QStringLiteral("AudioCodec"), QStringLiteral("audio/mpeg")}}}
    };

    // 返回对应索引的曲目数据；越界时返回空 map
    auto it = kDemoTracks.find(trackIndex);
    if (it != kDemoTracks.end()) {
        return it.value();
    }
    return {};
}
