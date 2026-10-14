---
title: "3.6 对话框进阶"
description: "exec() 一跑，主窗口旁边独立挂着的日志窗口跟着一起冻住。验证失败调了 reject()，调用方把用户点确定当成了点取消。向导做到第三步，各步控件的状态搅在一起。对话框拖到顺手的位置，下次打开又弹回默认角落。这些现象的根子分别在模态类型的阻塞范围、验证逻辑的拦截位置、向导的数据流设计和几何信息的持久化里，本篇逐个讲清。配套示例在 examples/advanced/03-qtwidgets/06-dialog-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.6——对话框进阶

[入门篇对话框体系基础](../../beginner/03-qtwidgets/06-dialog-system-beginner.md) 把 QDialog 的骨架搭好了，exec() 阻塞等待、accept() 和 reject() 关闭、QDialogButtonBox 配标准按钮、getter 把数据带回去。咱们弹一个简单的设置对话框，这些确实够用。可应用一旦多窗口、一上条件逻辑，麻烦就不在怎么弹出上了。您要是做过带独立工具窗口的软件，应该见过配置对话框一 exec()，旁边独立挂着的日志窗口跟着冻住，用户想翻一眼报警记录都点不动。验证逻辑写歪了同样闹误会，验证失败调 reject()，调用方把用户点确定当成了点取消。向导的数据要是散在各步控件里，回头改一步，后面的全乱。对话框的位置调好了，下次打开又弹回默认角落。这些麻烦的根子分别在模态类型的阻塞范围、验证的拦截位置、向导的数据流和几何信息的持久化里，咱们挨个讲清。

## 模态的阻塞范围：exec() 默认锁全部，WindowModal 只认父链

Qt::ApplicationModal 是 QDialog::exec() 的默认行为，它会锁住整个应用程序的所有窗口。咱们的应用同时开着主窗口、日志窗口和工具面板，任何一个 ApplicationModal 对话框弹出后，三个窗口全部无法接收用户输入。简单应用没问题，多窗口应用里就全冻在一起了。笔者翻过 Qt 6.9.1 的 QDialog 源码（qdialog.cpp），exec() 的文档注释写得很直白，对话框默认 application modal，函数转起一层嵌套事件循环等结果。注释还建议尽量改用 open()，说 exec() 容易引出一串危险 bug，比如对话框还开着就把父对象删了。

Qt::WindowModal 的范围窄得多，它只锁住对话框的父窗口以及父窗口的祖先窗口，其他独立的顶级窗口不受影响。主窗口 MainWin 打开对话框 A，A 的 parent 是 MainWin，A 设为 WindowModal，此时 MainWin 被锁住，和 MainWin 平级的 ToolWin 不受影响。嵌套对话框的场景里差别更明显，咱们看 DialogA 弹 SubDialogB 的例子，B 如果是 WindowModal 的，只锁住 A 和 MainWin，不影响其他窗口。

```cpp
// 父子关系：MainWin -> DialogA -> SubDialogB
// WindowModal：只有 DialogA 和 MainWin 冻结，ToolWin 仍可操作
sub_dialog->setWindowModality(Qt::WindowModal);
sub_dialog->exec();
```

咱们把三种模态的语义摆成表，选型的时候对着查：

| 模态类型             | 阻塞范围                   | 典型用法               |
| -------------------- | -------------------------- | ---------------------- |
| Qt::NonModal         | 不阻塞任何窗口             | 查找替换一类的浮动面板 |
| Qt::WindowModal      | 对话框的父窗口及其祖先窗口 | 多窗口应用的局部阻塞   |
| Qt::ApplicationModal | 应用内所有窗口             | 退出确认一类的全局决策 |

只认父链的语义，咱们换个场景就要出问题。假设应用有主窗口和一个独立日志窗口，两者没有父子关系，主窗口弹出 WindowModal 配置对话框，日志窗口仍可操作。配置流程要是依赖日志内容，用户在配置过程中清空了日志，两个窗口同时被操作，状态就对不上了，数据一致性从这里破掉。对策有两条：涉及跨窗口共享状态时用 ApplicationModal，或者把工具窗口设为主窗口的子窗口，让 WindowModal 自然覆盖到它。后一条更顺手，工具窗口反正常驻，认一个父窗口不算委屈它。

还有一个值得了解的细节，QDialog::open() 默认就是 WindowModal 的，而且不阻塞调用线程，结果通过 finished(int) 信号送回来。咱们要是嫌 exec() 的嵌套事件循环太重，open() 加 finished 信号是更轻量的替代方案。

配套示例在 `examples/advanced/03-qtwidgets/06-dialog-advanced/`，`cmake -B build && cmake --build build` 构建后运行。主窗口叫 ModalStrategyDemo，启动时旁边自动弹出一个没有父子关系的独立工具窗口，主窗口上两个按钮分别以 WindowModal 和 ApplicationModal 弹出 ValidatedDialog，就是下一节讲的那个带验证的端口对话框。咱们点“弹出 WindowModal 对话框”，趁对话框开着去点工具窗口里的“点击测试”按钮，按钮照样响应，底部状态标签记下“工具窗口按钮被点击了”。再点“弹出 ApplicationModal 对话框”试一遍，工具窗口整个冻住，怎么点都没反应。两种模态的边界在哪里，跑一轮就有体感了。

## 验证的拦截位置：重写 accept() 拦截，reject() 不表达验证失败

入门篇给过一套验证模式，咱们在自定义槽里先验证再调 accept()，验证不通过就 return。这套够用，但很多开发者会往前多走一步，在 accept() 槽里做验证，验证失败就调 reject()。坏就坏在多出来的 reject() 上，它把语义弄脏了。调用方看到 exec() 返回 Rejected，以为用户点了“取消”，其实是验证失败，“用户主动放弃”和“输入不合法被拒绝”两种完全不同的状态，从此分不开了。

咱们看一段有问题的代码：

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

调用方是 `if (dialog.exec() == QDialog::Accepted) { applyConfig(dialog.getPort()); }`。咱们跟着走一遍，用户点确定但端口为空时，reject() 导致 exec() 返回 Rejected，调用方以为用户取消。调用方在 Rejected 分支里有回滚逻辑的话，就会在用户点确定时意外执行回滚。正确做法是验证失败时直接 return，不调 reject() 也不调 accept()。

咱们再往深一层看，验证失败根本不该用任何关闭动作表达，对话框就不该关。最可靠的方式是重写 QDialog::accept()，先验证，通过再调 QDialog::accept()，不通过就保持对话框打开：

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

重写 accept() 比槽函数方案更稳，原因在于不依赖信号连接，无论 accept 从哪里触发，键盘、按钮还是别的代码路径，验证都会执行。咱们要是确实需要区分“取消”和“验证失败”，用 done() 传自定义结果码，调用方按返回值分流，语义就干净了。

另一个拦截点是重写 closeEvent(QCloseEvent*)，两条关闭路径只有一条经过它。点右上角的 X 关闭窗口，会触发 closeEvent，您要是在关闭前检查未保存的修改，调用 event->ignore() 可以阻止关闭。按 Escape 走的是另一条路，QDialog::keyPressEvent 把它直接交给 reject()，不经过 closeEvent，关闭前检查的逻辑放这儿就拦不到。

配套示例里的 ValidatedDialog 把验证做了两层。第一层在输入侧，QIntValidator 把输入限制在 1 到 65535 的整数，textChanged 连着 validateInput()，内容非法时 OK 按钮禁用、下方红字写明原因，内容有效时 OK 恢复启用。第二层就是重写的 accept()，toInt 之后复核范围，不合法就更新提示并 return，对话框原地不动。咱们输入 99999，OK 当场变灰，提示“端口号必须在 1-65535 之间。”，改成 8080，提示换成“输入有效。”，点确定后底部状态标签报出拿到的端口。按钮盒的 accepted 信号连的就是重写后的 accept，信号一响先进验证，输入侧这层管体验，accept() 复核这层才兜底。想看输入侧验证被绕开的样子，咱们把 validateInput() 里禁用按钮的两行临时注释掉，输入 99999 后点 OK，拦住它的是 accept() 的复核，红字提示照样弹出来，对话框照样不关。

## 多步向导不请 QWizard：步骤状态机加共享数据容器

QWizard 功能完善，自定义空间却有限，非线性跳转、条件性页面显示这些需求，在 QWizard 里实现起来很别扭。咱们自己搭向导，骨架就两件东西，一个步骤状态机，管当前步骤和跳转目标，一个共享数据容器，在所有步骤间传值，每个步骤读自己需要的字段、写自己负责的字段。

```cpp
struct WizardData
{
    int selected_type{0};       // 步骤 1 写入
    QString server_address;     // 步骤 2 写入
    int port{0};               // 步骤 2 写入
};
```

咱们拿一个三步设备配置向导把数据流走一遍。第一步选设备类型，QComboBox 里列 TCP 设备、串口设备、模拟设备。第二步配连接参数，按第一步的类型换控件，TCP 设备显示 IP 和端口，串口设备显示串口号和波特率，模拟设备不用额外参数。第三步只读确认，把 WizardData 里的内容摆出来给用户过目。

咱们在向导对话框内部用 QStackedWidget 管理所有步骤页面，底部放“上一步”“下一步”“取消”三个导航按钮。goNext() 的逻辑分三步，先验证当前步骤，再 commitCurrentStep() 把当前控件状态写回 WizardData，最后切换到下一步并让新页面的 onPageEntered(m_data) 根据已有数据刷新自身。第二步的 onPageEntered 读的就是 selected_type，按类型决定亮出哪组控件，联动从数据里来，不从控件里来。goBack() 不需要验证，用户回到上一步不涉及数据提交。最后一步的“下一步”按钮文字改为“完成”，点击后调 accept()。

数据和视图分了家，咱们改起来才有边界。步骤二不需要知道步骤一用了什么控件，它只读 m_data.selected_type 就够了。将来需要条件性跳过某个步骤，在 goNext 里加一个判断直接跳到再下一步即可，比如模拟设备不需要连接参数，goNext 在第一步就能直接落到第三步，向导的页面结构一行不用动。示例工程里没有现成的向导，咱们把 goNext 的三步走顺，动手搭的时候照着来。

## 位置和大小记到下次：saveGeometry 与 closeEvent 配合

用户把对话框拖到顺手的位置、调成舒服的大小，下次打开又弹回默认角落，产品级应用里这事说不过去。Qt 给了咱们现成的一对，saveGeometry() 和 restoreGeometry()，配上 QSettings 就能跨会话记住几何信息。

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

咱们把保存时机放在 closeEvent 而不是析构函数，原因在 saveGeometry() 需要窗口还处于有效状态。析构函数里窗口已经开始销毁，底层资源释放了，拿到的可能是无效数据，甚至读到零值。挂了 WA_DeleteOnClose 的对话框更隐蔽，finished 信号触发时窗口还在，可保存逻辑里要是有异步操作，等到实际写入时窗口可能已经没了。closeEvent 里同步保存最稳，但它在 QDialog 上只接住点 X 这一条关闭路径，OK、Cancel、Escape 的关闭都不经过它。咱们想让每条关闭路径都存上一份，把保存重写进 done() 里执行，accept()、reject() 和点 X 最终都汇进 done()，哪条路径也漏不掉。

restoreGeometry() 还带一份内置的屏幕有效性检查。用户上次在第二块显示器上拖了对话框，拔掉外接显示器后再打开，restoreGeometry() 发现保存的位置不在任何可用屏幕上，会自动调整到主屏幕的可见位置。咱们自己手动存 pos() 加 size() 做不到这一层，几何记忆请务必交给 saveGeometry() 和 restoreGeometry()。

示例工程没装几何记忆，咱们可以亲手补上。把上面两段补进 validated_dialog.cpp，给 ValidatedDialog 加 restoreDialogGeometry() 并在构造函数末尾调用，然后运行示例，把对话框拖到屏幕角落，用右上角的 X 关闭，再重新弹出，位置和大小原样回来。保存写在 closeEvent 里，点 X 关闭正好触发它，确定和取消两个按钮都不经过它，实验时别用错关闭方式。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，几何信息持久化用到 QtCore 的 QSettings，其余内容均在 QtWidgets 模块范围内。

[Qt 文档 · QDialog](https://doc.qt.io/qt-6/qdialog.html) -- 对话框基类，exec/open/accept/reject 完整接口和模态行为说明

[Qt 文档 · Qt::WindowModality](https://doc.qt.io/qt-6/qt.html#WindowModality-enum) -- 模态类型枚举，ApplicationModal 与 WindowModal 的语义定义

[Qt 文档 · QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 持久化设置存储，用于保存对话框几何信息

[Qt 文档 · QWidget](https://doc.qt.io/qt-6/qwidget.html) -- saveGeometry / restoreGeometry 接口，含屏幕有效性校验

[Qt 文档 · QStackedWidget](https://doc.qt.io/qt-6/qstackedwidget.html) -- 页面切换容器，适合多步向导的视图管理

[Qt 文档 · Restoring Window Geometry](https://doc.qt.io/qt-6/restoring-geometry.html) -- Qt 官方关于窗口几何信息保存恢复的专题指南

---

对话框的进阶就到这里。想再动手的，请您把示例完整玩一轮：点“弹出 WindowModal 对话框”，趁它开着去点旁边工具窗口的测试按钮，确认工具窗口没被冻结，再换 ApplicationModal 对比，然后在端口框里输 99999 看两层验证怎么拦，最后给 ValidatedDialog 补上几何记忆，用 X 关掉再弹出，看位置回不回得来。上一篇 [3.5 自定义控件进阶](./05-custom-widget-advanced.md) 把绘制拿到自己手里，下一篇 [3.7 主窗口进阶](./07-main-window-advanced.md) 讲 QMainWindow 的布局持久化和 Dock 管理，对话框之外的整扇主窗口，咱们去那边收拾。
