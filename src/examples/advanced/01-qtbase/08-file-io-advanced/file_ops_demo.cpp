/// @file    file_ops_demo.cpp
/// @brief   实现 QSaveFile 原子写入、QFile::map() 内存映射、QFileSystemWatcher 文件监控。
///
/// 对应教程：进阶层 01-QtBase/08-文件与 IO 进阶。

#include "file_ops_demo.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QFile>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QSaveFile>

// ---------------------------------------------------------------------------
// demoAtomicWrite
// ---------------------------------------------------------------------------

void demoAtomicWrite(const QString& dirPath)
{
    qDebug() << "\n=== QSaveFile 原子写入 ===";

    QString filePath = dirPath + "/atomic_test.txt";

    // --- 成功写入 ---
    {
        QSaveFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("这是原子写入的内容\n");
            file.write("commit() 之前数据在临时文件中\n");

            // commit() 将临时文件重命名为目标文件（原子操作）
            if (file.commit()) {
                qDebug() << "原子写入成功:" << filePath;
            }
        }
    }

    // 验证写入内容
    QFile verifyRead(filePath);
    if (verifyRead.open(QIODevice::ReadOnly)) {
        qDebug() << "验证内容:" << verifyRead.readAll();
        verifyRead.close();
    }

    // --- 取消写入（不调用 commit，临时文件自动丢弃）---
    {
        QSaveFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("这些内容不应该出现\n");
            // 不调用 commit()，QSaveFile 析构时自动丢弃临时文件
            qDebug() << "cancel 写入（不调用 commit）";
        }
    }

    // 验证原文件未被修改
    QFile verifyRead2(filePath);
    if (verifyRead2.open(QIODevice::ReadOnly)) {
        qDebug() << "取消后原文件内容不变:" << verifyRead2.readAll();
        verifyRead2.close();
    }

    qDebug() << "\n关键要点:";
    qDebug() << "  - QSaveFile 写入临时文件，commit() 原子重命名";
    qDebug() << "  - 未调用 commit() 时自动丢弃，不损坏原文件";
    qDebug() << "  - 适合需要保证数据完整性的场景（配置文件、数据库等）";
}

// ---------------------------------------------------------------------------
// demoMemoryMappedFile
// ---------------------------------------------------------------------------

void demoMemoryMappedFile(const QString& dirPath)
{
    qDebug() << "\n=== QFile::map() 内存映射 ===";

    QString filePath = dirPath + "/mapped_test.bin";

    // 创建测试文件
    {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            QByteArray data;
            for (int i = 0; i < 256; ++i) {
                data.append(static_cast<char>(i));
            }
            file.write(data);
            file.close();
            qDebug() << "创建测试文件:" << data.size() << "字节";
        }
    }

    // 使用 map() 内存映射读取
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        // map() 将文件内容映射到内存，返回指针
        // 优点: 不需要 read() 系统调用，直接访问内存
        uchar* mapped = file.map(0, file.size());
        if (mapped) {
            qDebug() << "内存映射成功，大小:" << file.size() << "字节";
            qDebug() << "前 16 字节:"
                     << QByteArray(reinterpret_cast<char*>(mapped), 16).toHex(' ');
            qDebug() << "最后 16 字节:"
                     << QByteArray(
                            reinterpret_cast<char*>(mapped + file.size() - 16), 16)
                            .toHex(' ');

            // 修改映射内容需要 ReadOnly 改为 ReadWrite，此处只做读取演示

            // unmap() 释放映射
            file.unmap(mapped);
            qDebug() << "unmap() 释放映射成功";
        } else {
            qDebug() << "内存映射失败:" << file.errorString();
        }
        file.close();
    }

    qDebug() << "\n关键要点:";
    qDebug() << "  - map() 将文件映射到进程地址空间，避免 read/write 系统调用";
    qDebug() << "  - 适合随机访问大文件的场景";
    qDebug() << "  - 使用完后必须 unmap() 释放";
}

// ---------------------------------------------------------------------------
// demoFileSystemWatcher
// ---------------------------------------------------------------------------

void demoFileSystemWatcher(const QString& dirPath)
{
    qDebug() << "\n=== QFileSystemWatcher 文件监控 ===";

    QFileSystemWatcher watcher;

    // 监控目录
    watcher.addPath(dirPath);
    qDebug() << "监控目录:" << dirPath;

    // 监控特定文件
    QString filePath = dirPath + "/watched_file.txt";
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write("初始内容\n");
        file.close();
    }
    watcher.addPath(filePath);
    qDebug() << "监控文件:" << filePath;

    // 连接信号
    int dirChangeCount = 0;
    int fileChangeCount = 0;

    QObject::connect(
        &watcher, &QFileSystemWatcher::directoryChanged,
        [&](const QString& path) {
            dirChangeCount++;
            qDebug() << "  [目录变化] " << path << "(第" << dirChangeCount << "次)";
        });

    QObject::connect(
        &watcher, &QFileSystemWatcher::fileChanged,
        [&](const QString& path) {
            fileChangeCount++;
            qDebug() << "  [文件变化] " << path << "(第" << fileChangeCount << "次)";
            // 文件被修改后可能需要重新添加监控（某些系统会移除监控）
            if (QFile::exists(path)) {
                watcher.addPath(path);
            }
        });

    // 触发一些文件变化
    qDebug() << "\n--- 修改监控的文件 ---";
    {
        QFile writeFile(filePath);
        if (writeFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            writeFile.write("追加内容\n");
            writeFile.close();
        }
    }

    // 处理事件以触发 watcher 信号
    QCoreApplication::processEvents();

    qDebug() << "\n关键要点:";
    qDebug() << "  - QFileSystemWatcher 监控文件和目录的变化";
    qDebug() << "  - fileChanged 信号在文件内容或属性变化时触发";
    qDebug() << "  - directoryChanged 信号在目录内容变化时触发";
    qDebug() << "  - 某些平台文件被替换后需要重新添加监控";
}
