/// @file    main.cpp
/// @brief   程序入口，依次调用四个文件 I/O 进阶演示函数。
///
/// 对应教程：进阶层 01-QtBase/08-文件与 IO 进阶。

#include <QCoreApplication>
#include <QDebug>
#include <QTemporaryDir>

#include "file_ops_demo.h"
#include "serializable.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========== Qt 文件 I/O 进阶示例 ==========\n";

    // 使用临时目录存放测试文件，程序退出后自动清理
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        qWarning() << "无法创建临时目录";
        return 1;
    }
    tempDir.setAutoRemove(true);
    qDebug() << "临时目录:" << tempDir.path();

    // ----------------------------------------------------------------------
    // 演示 1: QDataStream 版本兼容序列化
    // ----------------------------------------------------------------------
    qDebug() << "\n[演示 1] QDataStream 版本兼容序列化";
    demoDataStreamVersioning(tempDir.path() + "/books.dat");

    // ----------------------------------------------------------------------
    // 演示 2: QSaveFile 原子写入
    // ----------------------------------------------------------------------
    qDebug() << "\n[演示 2] QSaveFile 原子写入";
    demoAtomicWrite(tempDir.path());

    // ----------------------------------------------------------------------
    // 演示 3: QFile::map() 内存映射
    // ----------------------------------------------------------------------
    qDebug() << "\n[演示 3] QFile::map() 内存映射";
    demoMemoryMappedFile(tempDir.path());

    // ----------------------------------------------------------------------
    // 演示 4: QFileSystemWatcher 文件监控
    // ----------------------------------------------------------------------
    qDebug() << "\n[演示 4] QFileSystemWatcher 文件监控";
    demoFileSystemWatcher(tempDir.path());

    // ----------------------------------------------------------------------
    // 结束
    // ----------------------------------------------------------------------
    qDebug() << "\n===========================================";
    qDebug() << "要点总结:";
    qDebug() << "  - QDataStream::setVersion() 确保跨 Qt 版本兼容";
    qDebug() << "  - 自定义 << / >> 运算符实现复合类型序列化";
    qDebug() << "  - QSaveFile 原子写入保证数据完整性";
    qDebug() << "  - QFile::map() 内存映射适合大文件随机访问";
    qDebug() << "  - QFileSystemWatcher 监控文件和目录变化";

    return 0;
}
