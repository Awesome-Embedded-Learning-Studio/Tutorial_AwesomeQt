#include <QCoreApplication>      // QApplication 的控制台版本
#include <QFile>                 // 文件读写类
#include <QDir>                  // 目录操作类
#include <QFileInfo>             // 文件信息查询类
#include <QTextStream>           // 文本流操作类
#include <QDataStream>           // 二进制数据流类
#include <QDebug>                // 调试输出
#include <QDate>                 // 日期类
#include <QDateTime>             // 日期时间类

#include <iostream>              // 标准输入输出

/**
 * Qt 文件与 IO 示例程序
 *
 * 本示例展示 Qt 文件 IO 体系的核心用法：
 * 1. QFile 文本读写
 * 2. QTextStream 流式文本操作
 * 3. QDataStream 二进制序列化
 * 4. QDir 目录遍历与操作
 * 5. QFileInfo 文件元信息查询
 */

// ============================================================================
// 第一部分：QFile 基础文本读写
// ============================================================================
void demonstrateQFileBasic()
{
    qDebug() << "\n=== 1. QFile 基础文本读写 ===";

    // 写入文本文件
    QFile writeFile("example_text.txt");

    // 打开文件：只写模式 | 文本模式
    if (writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "文件打开成功，准备写入...";

        // 直接写入字节数组
        writeFile.write("Hello, Qt File IO!\n");
        writeFile.write("这是第二行文本。\n");
        writeFile.write("中文内容测试：Qt 的文本处理很强大。\n");

        writeFile.close();
        qDebug() << "文本文件写入完成";
    } else {
        qDebug() << "文件打开失败:" << writeFile.errorString();
    }

    // 读取文本文件
    QFile readFile("example_text.txt");

    if (readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "\n--- 读取文件内容 ---";

        // 方式一：一次性读取全部内容
        QByteArray allContent = readFile.readAll();
        qDebug() << "全部内容:" << allContent;

        readFile.close();

        // 方式二：重新打开，逐行读取
        readFile.open(QIODevice::ReadOnly | QIODevice::Text);
        qDebug() << "\n--- 逐行读取 ---";

        int lineNum = 0;
        while (!readFile.atEnd()) {
            QByteArray line = readFile.readLine();
            lineNum++;
            qDebug() << "行" << lineNum << ":" << line.trimmed();  // trimmed() 去掉换行符
        }

        readFile.close();
    }
}

// ============================================================================
// 第二部分：QTextStream 流式文本操作
// ============================================================================
void demonstrateQTextStream()
{
    qDebug() << "\n=== 2. QTextStream 流式文本操作 ===";

    // 使用 QTextStream 写入配置文件
    QFile configFile("config.ini");

    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&configFile);

        // 设置编码为 UTF-8（Qt 6 写法）
        out.setEncoding(QStringConverter::Utf8);

        qDebug() << "写入配置文件...";

        // 流式写入，类似 C++ iostream
        out << "[Server]\n";
        out << "host = 192.168.1.100\n";
        out << "port = " << 8080 << "\n";
        out << "timeout = " << 30.5 << "\n";
        out << "debug = " << true << "\n";
        out << "\n";

        out << "[Database]\n";
        out << "driver = sqlite\n";
        out << "path = /var/data/app.db\n";

        configFile.close();
        qDebug() << "配置文件写入完成";
    }

    // 使用 QTextStream 读取配置文件
    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);
        in.setEncoding(QStringConverter::Utf8);

        qDebug() << "\n--- 读取配置文件 ---";

        QString line;
        int lineNum = 0;
        while (!in.atEnd()) {
            line = in.readLine();
            lineNum++;
            if (!line.isEmpty()) {
                qDebug() << "行" << lineNum << ":" << line;
            }
        }

        configFile.close();
    }
}

// ============================================================================
// 第三部分：QDataStream 二进制序列化
// ============================================================================
void demonstrateQDataStream()
{
    qDebug() << "\n=== 3. QDataStream 二进制序列化 ===";

    // 写入二进制数据
    QFile binFile("data.bin");

    if (binFile.open(QIODevice::WriteOnly)) {
        QDataStream out(&binFile);

        qDebug() << "写入二进制数据...";

        // 写入各种基本类型
        out << 42;                           // int
        out << 3.1415926;                    // double
        out << QString("Hello, Binary!");    // QString
        out << QByteArray("ABC");            // QByteArray

        // 写入容器
        QList<int> numbers = {10, 20, 30, 40, 50};
        out << numbers;

        QStringList names = {"Alice", "Bob", "Charlie"};
        out << names;

        binFile.close();
        qDebug() << "二进制数据写入完成";
    }

    // 读取二进制数据
    if (binFile.open(QIODevice::ReadOnly)) {
        QDataStream in(&binFile);

        qDebug() << "\n--- 读取二进制数据 ---";

        // 按照写入顺序读取
        int intValue;
        double doubleValue;
        QString stringValue;
        QByteArray byteArrayValue;
        QList<int> numbers;
        QStringList names;

        in >> intValue;
        in >> doubleValue;
        in >> stringValue;
        in >> byteArrayValue;
        in >> numbers;
        in >> names;

        qDebug() << "int:" << intValue;
        qDebug() << "double:" << doubleValue;
        qDebug() << "QString:" << stringValue;
        qDebug() << "QByteArray:" << byteArrayValue;
        qDebug() << "QList<int>:" << numbers;
        qDebug() << "QStringList:" << names;

        binFile.close();
    }
}

// ============================================================================
// 第四部分：QDir 目录操作
// ============================================================================
void demonstrateQDir()
{
    qDebug() << "\n=== 4. QDir 目录操作 ===";

    // 创建目录结构
    QDir dir;

    QString testDir = "test_data";
    QString subDir = testDir + "/subdir1";
    QString deepDir = subDir + "/deep";

    // mkpath 会递归创建所有必要的父目录
    if (!dir.exists(testDir)) {
        qDebug() << "创建目录结构:" << deepDir;
        if (dir.mkpath(deepDir)) {
            qDebug() << "目录创建成功";
        }
    }

    // 在目录中创建一些测试文件
    QFile::remove(testDir + "/file1.txt");
    QFile::remove(testDir + "/file2.md");
    QFile::remove(subDir + "/file3.txt");

    QFile f1(testDir + "/file1.txt");
    if (f1.open(QIODevice::WriteOnly)) {
        f1.write("Content of file1");
        f1.close();
    }

    QFile f2(testDir + "/file2.md");
    if (f2.open(QIODevice::WriteOnly)) {
        f2.write("# Markdown file");
        f2.close();
    }

    // 遍历目录
    qDebug() << "\n--- 遍历目录" << testDir << "---";

    QDir currentDir(testDir);

    // 设置过滤器：只显示文件，不显示 . 和 ..
    currentDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    // 设置排序：按名称排序
    currentDir.setSorting(QDir::Name);

    QFileInfoList fileList = currentDir.entryInfoList();

    qDebug() << "目录内容（" << fileList.size() << "项）:";
    for (const QFileInfo &fileInfo : fileList) {
        QString type = fileInfo.isDir() ? "[DIR]" : "[FILE]";
        qDebug() << "  " << type << fileInfo.fileName();
    }

    // 使用通配符过滤文件
    qDebug() << "\n--- 只显示 .txt 文件 ---";
    QStringList filters;
    filters << "*.txt";
    currentDir.setNameFilters(filters);

    QStringList txtFiles = currentDir.entryList();
    for (const QString &fileName : txtFiles) {
        qDebug() << "  " << fileName;
    }
}

// ============================================================================
// 第五部分：QFileInfo 文件信息查询
// ============================================================================
void demonstrateQFileInfo()
{
    qDebug() << "\n=== 5. QFileInfo 文件信息查询 ===";

    // 先确保测试文件存在
    QFile testFile("test_info.txt");
    if (testFile.open(QIODevice::WriteOnly)) {
        testFile.write("This is a test file for QFileInfo demo.\n");
        testFile.write("Let's see what information we can get.\n");
        testFile.close();
    }

    QFileInfo fileInfo("test_info.txt");

    qDebug() << "--- 文件基本信息 ---";
    qDebug() << "文件名:" << fileInfo.fileName();
    qDebug() << "完整路径:" << fileInfo.absoluteFilePath();
    qDebug() << "目录路径:" << fileInfo.absolutePath();
    qDebug() << "文件大小:" << fileInfo.size() << "字节";
    qDebug() << "是否可读:" << fileInfo.isReadable();
    qDebug() << "是否可写:" << fileInfo.isWritable();
    qDebug() << "是否可执行:" << fileInfo.isExecutable();
    qDebug() << "是否隐藏:" << fileInfo.isHidden();

    qDebug() << "\n--- 文件后缀信息 ---";
    qDebug() << "后缀名:" << fileInfo.suffix();           // txt
    qDebug() << "基础名称:" << fileInfo.baseName();        // test_info
    qDebug() << "完整后缀:" << fileInfo.completeSuffix();  // txt

    qDebug() << "\n--- 时间信息 ---";
    qDebug() << "创建时间:" << fileInfo.birthTime().toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << "修改时间:" << fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << "访问时间:" << fileInfo.lastRead().toString("yyyy-MM-dd hh:mm:ss");

    // 演示 completeSuffix 的作用（对于类似 .tar.gz 的文件）
    QFileInfo tarInfo("archive.tar.gz");
    qDebug() << "\n--- 多重后缀示例 ---";
    qDebug() << "文件名:" << tarInfo.fileName();
    qDebug() << "suffix() 返回:" << tarInfo.suffix();             // gz
    qDebug() << "completeSuffix() 返回:" << tarInfo.completeSuffix();  // tar.gz
}

// ============================================================================
// 第六部分：简单日志管理器（综合示例）
// ============================================================================
class SimpleLogManager
{
public:
    explicit SimpleLogManager(const QString &logPath)
        : m_logPath(logPath)
    {
        // 确保日志目录存在
        QDir dir;
        if (!dir.exists(m_logPath)) {
            dir.mkpath(m_logPath);
            qDebug() << "创建日志目录:" << m_logPath;
        }
    }

    // 写入日志
    void writeLog(const QString &level, const QString &message)
    {
        // 按日期生成日志文件名
        QString fileName = m_logPath + "/log_" +
                          QDate::currentDate().toString("yyyy-MM-dd") + ".txt";

        QFile file(fileName);

        // 以追加模式打开
        if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);

            // 写入时间戳 + 日志级别 + 消息
            out << QDateTime::currentDateTime().toString("hh:mm:ss")
                << " [" << level << "] "
                << message << "\n";

            file.close();
        }
    }

    // 读取今天的所有日志
    QStringList readTodayLogs()
    {
        QString fileName = m_logPath + "/log_" +
                          QDate::currentDate().toString("yyyy-MM-dd") + ".txt";

        QFile file(fileName);
        QStringList logs;

        if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);

            while (!in.atEnd()) {
                logs << in.readLine();
            }

            file.close();
        }

        return logs;
    }

private:
    QString m_logPath;
};

void demonstrateLogManager()
{
    qDebug() << "\n=== 6. 简单日志管理器（综合示例） ===";

    SimpleLogManager logger("logs");

    // 写入不同级别的日志
    qDebug() << "写入日志...";

    logger.writeLog("INFO", "应用程序启动");
    logger.writeLog("DEBUG", "配置文件加载成功");
    logger.writeLog("WARN", "配置项 timeout 未设置，使用默认值 30");
    logger.writeLog("INFO", "数据库连接成功");
    logger.writeLog("ERROR", "无法连接到服务器，将在 5 秒后重试");

    // 读取并显示今天的日志
    qDebug() << "\n--- 读取今天的日志 ---";
    QStringList logs = logger.readTodayLogs();

    if (logs.isEmpty()) {
        qDebug() << "今天还没有日志记录";
    } else {
        for (const QString &log : logs) {
            qDebug() << "  " << log;
        }
    }
}

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========================================";
    qDebug() << "Qt 文件与 IO 示例程序";
    qDebug() << "========================================";

    // 运行各个演示函数
    demonstrateQFileBasic();
    demonstrateQTextStream();
    demonstrateQDataStream();
    demonstrateQDir();
    demonstrateQFileInfo();
    demonstrateLogManager();

    qDebug() << "\n========================================";
    qDebug() << "演示完成！生成的文件：";
    qDebug() << "  - example_text.txt (QFile 文本读写示例)";
    qDebug() << "  - config.ini (QTextStream 配置文件示例)";
    qDebug() << "  - data.bin (QDataStream 二进制数据示例)";
    qDebug() << "  - test_data/ (QDir 目录操作示例)";
    qDebug() << "  - test_info.txt (QFileInfo 信息查询示例)";
    qDebug() << "  - logs/log_*.txt (日志管理器示例)";
    qDebug() << "========================================";

    // 暂停一下，让用户看到输出（仅在调试时有用）
    // std::cout << "\n按 Enter 键退出...";
    // std::cin.get();

    return 0;
}
