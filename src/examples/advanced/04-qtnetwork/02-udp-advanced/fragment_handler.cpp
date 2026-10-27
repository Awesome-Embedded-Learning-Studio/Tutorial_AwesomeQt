/// @file    fragment_handler.cpp
/// @brief   UDP 数据报分片与重组处理器的实现。
///
/// @details 对应教程：进阶层 04-QtNetwork/02-UDP 高级用法。
///          实现基于 3 字节头部的分片协议：[totalFragments][fragmentIndex][messageId]。

#include "fragment_handler.h"

#include <QDebug>

FragmentHandler::FragmentHandler(QObject* parent)
    : QObject(parent)
    , m_nextMessageId(0)
{
}

QVector<QByteArray> FragmentHandler::fragment(const QByteArray& data, int maxPayloadSize)
{
    QVector<QByteArray> result;

    if (data.isEmpty()) {
        return result;
    }

    // 计算需要的总片数，向上取整
    const int totalFragments = (data.size() + maxPayloadSize - 1) / maxPayloadSize;

    // 协议头部只有 1 字节表示总片数，上限 255
    if (totalFragments > 255) {
        qWarning() << "FragmentHandler: data too large, would need" << totalFragments
                    << "fragments (max 255)";
        return result;
    }

    const quint8 msgId = m_nextMessageId;
    ++m_nextMessageId;    // 自动回绕：quint8 溢出后归零

    for (int i = 0; i < totalFragments; ++i) {
        const int offset = i * maxPayloadSize;
        const auto length = std::min<qsizetype>(maxPayloadSize, data.size() - offset);

        QByteArray header(kHeaderSize, Qt::Initialization::Uninitialized);
        header[static_cast<int>(kOffsetTotalFragments)] =
            static_cast<char>(totalFragments);
        header[static_cast<int>(kOffsetFragmentIndex)] =
            static_cast<char>(i);
        header[static_cast<int>(kOffsetMessageId)] =
            static_cast<char>(msgId);

        QByteArray fragment;
        // 预分配减少重复扩容
        fragment.reserve(kHeaderSize + length);
        fragment.append(header);
        fragment.append(data.constData() + offset, length);

        result.append(std::move(fragment));
    }

    qDebug() << "FragmentHandler: split into" << totalFragments
             << "fragments, messageId =" << static_cast<int>(msgId)
             << ", payload per fragment <=" << maxPayloadSize;

    return result;
}

void FragmentHandler::processFragment(const QByteArray& frag)
{
    // 分片必须至少包含 3 字节头部
    if (frag.size() < kHeaderSize) {
        qWarning() << "FragmentHandler: fragment too short (" << frag.size()
                    << "bytes), minimum is" << kHeaderSize;
        return;
    }

    const quint8 totalFragments = static_cast<quint8>(
        frag[static_cast<int>(kOffsetTotalFragments)]);
    const quint8 fragmentIndex  = static_cast<quint8>(
        frag[static_cast<int>(kOffsetFragmentIndex)]);
    const quint8 messageId      = static_cast<quint8>(
        frag[static_cast<int>(kOffsetMessageId)]);

    // 防御性检查：序号不能超过总片数
    if (fragmentIndex >= totalFragments) {
        qWarning() << "FragmentHandler: invalid fragment index" << fragmentIndex
                    << ">=" << totalFragments;
        return;
    }

    auto& state = m_reassembly[messageId];
    state.totalFragments = totalFragments;

    // 片可能重复到达（UDP 不保证去重），仅接受首次
    if (state.fragments.contains(fragmentIndex)) {
        qDebug() << "FragmentHandler: duplicate fragment" << fragmentIndex
                 << "for message" << static_cast<int>(messageId) << ", ignoring";
        return;
    }

    // 存储负载部分（跳过头部）
    state.fragments.insert(fragmentIndex, frag.mid(kHeaderSize));
    state.receivedCount++;

    // 判断是否齐片
    if (state.receivedCount >= totalFragments) {
        QByteArray complete;
        // 预估总大小，减少扩容
        complete.reserve(state.fragments.size() * 800);

        for (int i = 0; i < totalFragments; ++i) {
            complete.append(state.fragments.value(i));
        }

        qDebug() << "FragmentHandler: reassembled message"
                 << static_cast<int>(messageId)
                 << "total" << complete.size() << "bytes";

        emit messageAssembled(complete);

        // 清理重组状态，释放内存
        m_reassembly.remove(messageId);
    }
}

int FragmentHandler::pendingMessageCount() const
{
    return m_reassembly.size();
}
