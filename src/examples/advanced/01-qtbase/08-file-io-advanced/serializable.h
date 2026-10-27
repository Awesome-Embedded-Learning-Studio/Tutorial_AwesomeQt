/// @file    serializable.h
/// @brief   定义 BookRecord 结构体及 QDataStream 序列化运算符，演示版本兼容机制。
///
/// 对应教程：进阶层 01-QtBase/08-文件与 IO 进阶。

#pragma once

#include <QDate>
#include <QString>

// ---------------------------------------------------------------------------
// 可序列化的复合结构体
// ---------------------------------------------------------------------------

/// @brief 表示一本书的记录，支持通过 QDataStream 进行版本兼容的序列化。
struct BookRecord {
    QString title;         ///< 书名
    QString author;        ///< 作者
    int year;              ///< 出版年份
    double rating;         ///< 评分（版本 2 新增）
    QDate publishDate;     ///< 出版日期（版本 2 新增）

    /// @brief 当前序列化格式版本号，用于读取时的向前兼容检查。
    static constexpr int CURRENT_VERSION = 2;
};

// ---------------------------------------------------------------------------
// QDataStream 自定义序列化运算符 << / >>
// ---------------------------------------------------------------------------

/// @brief 将 BookRecord 写入数据流，包含版本号字段。
/// @param[out] out 目标数据流。
/// @param[in]  book 待写入的图书记录。
/// @return 数据流引用，支持链式调用。
QDataStream& operator<<(QDataStream& out, const BookRecord& book);

/// @brief 从数据流读取 BookRecord，支持版本兼容回退。
/// @param[in]  in  源数据流。
/// @param[out] book 读取结果存放的目标对象。
/// @return 数据流引用，支持链式调用。
QDataStream& operator>>(QDataStream& in, BookRecord& book);

// ---------------------------------------------------------------------------
// 演示 QDataStream 版本兼容序列化
// ---------------------------------------------------------------------------

/// @brief 演示写入和读取 BookRecord，展示版本兼容序列化的完整流程。
/// @param[in] filePath 用于存放序列化数据的文件路径。
void demoDataStreamVersioning(const QString& filePath);
