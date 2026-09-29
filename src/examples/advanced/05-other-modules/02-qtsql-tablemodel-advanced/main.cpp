/// @file    main.cpp
/// @brief   程序入口：演示 QSqlRelationalTableModel 的增删改查操作。
///
/// 对应教程：进阶层 05-其他模块/02-QSqlTableModel 高级用法。
/// 演示内容：
///   1. 创建 SQLite 内存数据库 + 三张表 + 样例数据
///   2. QSqlRelationalTableModel + setRelation() 外键映射
///   3. 打印全部员工（含部门名称）
///   4. 筛选 salary > 50000
///   5. 按 salary 降序排序
///   6. 插入新员工（指定部门引用）
///   7. 更新员工的部门

#include "relational_model.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRelationalTableModel>
#include <QSqlRecord>
#include <QSqlRelation>
#include <QDebug>
#include <QTimer>

/// @brief 打印 model 中当前所有记录。
/// @param[in] model    已执行 select() 的关系型模型。
/// @param[in] header   打印前显示的标题行。
/// @note 通过 record() 获取字段名，保证列顺序与模型一致。
static void printModelRows(QSqlRelationalTableModel* model, const QString& header)
{
    if (model == nullptr) {
        qWarning() << "[printModelRows] model is null.";
        return;
    }

    qDebug().noquote() << "";
    qDebug().noquote() << "===" << header << "===";

    // 打印表头
    QSqlRecord headerRec = model->record();
    QString headerLine;
    for (int col = 0; col < headerRec.count(); ++col) {
        headerLine += headerRec.fieldName(col).leftJustified(20, ' ');
    }
    qDebug().noquote() << headerLine;
    qDebug().noquote() << QString("-").repeated(headerLine.size());

    // 遍历所有行并打印
    for (int row = 0; row < model->rowCount(); ++row) {
        QString rowLine;
        QSqlRecord rec = model->record(row);
        for (int col = 0; col < rec.count(); ++col) {
            rowLine += rec.value(col).toString().leftJustified(20, ' ');
        }
        qDebug().noquote() << rowLine;
    }

    qDebug().noquote() << QString("=").repeated(headerLine.size());
}

/// @brief 通过原始 SQL 执行多表 JOIN 查询并打印结果。
/// @param[in] db     已打开的数据库连接。
/// @param[in] sql    要执行的 SELECT 语句。
/// @param[in] header 打印前显示的标题行。
/// @note QSqlRelationalTableModel 只支持单列外键映射，
///       复杂多表 JOIN 仍需使用 QSqlQuery 手动编写 SQL。
static void printJoinQuery(QSqlDatabase& db, const QString& sql, const QString& header)
{
    qDebug().noquote() << "";
    qDebug().noquote() << "===" << header << "===";

    QSqlQuery query(db);
    if (!query.exec(sql)) {
        qWarning() << "[printJoinQuery] query failed:" << query.lastError().text();
        return;
    }

    // 打印列名
    QSqlRecord rec = query.record();
    QString headerLine;
    for (int col = 0; col < rec.count(); ++col) {
        headerLine += rec.fieldName(col).leftJustified(20, ' ');
    }
    qDebug().noquote() << headerLine;
    qDebug().noquote() << QString("-").repeated(headerLine.size());

    // 打印每行数据
    while (query.next()) {
        QString rowLine;
        for (int col = 0; col < rec.count(); ++col) {
            rowLine += query.value(col).toString().leftJustified(20, ' ');
        }
        qDebug().noquote() << rowLine;
    }

    qDebug().noquote() << QString("=").repeated(headerLine.size());
}

/// @brief 主函数：创建数据库、配置模型、演示增删改查。
/// @param[in] argc 命令行参数个数。
/// @param[in] argv 命令行参数数组。
/// @return 应用程序退出码。
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // -----------------------------------------------------------------------
    // 步骤 1：创建数据库并填充样例数据
    // -----------------------------------------------------------------------
    RelationalModelSetup setup;
    if (!setup.createAndPopulateDatabase()) {
        qCritical() << "[main] Failed to create database. Exiting.";
        return 1;
    }

    // -----------------------------------------------------------------------
    // 步骤 2：配置 QSqlRelationalTableModel
    // -----------------------------------------------------------------------
    QSqlRelationalTableModel* model = setup.setupRelationalModel();
    if (model == nullptr) {
        qCritical() << "[main] Failed to setup relational model. Exiting.";
        return 1;
    }

    // -----------------------------------------------------------------------
    // 步骤 3：打印全部员工（dept_id 已映射为部门名称）
    // -----------------------------------------------------------------------
    printModelRows(model, "All Employees (dept_id mapped to department name)");

    // -----------------------------------------------------------------------
    // 步骤 4：筛选 salary > 50000
    // -----------------------------------------------------------------------
    // setFilter() 等价于 SQL 的 WHERE 子句，不包含 WHERE 关键字本身
    model->setFilter("salary > 50000");
    if (!model->select()) {
        qWarning() << "[main] filter select failed:" << model->lastError().text();
    }
    printModelRows(model, "Employees with salary > 50000");

    // 清除筛选条件，恢复全量数据
    model->setFilter("");
    if (!model->select()) {
        qWarning() << "[main] clear filter select failed:" << model->lastError().text();
    }

    // -----------------------------------------------------------------------
    // 步骤 5：按 salary 降序排序
    // -----------------------------------------------------------------------
    // setSort(列索引, 排序方向) 等价于 ORDER BY 子句
    // 列 3 = salary（0-based 索引：id=0, name=1, dept_id=2, salary=3）
    model->setSort(3, Qt::DescendingOrder);
    if (!model->select()) {
        qWarning() << "[main] sort select failed:" << model->lastError().text();
    }
    printModelRows(model, "Employees sorted by salary DESC");

    // -----------------------------------------------------------------------
    // 步骤 6：插入新员工（指定部门引用）
    // -----------------------------------------------------------------------
    // 使用 insertRecord() 比直接操作数据库更安全，
    // 因为 QSqlRelationalTableModel 会自动处理外键映射。
    // 注意：setRelation(2, ...) 后，列 2 的内部字段名已被 Qt 重命名，
    //       但 setValue("dept_id", ...) 仍然有效——Qt 内部会映射回外键列。
    //       此处使用列索引赋值，更直观且不依赖字段名字符串。
    QSqlRecord newEmployee = model->record();
    newEmployee.setValue(1, "Grace");          // 列 1 = name
    newEmployee.setValue(2, 1);                // 列 2 = dept_id (1=Engineering)
    newEmployee.setValue(3, 68000.0);          // 列 3 = salary
    // 列 0 = id，不设置则 SQLite AUTOINCREMENT 自动分配

    if (!model->insertRecord(-1, newEmployee)) {
        qWarning() << "[main] insertRecord failed:" << model->lastError().text();
    } else if (!model->submitAll()) {
        // OnManualSubmit 模式必须显式提交
        qWarning() << "[main] submitAll after insert failed:" << model->lastError().text();
        model->revertAll();
    } else {
        qDebug().noquote() << "";
        qDebug().noquote() << "[Insert] Added Grace to Engineering with salary 68000";
    }

    // 恢复默认排序（按 id 升序）并刷新
    model->setSort(0, Qt::AscendingOrder);
    if (!model->select()) {
        qWarning() << "[main] re-select after insert failed:" << model->lastError().text();
    }
    printModelRows(model, "All Employees after inserting Grace");

    // -----------------------------------------------------------------------
    // 步骤 7：更新员工部门（将 Diana 从 Marketing 调到 Engineering）
    // -----------------------------------------------------------------------
    // 使用 setFilter 定位目标行，比遍历所有行更高效
    // 必须用 "employees.name" 限定表名，因为 setRelation() 会产生 LEFT JOIN，
    // 导致 employees.name 和 departments.name 同时存在，引发歧义
    model->setFilter("employees.name = 'Diana'");
    if (!model->select()) {
        qWarning() << "[main] filter for Diana failed:" << model->lastError().text();
    }

    if (model->rowCount() > 0) {
        // setData(ModelIndex, value) 修改指定单元格
        // 使用 model->index(row, col) 获取模型索引
        QModelIndex deptIndex = model->index(0, 2);   // 列 2 = dept_id
        if (!model->setData(deptIndex, 1)) {           // 1 = Engineering
            qWarning() << "[main] setData failed:" << model->lastError().text();
        } else if (!model->submitAll()) {
            qWarning() << "[main] submitAll after update failed:"
                       << model->lastError().text();
            model->revertAll();
        } else {
            qDebug().noquote() << "";
            qDebug().noquote() << "[Update] Moved Diana from Marketing to Engineering";
        }
    }

    // 清除筛选，查看更新后的全量数据
    model->setFilter("");
    if (!model->select()) {
        qWarning() << "[main] final select failed:" << model->lastError().text();
    }
    printModelRows(model, "All Employees after moving Diana to Engineering");

    // -----------------------------------------------------------------------
    // 步骤 8：演示多表 JOIN（QSqlQuery 手动编写）
    // -----------------------------------------------------------------------
    // QSqlRelationalTableModel 仅支持单列外键映射，
    // 复杂查询仍需手写 SQL，这里展示三表 JOIN
    QSqlDatabase db = setup.database();
    printJoinQuery(
        db,
        "SELECT e.name AS employee, d.name AS department, t.title "
        "FROM employees e "
        "LEFT JOIN departments d ON e.dept_id = d.id "
        "LEFT JOIN titles t ON (e.salary >= 65000 AND t.title = 'Senior Engineer') "
        "ORDER BY e.name",
        "Multi-table JOIN (employees + departments + titles)");

    // -----------------------------------------------------------------------
    // 控制台应用自动退出，无需进入事件循环
    // -----------------------------------------------------------------------
    qDebug().noquote() << "";
    qDebug().noquote() << "Demo completed successfully. Exiting.";

    QTimer::singleShot(0, &app, &QCoreApplication::quit);
    return app.exec();
}
