/// @file    main.cpp
/// @brief   QtMultimedia 高级示例主程序 —— 演示元数据提取与播放列表管理。
///
/// 对应教程：进阶层 05-其他模块/04-QtMultimedia 高级。
/// 本程序为控制台应用，使用 QCoreApplication 运行，
/// 无需 GUI 窗口和真实音频文件即可演示核心功能。

#include "metadata_reader.h"
#include "playlist_manager.h"

#include <QCoreApplication>
#include <QTimer>
#include <QUrl>

#include <QDebug>
#include <QString>

namespace {

/// @brief 将 PlayMode 枚举转换为可读字符串。
/// @param[in] mode 播放模式枚举值。
/// @return 人类可读的模式名称。
QString playModeToString(PlayMode mode)
{
    switch (mode) {
        case PlayMode::kSequential:
            return QStringLiteral("Sequential");
        case PlayMode::kLoop:
            return QStringLiteral("Loop");
        case PlayMode::kShuffle:
            return QStringLiteral("Shuffle");
    }
    return QStringLiteral("Unknown");
}

/// @brief 将毫秒时长格式化为 "mm:ss" 字符串。
/// @param[in] ms 毫秒数。
/// @return 格式化后的时间字符串。
QString formatDuration(qint64 ms)
{
    int totalSeconds = static_cast<int>(ms / 1000);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes)
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

/// @brief 打印一条分隔线。
/// @param[in] title 分隔线中的标题文本。
void printSection(const QString& title)
{
    qDebug().noquote() << QStringLiteral("\n======== %1 ========").arg(title);
}

/// @brief 打印元数据映射中的所有字段。
/// @param[in] data 元数据键值对。
void printMetadata(const QMap<QString, QVariant>& data)
{
    // 按固定顺序输出关键字段
    const QStringList kFields = {
        QStringLiteral("Title"),
        QStringLiteral("Author"),
        QStringLiteral("Album"),
        QStringLiteral("Duration"),
        QStringLiteral("TrackNumber"),
        QStringLiteral("AudioBitRate"),
        QStringLiteral("AudioCodec"),
    };

    for (const QString& field : kFields) {
        QVariant value = data.value(field);
        if (value.isValid()) {
            // Duration 和 BitRate 需要格式化显示
            QString displayValue;
            if (field == QStringLiteral("Duration")) {
                displayValue = formatDuration(value.toLongLong());
            }
            else if (field == QStringLiteral("AudioBitRate")) {
                displayValue =
                    QString::number(value.toInt() / 1000) +
                    QStringLiteral(" kbps");
            }
            else {
                displayValue = value.toString();
            }
            qDebug().noquote()
                << QStringLiteral("  %1: %2").arg(field, -14).arg(displayValue);
        }
    }
}

/// @brief 演示 1：使用 generateDemoMetadata 展示模拟元数据。
void demoMetadataExtraction()
{
    printSection(QStringLiteral("Demo 1: Metadata Extraction (Demo Data)"));

    for (int i = 0; i < 5; ++i) {
        qDebug().noquote()
            << QStringLiteral("\n--- Track %1 ---").arg(i + 1);
        QMap<QString, QVariant> data =
            MetadataReader::generateDemoMetadata(i);
        printMetadata(data);
    }
}

/// @brief 演示 2：播放列表管理 —— 添加曲目与模式切换。
void demoPlaylistManagement()
{
    printSection(QStringLiteral("Demo 2: Playlist Management"));

    PlaylistManager playlist;

    // 使用 demo URL 模拟 5 首曲目
    const QStringList kFileNames = {
        QStringLiteral("clair_de_lune.mp3"),
        QStringLiteral("four_seasons_spring.flac"),
        QStringLiteral("moonlight_sonata.mp3"),
        QStringLiteral("nocturne_op9_no2.flac"),
        QStringLiteral("swan_lake.mp3"),
    };

    qDebug().noquote() << QStringLiteral("\nAdding tracks to playlist...");
    for (int i = 0; i < kFileNames.size(); ++i) {
        QUrl url = QUrl::fromLocalFile(kFileNames.at(i));
        int idx = playlist.addTrack(url);
        qDebug().noquote()
            << QStringLiteral("  [%1] Added: %2")
                   .arg(idx)
                   .arg(kFileNames.at(i));
    }
    qDebug().noquote()
        << QStringLiteral("Total tracks: %1").arg(playlist.trackCount());

    // 测试顺序模式
    qDebug().noquote()
        << QStringLiteral("\n--- Sequential Mode ---");
    playlist.setPlayMode(PlayMode::kSequential);
    qDebug().noquote()
        << QStringLiteral("Mode: %1").arg(playModeToString(playlist.playMode()));

    // 一直按 next 直到到达末尾
    for (int i = 0; i < playlist.trackCount() + 1; ++i) {
        qDebug().noquote()
            << QStringLiteral("  Current: [%1] %2")
                   .arg(playlist.currentIndex())
                   .arg(playlist.currentUrl().fileName());
        playlist.next();
    }
    qDebug().noquote()
        << QStringLiteral("  (Sequential: stopped at last track [%1])")
               .arg(playlist.currentIndex());

    // 测试循环模式
    qDebug().noquote()
        << QStringLiteral("\n--- Loop Mode ---");
    playlist.setPlayMode(PlayMode::kLoop);
    playlist.setCurrentIndex(0);
    qDebug().noquote()
        << QStringLiteral("Mode: %1").arg(playModeToString(playlist.playMode()));

    // 循环模式下 next 会从末尾回到开头
    for (int i = 0; i < 7; ++i) {
        playlist.next();
        qDebug().noquote()
            << QStringLiteral("  Next -> [%1] %2")
                   .arg(playlist.currentIndex())
                   .arg(playlist.currentUrl().fileName());
    }

    // 测试随机模式
    qDebug().noquote()
        << QStringLiteral("\n--- Shuffle Mode ---");
    playlist.setPlayMode(PlayMode::kShuffle);
    qDebug().noquote()
        << QStringLiteral("Mode: %1").arg(playModeToString(playlist.playMode()));

    for (int i = 0; i < 5; ++i) {
        playlist.next();
        qDebug().noquote()
            << QStringLiteral("  Shuffle -> [%1] %2")
                   .arg(playlist.currentIndex())
                   .arg(playlist.currentUrl().fileName());
    }
}

/// @brief 演示 3：播放列表导航 —— previous / setCurrentIndex。
void demoPlaylistNavigation()
{
    printSection(QStringLiteral("Demo 3: Playlist Navigation"));

    PlaylistManager playlist;

    // 添加曲目
    const QStringList kFileNames = {
        QStringLiteral("track_a.mp3"),
        QStringLiteral("track_b.mp3"),
        QStringLiteral("track_c.mp3"),
        QStringLiteral("track_d.mp3"),
    };

    for (const QString& name : kFileNames) {
        playlist.addTrack(QUrl::fromLocalFile(name));
    }

    // 使用 setCurrentIndex 直接跳转
    qDebug().noquote() << QStringLiteral("\nDirect jump:");
    playlist.setCurrentIndex(2);
    qDebug().noquote()
        << QStringLiteral("  Jump to [2]: %1")
               .arg(playlist.currentUrl().fileName());

    playlist.setCurrentIndex(0);
    qDebug().noquote()
        << QStringLiteral("  Jump to [0]: %1")
               .arg(playlist.currentUrl().fileName());

    // 测试 previous 在循环模式下的行为
    qDebug().noquote() << QStringLiteral("\nPrevious (Loop mode):");
    playlist.setPlayMode(PlayMode::kLoop);
    playlist.setCurrentIndex(0);

    // 在第一首按 previous 应回到最后一首
    playlist.previous();
    qDebug().noquote()
        << QStringLiteral("  Prev from [0] -> [%1]: %2")
               .arg(playlist.currentIndex())
               .arg(playlist.currentUrl().fileName());

    // 继续往前
    playlist.previous();
    qDebug().noquote()
        << QStringLiteral("  Prev from [%1] -> [%2]: %3")
               .arg(playlist.currentIndex() + 1)
               .arg(playlist.currentIndex())
               .arg(playlist.currentUrl().fileName());

    // 测试边界：无效索引
    qDebug().noquote() << QStringLiteral("\nBoundary check:");
    int beforeInvalid = playlist.currentIndex();
    playlist.setCurrentIndex(999);
    qDebug().noquote()
        << QStringLiteral("  setCurrentIndex(999): index still [%1] (unchanged)")
               .arg(playlist.currentIndex());
    Q_ASSERT(playlist.currentIndex() == beforeInvalid);
}

}  // anonymous namespace

/// @brief 程序入口，依次执行三个演示并自动退出。
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 执行所有演示
    demoMetadataExtraction();
    demoPlaylistManagement();
    demoPlaylistNavigation();

    // 打印总结并延迟退出，确保事件循环处理完所有待发信号
    printSection(QStringLiteral("All demos completed"));
    qDebug().noquote()
        << QStringLiteral("This example used demo data only.");
    qDebug().noquote()
        << QStringLiteral("With real audio files, MetadataReader::analyzeFile()");
    qDebug().noquote()
        << QStringLiteral("can extract actual metadata via QMediaPlayer.");

    // 1 秒后自动退出，保证控制台输出完整显示
    QTimer::singleShot(1000, &app, &QCoreApplication::quit);

    return app.exec();
}
