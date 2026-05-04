/**
 * Qt5Compat 迁移对照示例
 *
 * 本示例演示旧 API（Core5Compat）和新 API 的使用对比：
 * - QRegExp → QRegularExpression 迁移
 * - QTextCodec → QStringConverter 迁移
 * - 全局匹配、编码转换等常见场景的迁移对照
 *
 * 示例同时使用旧 API 和新 API 实现相同功能，
 * 迁移完成后只需保留新 API 写法并移除 Core5Compat 依赖即可。
 */

#include <QDebug>
#include <QCoreApplication>

#include "migrationutils.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt5Compat 迁移对照示例";
    qDebug() << "本示例对比展示旧 API（Core5Compat）和新 API 的使用方式";
    qDebug() << "";

    demoRegexMigration();
    demoRegexGlobalMatchMigration();
    demoCodecMigration();
    demoMigrationStrategy();

    qDebug() << "迁移对照演示完成";

    return 0;
}
