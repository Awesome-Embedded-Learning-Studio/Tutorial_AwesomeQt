/// @file    serializable.cpp
/// @brief   实现 BookRecord 的 QDataStream 序列化运算符及版本兼容演示函数。
///
/// 对应教程：进阶层 01-QtBase/08-文件与 IO 进阶。

#include "serializable.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>

// ---------------------------------------------------------------------------
// operator<<
// ---------------------------------------------------------------------------

QDataStream& operator<<(QDataStream& out, const BookRecord& book)
{
    // 先写入版本号，用于读取时的兼容性检查
    out << BookRecord::CURRENT_VERSION;
    out << book.title;
    out << book.author;
    out << book.year;
    out << book.rating;
    out << book.publishDate;
    return out;
}

// ---------------------------------------------------------------------------
// operator>>
// ---------------------------------------------------------------------------

QDataStream& operator>>(QDataStream& in, BookRecord& book)
{
    int version = 1;  // 默认版本 1（旧格式没有版本号字段）
    in >> version;

    in >> book.title;
    in >> book.author;
    in >> book.year;

    if (version >= 2) {
        // 版本 2 新增了 rating 和 publishDate 字段
        in >> book.rating;
        in >> book.publishDate;
    } else {
        // 版本 1 兼容：设置默认值
        book.rating = 0.0;
        book.publishDate = QDate();
    }

    return in;
}

// ---------------------------------------------------------------------------
// demoDataStreamVersioning
// ---------------------------------------------------------------------------

void demoDataStreamVersioning(const QString& filePath)
{
    qDebug() << "\n=== QDataStream 版本兼容序列化 ===";

    // --- 写入数据 ---
    qDebug() << "\n--- 写入数据 ---";
    BookRecord book1{"Qt6 编程指南", "张三", 2024, 4.8, QDate(2024, 3, 15)};
    BookRecord book2{"C++ Primer", "Stanley Lippman", 2012, 4.5, QDate(2012, 8, 6)};

    QFile writeFile(filePath);
    if (writeFile.open(QIODevice::WriteOnly)) {
        QDataStream out(&writeFile);
        // 设置数据流版本，确保跨 Qt 版本兼容
        out.setVersion(QDataStream::Qt_6_0);

        out << book1;
        out << book2;

        writeFile.close();
        qDebug() << "写入成功:" << book1.title << "和" << book2.title;
    }

    // --- 读取数据 ---
    qDebug() << "\n--- 读取数据 ---";
    QFile readFile(filePath);
    if (readFile.open(QIODevice::ReadOnly)) {
        QDataStream in(&readFile);
        in.setVersion(QDataStream::Qt_6_0);

        BookRecord read1, read2;
        in >> read1 >> read2;

        readFile.close();
        qDebug() << "读取到:" << read1.title << "评分:" << read1.rating
                 << "出版日期:" << read1.publishDate.toString(Qt::ISODate);
        qDebug() << "读取到:" << read2.title << "评分:" << read2.rating
                 << "出版日期:" << read2.publishDate.toString(Qt::ISODate);
    }

    qDebug() << "\n关键要点:";
    qDebug() << "  - setVersion() 确保不同 Qt 版本间的数据格式兼容";
    qDebug() << "  - 写入版本号字段可以在读取时做向前兼容";
    qDebug() << "  - 旧版本程序读取新格式数据时使用默认值";
}
