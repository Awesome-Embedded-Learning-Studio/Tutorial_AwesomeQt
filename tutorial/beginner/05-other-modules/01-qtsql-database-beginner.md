# 现代Qt开发教程（新手篇）5.1——QtSql 数据库连接与查询基础

## 1. 前言：应用开发绕不开的那道坎

说实话，做应用开发做到一定程度，一定会撞上数据持久化这堵墙。你可以用文件、用 JSON、用 XML，但当你需要结构化查询、事务保证、并发读写的时候，这些方案统统不够看。数据库不是什么高深的东西，它就是一个帮你管理结构化数据的工具，而 Qt 内置了对 SQLite 的完整支持，开箱即用，连驱动都不用装。

这篇我们要做的是把 QtSql 模块的核心用法搞清楚——从连接数据库、执行 SQL、预编译防注入，到错误处理和事务管理，一条线走到底。你跟着走完一遍之后，在自己的项目里集成数据库操作就不会再有心理障碍了。

## 2. 环境说明

本篇基于 Qt 6.9.1，所有数据库相关类都在 QtSql 模块里，CMake 配置需要额外引入 `Qt6::Sql`。示例使用 SQLite 作为数据库引擎——它不需要安装任何服务端，数据库就是一个本地文件，非常适合嵌入式场景和桌面应用的本地存储。编译工具链方面，MSVC 2019+、GCC 11+、Clang 14+ 均可，C++17 标准，CMake 3.26+ 构建系统。

## 3. 核心概念讲解

### 3.1 连接数据库——QSqlDatabase 的基本用法

Qt 里所有数据库操作的起点都是 `QSqlDatabase`。你可以把它理解为一个数据库连接的句柄，所有后续的查询、事务、错误处理都建立在这个连接之上。

SQLite 是 Qt 内置驱动的数据库，连接方式非常简洁：

```cpp
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// 添加一个 SQLite 数据库连接
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName("myapp.db");  // 数据库文件名（相对或绝对路径）

if (!db.open()) {
    qDebug() << "数据库打开失败:" << db.lastError().text();
    return;
}
qDebug() << "数据库连接成功";
```

这里有几个关键点需要说清楚。`QSqlDatabase::addDatabase("QSQLITE")` 的参数是驱动名，Qt 支持的驱动名包括 `"QSQLITE"`（SQLite）、`"QMYSQL"`（MySQL）、`"QPSQL"`（PostgreSQL）、`"QODBC"`（ODBC）等。驱动名是大小写敏感的，写错了会直接报驱动未找到的错误。

`setDatabaseName()` 对 SQLite 来说就是数据库文件路径。如果文件不存在，SQLite 会自动创建一个新的空数据库文件。这意味着你不需要预先建库，第一次 `open()` 的时候一切都会自动就位。但注意，如果你传入的是一个绝对路径，确保目录存在——SQLite 不会帮你创建目录，只会在已有目录下创建文件。

连接名这个概念也要提一下。当你调用 `addDatabase()` 时，Qt 会给这个连接分配一个默认名称 `"qt_sql_default_connection"`。如果你的应用只需要一个数据库连接，默认名称就够了。但如果需要同时连接多个数据库（比如一个本地 SQLite 加一个远程 MySQL），就需要给每个连接起个名字：

```cpp
// 第二个参数是连接名
QSqlDatabase db1 = QSqlDatabase::addDatabase("QSQLITE", "local_db");
QSqlDatabase db2 = QSqlDatabase::addDatabase("QMYSQL", "remote_db");
```

之后通过 `QSqlDatabase::database("local_db")` 来获取对应的连接。默认连接名的好处是 `QSqlQuery` 构造时不传 `QSqlDatabase` 参数就会自动使用默认连接，省事不少。

数据库用完之后要关闭，但关闭之前需要确保所有 `QSqlQuery` 对象已经销毁或者不再持有引用，否则 `close()` 可能会失败或者产生警告。推荐的做法是让 `QSqlQuery` 的生命周期短于 `QSqlDatabase`——用完了就销毁，在程序退出前统一 `close()`。

### 3.2 执行 SQL——QSqlQuery 的基本操作

连接建好了，接下来就是执行 SQL 语句。Qt 提供了 `QSqlQuery` 类来完成这个任务，它既可以执行 DDL（建表、删表），也可以执行 DML（增删改查）。

先来建一张表：

```cpp
QSqlQuery query;
bool ok = query.exec(
    "CREATE TABLE IF NOT EXISTS employees ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  name TEXT NOT NULL,"
    "  department TEXT,"
    "  salary REAL,"
    "  hire_date TEXT"
    ")"
);

if (!ok) {
    qDebug() << "建表失败:" << query.lastError().text();
}
```

`query.exec()` 接收一个 SQL 字符串并执行，返回 `bool` 表示成功或失败。失败的时候通过 `query.lastError()` 获取错误信息。这个模式在整个 QtSql 模块里是一致的——几乎每个操作都返回 `bool`，每个对象都有 `lastError()` 方法。

插入数据：

```cpp
query.exec("INSERT INTO employees (name, department, salary, hire_date) "
           "VALUES ('Alice', 'Engineering', 85000.0, '2024-03-15')");
query.exec("INSERT INTO employees (name, department, salary, hire_date) "
           "VALUES ('Bob', 'Marketing', 65000.0, '2024-06-01')");
```

查询数据就需要遍历结果集了：

```cpp
QSqlQuery query("SELECT name, department, salary FROM employees WHERE salary > 70000");

while (query.next()) {
    QString name = query.value(0).toString();
    QString dept = query.value(1).toString();
    double salary = query.value(2).toDouble();
    qDebug() << name << dept << salary;
}
```

`query.next()` 把游标移动到下一行记录，在第一次调用时移动到第一行。`query.value(index)` 按列索引（从 0 开始）获取当前行的字段值，返回的是 `QVariant`，你需要调用 `.toString()`、`.toInt()`、`.toDouble()` 等方法转换成具体类型。

你也可以用列名来获取值，这比用索引更清晰：

```cpp
QSqlQuery query("SELECT * FROM employees");
while (query.next()) {
    int id = query.value("id").toInt();
    QString name = query.value("name").toString();
    double salary = query.value("salary").toDouble();
    qDebug() << id << name << salary;
}
```

按列名获取的底层机制是 SQLite 驱动会维护列名到索引的映射。但要注意，如果你在 SQL 里用了别名（比如 `SELECT salary * 12 AS annual_salary`），那 `value()` 的参数必须用别名 `"annual_salary"` 而不是原始列名。

更新和删除操作同样通过 `exec()` 完成：

```cpp
query.exec("UPDATE employees SET salary = 90000 WHERE name = 'Alice'");
query.exec("DELETE FROM employees WHERE department = 'Marketing'");
```

`exec()` 执行的是 INSERT、UPDATE、DELETE 这类不返回结果集的语句时，你可以通过 `query.numRowsAffected()` 获取受影响的行数。这个方法对判断操作是否真正生效很有用——比如你 UPDATE 了一行但实际上没有匹配到任何记录，`numRowsAffected()` 会返回 0。

### 3.3 预编译与参数绑定——SQL 注入的终结者

前面我们直接把值拼进了 SQL 字符串里，这在教学演示里没问题，但在实际工程中这是大忌。原因很简单——SQL 注入。如果用户输入的名字里包含单引号或者 SQL 关键字，拼出来的 SQL 语句结构就会被破坏，轻则查询结果错误，重则整个数据库被清空。

Qt 提供了 `prepare()` + `bindValue()` 的预编译机制来彻底解决这个问题：

```cpp
QSqlQuery query;
query.prepare("INSERT INTO employees (name, department, salary, hire_date) "
              "VALUES (:name, :dept, :salary, :hire_date)");

query.bindValue(":name", "Charlie");
query.bindValue(":dept", "Engineering");
query.bindValue(":salary", 95000.0);
query.bindValue(":hire_date", "2024-09-01");

if (!query.exec()) {
    qDebug() << "插入失败:" << query.lastError().text();
}
```

这里的 `:name`、`:dept` 等是命名占位符，`bindValue()` 把实际值绑定上去。Qt 的驱动层会负责正确的转义和类型转换，你传什么值它都能正确处理——字符串里的单引号、特殊字符、二进制数据，全部安全。

除了命名占位符，Qt 也支持位置占位符：

```cpp
QSqlQuery query;
query.prepare("INSERT INTO employees (name, department, salary) VALUES (?, ?, ?)");

query.addBindValue("Diana");
query.addBindValue("Sales");
query.addBindValue(55000.0);

query.exec();
```

位置占位符用 `?` 表示，通过 `addBindValue()` 按顺序绑定。两种方式功能完全等价，命名占位符可读性更好，位置占位符写起来更紧凑。个人偏好命名占位符，因为列多了之后你不会搞混第几个 `?` 对应哪个字段。

预编译还有一个容易被忽略的好处：性能。当你需要批量插入大量数据时，同一个 `prepare()` 只需要编译一次 SQL 语句，后续每次只更换绑定值即可。比起每次 `exec()` 都传一个全新的 SQL 字符串让数据库重新解析，预编译方式能显著减少解析开销。后面讲到事务的时候我们会把这个优势发挥到极致。

### 3.4 错误处理——QSqlError 的正确打开方式

数据库操作出错是常态，不是异常。文件被占用、磁盘满了、SQL 语法写错了、约束冲突了，各种原因都可能导致操作失败。所以错误处理必须作为正常逻辑的一部分来写，而不是事后补救。

`QSqlError` 是 QtSql 里统一的错误描述类。每个 `QSqlQuery` 和 `QSqlDatabase` 都有 `lastError()` 方法返回最近一次操作的错误信息：

```cpp
QSqlQuery query;
if (!query.exec("SELECT * FROM nonexistent_table")) {
    QSqlError error = query.lastError();

    qDebug() << "错误类型:" << error.type();        // QSqlError::StatementError
    qDebug() << "错误编号:" << error.nativeErrorCode();  // SQLite 的错误码
    qDebug() << "错误描述:" << error.databaseText();     // 数据库返回的原始文本
    qDebug() << "驱动描述:" << error.driverText();       // 驱动层面的描述
    qDebug() << "完整信息:" << error.text();             // 综合描述
}
```

`QSqlError::type()` 返回的错误类型有四种：`NoError`（没有错误）、`ConnectionError`（连接失败）、`StatementError`（SQL 语句错误）、`TransactionError`（事务操作失败）。根据类型你可以大致判断问题出在哪个环节。

在实际工程里，建议把数据库错误统一记录到日志系统，而不是只在控制台打印：

```cpp
bool executeQuery(QSqlQuery &query, const QString &sql)
{
    if (!query.exec(sql)) {
        QSqlError err = query.lastError();
        qWarning() << "SQL 执行失败:"
                    << "type=" << err.type()
                    << "text=" << err.text()
                    << "sql=" << sql;
        return false;
    }
    return true;
}
```

这样你能在生产环境里回溯问题。光有错误描述还不够，把出问题的 SQL 语句一起记录下来，调试效率会高很多。

### 3.5 事务管理——要么全成功，要么全回滚

事务是数据库保证数据一致性的核心机制。简单来说，事务把一组操作捆绑在一起，要么全部成功，要么全部撤销，不存在"一半成功一半失败"的中间状态。

典型的场景是批量插入。假设你要插入 1000 条记录，如果没有事务，每条 INSERT 都是一次独立的写操作，数据库要写 1000 次磁盘。如果第 500 条失败了，前 499 条已经写进去了，你还得想办法清理。用了事务之后，1000 条 INSERT 在内存中累积，最后一次 COMMIT 写入磁盘——性能提升巨大，而且一旦失败自动 ROLLBACK，数据回到事务开始前的状态。

```cpp
QSqlDatabase db = QSqlDatabase::database();  // 获取默认连接
QSqlQuery query;

if (!db.transaction()) {
    qDebug() << "事务启动失败:" << db.lastError().text();
    return;
}

query.prepare("INSERT INTO employees (name, department, salary) VALUES (?, ?, ?)");

bool success = true;
for (int i = 0; i < 1000; ++i) {
    query.addBindValue(QString("Employee_%1").arg(i));
    query.addBindValue("Batch");
    query.addBindValue(50000.0 + i * 10);

    if (!query.exec()) {
        qDebug() << "批量插入失败，第" << i << "条:" << query.lastError().text();
        success = false;
        break;
    }
}

if (success) {
    db.commit();     // 全部成功，提交事务
    qDebug() << "批量插入完成";
} else {
    db.rollback();   // 有失败，回滚事务
    qDebug() << "批量插入已回滚";
}
```

这里有一个非常重要的细节：`transaction()`、`commit()`、`rollback()` 都是 `QSqlDatabase` 的方法，不是 `QSqlQuery` 的。事务是绑定在连接级别的，不是语句级别的。一个事务里可以执行多条不同的 SQL 语句——INSERT、UPDATE、DELETE 混着来都行，只要它们使用的是同一个数据库连接。

事务还有一个容易踩的坑：嵌套事务。SQLite 本身不支持真正的嵌套事务，如果你在未提交的事务里又调了 `transaction()`，行为是未定义的。如果确实需要嵌套操作，SQLite 提供了 SAVEPOINT 机制，可以通过直接执行 `SAVEPOINT sp1` 和 `RELEASE sp1` 这样的 SQL 语句来模拟，但这已经超出了入门篇的范围。

## 4. 综合示例：一个完整的数据库操作流程

把前面学的串起来，我们写一个完整的流程——建库、建表、批量插入、条件查询、更新、删除，每一步都做错误检查：

```cpp
void databaseDemo()
{
    // 1. 连接数据库
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("company.db");

    if (!db.open()) {
        qWarning() << "无法打开数据库:" << db.lastError().text();
        return;
    }

    // 2. 建表
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS employees ("
               "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "  name TEXT NOT NULL,"
               "  department TEXT,"
               "  salary REAL)");

    // 3. 批量插入（事务 + 预编译）
    db.transaction();
    query.prepare("INSERT INTO employees (name, department, salary) "
                  "VALUES (:name, :dept, :salary)");

    QStringList names = {"Alice", "Bob", "Charlie", "Diana", "Eve"};
    for (int i = 0; i < names.size(); ++i) {
        query.bindValue(":name", names[i]);
        query.bindValue(":dept", i % 2 == 0 ? "Engineering" : "Marketing");
        query.bindValue(":salary", 60000.0 + i * 5000);
        query.exec();
    }
    db.commit();

    // 4. 查询
    QSqlQuery selectQuery("SELECT name, department, salary FROM employees "
                          "WHERE salary > 65000 ORDER BY salary DESC");
    while (selectQuery.next()) {
        qDebug() << selectQuery.value("name").toString()
                 << selectQuery.value("department").toString()
                 << selectQuery.value("salary").toDouble();
    }

    // 5. 关闭连接
    db.close();
}
```

这段代码覆盖了日常开发中最常见的数据库操作模式。你可以把它作为模板，根据实际需求增删修改。

## 5. 练习项目

练习项目：简易通讯录管理程序。

我们要做一个基于 SQLite 的命令行通讯录，支持添加联系人、按姓名搜索、显示所有联系人、删除联系人这几个核心操作。每个联系人包含姓名、电话号码、邮箱和分组四个字段。

完成标准是这样的：程序启动时自动创建数据库和表（如果不存在）；支持 `add` 命令添加联系人（使用预编译参数绑定）；支持 `search` 命令按姓名模糊搜索（SQL LIKE 语句）；支持 `list` 命令列出所有联系人并按姓名排序；支持 `delete` 命令按 ID 删除联系人；批量导入功能使用事务保证原子性。

几个实现提示：建表 SQL 用 `CREATE TABLE IF NOT EXISTS` 确保幂等性；搜索时用 `query.prepare("SELECT * FROM contacts WHERE name LIKE :pattern")` 配合 `bindValue(":pattern", "%" + keyword + "%")` 实现模糊匹配；删除操作前先查询确认记录存在，避免静默失败。

## 6. 官方文档参考

[Qt 文档 · QSqlDatabase](https://doc.qt.io/qt-6/qsqldatabase.html) -- 数据库连接管理

[Qt 文档 · QSqlQuery](https://doc.qt.io/qt-6/qsqlquery.html) -- SQL 语句执行与结果遍历

[Qt 文档 · QSqlError](https://doc.qt.io/qt-6/qsqlerror.html) -- 数据库错误信息

[Qt 文档 · SQL Driver](https://doc.qt.io/qt-6/sql-driver.html) -- Qt 支持的数据库驱动列表

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了！QtSql 模块的核心并不复杂——`QSqlDatabase` 管连接、`QSqlQuery` 管执行、`QSqlError` 管错误、事务管一致性。掌握了这四个核心组件，日常的数据库操作基本都能覆盖。下一篇我们看看 `QSqlTableModel`，它把数据库表直接映射成了 Qt 的 Model，配合 `QTableView` 可以实现零代码的数据库浏览与编辑。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
