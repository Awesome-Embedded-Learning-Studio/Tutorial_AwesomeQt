---
title: "3.6 对话框进阶"
description: "入门篇我们学会了 QDialog 的基本用法——exec() 弹出、accept()/reject() 关闭。但在真实项目中，对话框的模态范围、输入验证、多步数据流和位置记忆这些问题，才是让对话框变得'工程可用'的关键。"
---

# 现代Qt开发教程（进阶篇）3.6——对话框进阶

## 1. 前言 / 为什么对话框还有进阶篇

入门篇我们把 QDialog 的骨架搭好了——exec() 阻塞等待、accept()/reject() 关闭、QDialogButtonBox 配标准按钮、getter 返回数据。说实话如果你只是弹一个简单的设置对话框，这些确实够用了。但真实项目里，对话框的麻烦远不止于此。我曾经维护过一个工业配置软件，三层嵌套对话框全用的默认 ApplicationModal，弹子对话框的时候连主窗口上正在滚动的报警日志都冻住了，运维人员差点报警。那次排查让我把模态范围从上到下梳理了一遍，才发现 Qt::WindowModal 和 Qt::ApplicationModal 的区别在多窗口场景下是决定性的。

这篇进阶篇聚焦四个在入门篇没展开的问题：模态范围的精确控制、输入验证的正确拦截位置、自定义多步向导的数据流管理、以及对话框几何信息的持久化记忆。这四个问题搞定了，你的对话框才算是"工程可用"的。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。几何信息持久化用到 QtCore 中的 QSettings，其余内容均在 QtWidgets 模块范围内。

## 3. 核心概念讲解

### 3.1 模态范围——ApplicationModal vs WindowModal 的本质区别

Qt::ApplicationModal 是 QDialog::exec() 的默认行为——它会锁住整个应用程序的所有窗口。如果你的应用同时打开了主窗口、日志窗口和工具面板，任何一个 ApplicationModal 对话框弹出后，三个窗口全部无法接收用户输入。对简单应用没问题，但多窗口应用中这就是灾难。

Qt::WindowModal 的范围更窄——它只锁住对话框的父窗口以及父窗口的祖先窗口，其他独立的顶级窗口不受影响。主窗口 MainWin 打开对话框 A（A 的 parent 是 MainWin），A 设为 WindowModal，此时 MainWin 被锁住，但和 MainWin 平级的 ToolWin 不受影响。嵌套对话框场景中这个区别更关键：DialogA 弹 SubDialogB 时，B 如果是 WindowModal 的，只锁住 A 和 MainWin，不影响其他窗口。

```cpp
// 父子关系：MainWin -> DialogA -> SubDialogB
// WindowModal：只有 DialogA 和 MainWin 冻结，ToolWin 仍可操作
sub_dialog->setWindowModality(Qt::WindowModal);
sub_dialog->exec();
```

另外一个值得了解的细节：QDialog::open() 默认就是 WindowModal 的，且不阻塞调用线程——通过 `finished(int)` 信号通知结果。如果你嫌 exec() 的嵌套事件循环太重，open() + finished 信号是更轻量的替代方案。

### 3.2 输入验证——别在 accept() 里做验证

入门篇我们给出的验证模式是在自定义槽中先验证再调 accept()，验证不通过就 return。这个模式够用，但很多开发者会在 accept() 槽里做验证，验证失败就调 reject()。后果是什么？调用方看到 exec() 返回 Rejected，以为用户点了"取消"，但其实是验证失败。调用方无法区分"用户主动放弃"和"输入不合法被拒绝"这两种完全不同的语义。

正确的做法是拦截关闭行为本身，不通过 reject() 表达验证失败——对话框根本就不该关闭。最可靠的方式是重写 `QDialog::accept()`：先验证，通过再调 `QDialog::accept()`，不通过就保持对话框打开。这比槽函数方案更好，因为它不依赖信号连接——无论 accept 从哪里触发，验证都会执行。

```cpp
void MyDialog::accept()
{
    if (!validateInputs()) {
        highlightInvalidFields();
        return;  // 对话框不会关闭
    }
    QDialog::accept();
}
```

另一个拦截点是重写 `closeEvent(QCloseEvent*)`。用户点 X 或按 Escape 时触发 closeEvent，如果你需要在关闭前检查未保存的修改，调用 `event->ignore()` 可以阻止关闭。

现在有一道调试题给大家。看下面这段代码：

```cpp
void ConfigDialog::onOkClicked()
{
    if (m_portEdit->text().isEmpty()) {
        m_portEdit->setFocus();
        reject();  // 端口为空，拒绝关闭
        return;
    }
    accept();
}
```

调用方是 `if (dialog.exec() == QDialog::Accepted) { applyConfig(dialog.getPort()); }`。问题出在哪里？用户点确定但端口为空时，reject() 导致 exec() 返回 Rejected，调用方以为用户取消。如果调用方在 Rejected 分支有回滚逻辑，就会在用户点确定时意外执行回滚。正确做法是验证失败时直接 return，不调 reject() 也不调 accept()。

### 3.3 自定义多步向导——不依赖 QWizard 的数据流管理

QWizard 功能完善但自定义空间有限——非线性跳转、条件性页面显示这些需求在 QWizard 里实现起来很别扭。自定义多步向导的核心是"步骤状态机"加一个共享数据容器：状态机管理当前步骤和跳转目标，数据容器在所有步骤间共享，每个步骤读自己需要的字段、写自己负责的字段。

```cpp
struct WizardData
{
    int selected_type{0};       // 步骤 1 写入
    QString server_address;     // 步骤 2 写入
    int port{0};               // 步骤 2 写入
};
```

向导对话框内部用 QStackedWidget 管理所有步骤页面，底部放"上一步""下一步""取消"三个导航按钮。goNext() 的逻辑分三步：先验证当前步骤，再 commitCurrentStep() 把当前控件状态写回 WizardData，最后切换到下一步并让新页面的 onPageEntered(m_data) 根据已有数据刷新自身。goBack() 不需要验证——用户回到上一步不涉及数据提交。最后一步的"下一步"按钮文字改为"完成"，点击后调 accept()。

这个设计的关键在于数据和视图的分离。步骤二不需要知道步骤一用了什么控件，它只需要读 m_data.selected_type 就够了。如果将来需要条件性跳过某个步骤，在 goNext 里加一个判断直接跳到再下一步即可。

### 3.4 对话框几何信息记忆——QSettings + closeEvent 配合

用户调整了对话框的大小和位置，下次打开又回到默认位置——产品级应用中这是不可接受的。Qt 提供了 saveGeometry() 和 restoreGeometry() 配合 QSettings 实现跨会话持久化。

```cpp
void MyDialog::closeEvent(QCloseEvent* event)
{
    QSettings settings("MyOrg", "MyApp");
    settings.beginGroup("MyDialog");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
    QDialog::closeEvent(event);
}

// 构造函数末尾调用
void MyDialog::restoreDialogGeometry()
{
    QSettings settings("MyOrg", "MyApp");
    settings.beginGroup("MyDialog");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();
}
```

为什么用 closeEvent 而不是析构函数？因为 saveGeometry() 需要窗口还处于有效状态，析构函数中窗口已经开始销毁，拿到的可能是无效数据。restoreGeometry() 还有一个内置的屏幕有效性检查：用户上次在第二块显示器上拖了对话框，拔掉外接显示器后 restoreGeometry() 发现保存的位置不在任何可用屏幕上了，会自动调整到主屏幕可见位置。这个细节自己手动存 pos() + size() 做不到，务必用 saveGeometry()/restoreGeometry()。

## 4. 踩坑预防

第一个坑是在 accept() 槽中做验证并调 reject() 拒绝关闭。前面调试题已经分析过了。后果是 exec() 返回值被污染——调用方无法区分"用户主动取消"和"验证失败被拒绝"。如果调用方在 Rejected 分支有回滚逻辑，就会在用户点确定时意外执行回滚。正确做法是验证失败直接 return，让对话框保持打开。如果确实需要区分"取消"和"验证失败"，用 done() 传自定义结果码。

第二个坑是 WindowModal 对话框只屏蔽父窗口而不屏蔽同级窗口。假设应用有主窗口和一个独立日志窗口（没有父子关系），主窗口弹了 WindowModal 配置对话框，日志窗口仍可操作。如果配置流程依赖日志内容，用户在配置过程中清空了日志就会出问题。后果是用户同时操作多个窗口导致数据不一致。解决方案：涉及跨窗口共享状态时用 ApplicationModal，或者把工具窗口设为主窗口的子窗口让 WindowModal 自然覆盖。

第三个坑是对话框关闭时未保存几何信息就销毁。在析构函数里调 saveGeometry()，此时窗口底层资源已经释放，读到的是零值。用 WA_DeleteOnClose 的对话框更隐蔽——finished 信号触发时窗口还在，但如果保存逻辑里有异步操作，等到实际写入时窗口可能已经没了。最安全的做法是在 closeEvent 中同步保存，这是窗口还完全有效时最后一个可靠的拦截点。

## 5. 练习项目

练习项目：三步设备配置向导。不使用 QWizard，完全手动搭建。第一步"选择设备类型"提供 QComboBox 从"TCP 设备""串口设备""模拟设备"中选择；第二步"配置连接参数"根据第一步的类型显示不同控件——TCP 设备显示 IP 和端口，串口设备显示串口号和波特率，模拟设备无需额外参数；第三步"确认配置"以只读文本展示所有信息。底部放"上一步""下一步""取消"，最后一步"下一步"文字变为"完成"。额外要求：对话框记忆自己的位置和大小，下次打开恢复。

完成标准：三步自由前进后退且控件状态正确恢复，设备类型切换时第二步控件正确更新，点击完成后调用方通过 getter 拿到所有配置数据，窗口位置关闭后正确持久化。提示几个关键点：共享数据结构传递步骤间数据，QStackedWidget 管理页面切换，saveGeometry/restoreGeometry 配合 QSettings 做位置记忆。

## 6. 官方文档参考链接

[Qt 文档 · QDialog](https://doc.qt.io/qt-6/qdialog.html) -- 对话框基类，exec/open/accept/reject 完整接口和模态行为说明

[Qt 文档 · Qt::WindowModality](https://doc.qt.io/qt-6/qt.html#WindowModality-enum) -- 模态类型枚举，ApplicationModal 与 WindowModal 的语义定义

[Qt 文档 · QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 持久化设置存储，用于保存对话框几何信息

[Qt 文档 · QWidget](https://doc.qt.io/qt-6/qwidget.html) -- saveGeometry / restoreGeometry 接口，含屏幕有效性校验

[Qt 文档 · QStackedWidget](https://doc.qt.io/qt-6/qstackedwidget.html) -- 页面切换容器，适合多步向导的视图管理

[Qt 文档 · Restoring Window Geometry](https://doc.qt.io/qt-6/restoring-geometry.html) -- Qt 官方关于窗口几何信息保存恢复的专题指南

---

到这里，对话框进阶就过了一遍。模态范围的精确控制让你在多窗口应用中不再误伤无辜窗口，验证逻辑的正确拦截位置保证了调用方语义不被污染，自定义多步向导的数据流方案比 QWizard 更灵活也更可控，几何信息记忆则是让对话框从"能用"变成"好用"的最后一块拼图。下一篇我们来看 QSS 样式系统——那才是让对话框从"好用"变成"好看"的开始。
