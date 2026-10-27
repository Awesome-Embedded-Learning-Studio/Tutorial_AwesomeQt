/// @file    metadata_reader.h
/// @brief   Audio file metadata reader using QMediaPlayer.
///
/// 对应教程：进阶层 05-其他模块/04-QtMultimedia 高级。
/// 演示 QMediaMetaData 的提取与使用，同时提供无需真实媒体文件的演示数据生成。

#pragma once

#include <QMap>
#include <QObject>
#include <QUrl>
#include <QVariant>

class QMediaPlayer;

/// @brief 从音频文件中提取元数据信息。
///
/// 内部使用 QMediaPlayer 加载媒体源并读取 QMediaMetaData。
/// 由于实际教学环境中可能没有真实音频文件，
/// 还提供了 generateDemoMetadata() 静态方法生成模拟数据。
class MetadataReader : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化 QMediaPlayer。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit MetadataReader(QObject* parent = nullptr);

    /// @brief 析构函数，确保资源释放。
    ~MetadataReader() override;

    /// @brief 分析指定 URL 的媒体文件元数据。
    /// @param[in] url 媒体文件的 URL 路径。
    /// @note 设置 source 后等待 QMediaPlayer 的 metadataChanged 信号，
    ///       异步获取元数据；如果文件不存在会触发 error 信号。
    void analyzeFile(const QUrl& url);

    /// @brief 生成一组演示用的元数据，无需真实媒体文件。
    /// @param[in] trackIndex 曲目索引，用于生成不同的演示数据。
    /// @return 包含模拟元数据的 QMap。
    /// @note 教学用：当没有实际音频文件时，用此方法展示元数据结构。
    static QMap<QString, QVariant> generateDemoMetadata(int trackIndex);

signals:
    /// @brief 元数据提取完成时发射。
    /// @param[out] data 包含所有提取到的元数据键值对。
    void metadataReady(const QMap<QString, QVariant>& data);

    /// @brief 发生错误时发射。
    /// @param[out] msg 人类可读的错误描述。
    void error(const QString& msg);

private slots:
    /// @brief 媒体状态变化时检查元数据可用性。
    /// @note QMediaPlayer 加载媒体后异步准备元数据，
    ///       需要在状态变化时检查是否已就绪。
    void onMediaStatusChanged(int status);

private:
    QMediaPlayer* m_player;  ///< 内部播放器实例，用于加载和解析媒体
};
