/// @file    fragment_handler.h
/// @brief   UDP 大数据分片与重组处理器。
///
/// @details 对应教程：进阶层 04-QtNetwork/02-UDP 高级用法。
///          UDP 数据报有大小限制（通常 MTU ~1500 字节），发送超过 MTU 的数据需要
///          自行分片。本类实现基于序号的分片/重组协议，每个分片携带 3 字节头部：
///          [totalFragments 1B][fragmentIndex 1B][messageId 1B]，随后是有效负载。

#pragma once

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QVector>

/// @brief UDP 数据报分片与重组处理器。
///
/// 发送端调用 fragment() 将大数据拆成适合 MTU 的小片；
/// 接收端调用 processFragment() 逐片提交，全部到齐后发射 messageAssembled 信号。
class FragmentHandler : public QObject
{
    Q_OBJECT

public:
    /// @brief 分片协议头部固定长度（3 字节）。
    static constexpr int kHeaderSize = 3;

    /// @brief 头部各字段在字节流中的偏移量。
    enum HeaderOffset : int
    {
        kOffsetTotalFragments = 0,   ///< 总分片数（1 字节，最多 255 片）
        kOffsetFragmentIndex = 1,    ///< 当前分片序号（1 字节，从 0 起）
        kOffsetMessageId     = 2     ///< 消息标识（1 字节，用于区分不同消息）
    };

    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit FragmentHandler(QObject* parent = nullptr);

    /// @brief 将一块完整数据分片，每片附带协议头部。
    /// @param[in] data           原始数据。
    /// @param[in] maxPayloadSize 每片有效负载的最大字节数（不含头部），默认 800。
    /// @return 分片列表，每片 = [header 3B] + [payload]；空列表表示输入为空。
    /// @note  返回的分片可直接通过 UDP 发送，接收端按序号重组即可。
    QVector<QByteArray> fragment(const QByteArray& data, int maxPayloadSize = 800);

    /// @brief 处理收到的单个分片，当同一 messageId 的全部分片到齐后发射信号。
    /// @param[in] frag 包含 3 字节头部的分片数据。
    /// @note  片可能乱序到达，内部用 QMap 缓存直到齐片。messageId 溢出后回绕（0~255）。
    void processFragment(const QByteArray& frag);

    /// @brief 获取当前正在重组的消息数量（用于调试/监控）。
    /// @return 未完成重组的消息条数。
    int pendingMessageCount() const;

signals:
    /// @brief 全部分片到齐、重组完成时发射。
    /// @param completeData 重组后的完整原始数据（不含任何分片头部）。
    void messageAssembled(const QByteArray& completeData);

private:
    /// @brief 单条消息的重组状态。
    struct ReassemblyState
    {
        quint8 totalFragments = 0;           ///< 期望收到的总片数
        quint8 receivedCount  = 0;           ///< 已收到的片数
        QMap<int, QByteArray> fragments;     ///< 按 fragmentIndex 存储的负载
    };

    quint8 m_nextMessageId;   ///< 下一条待发送消息的 ID（回绕 0~255）
    QMap<quint8, ReassemblyState> m_reassembly;   ///< 按 messageId 索引的重组状态表
};
