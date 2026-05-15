---
title: "1.7 事件系统进阶：自定义事件与过滤器"
description: "入门篇我们了解了 QEvent 的基本概念、事件循环的工作流程，以及 sendEvent 和 postEvent 的区别。"
---

# 现代Qt开发教程（进阶篇）1.7——事件系统进阶：自定义事件与过滤器

## 1. 前言 / 事件系统是 Qt 的心脏

入门篇我们了解了 QEvent 的基本概念、事件循环的工作流程，以及 sendEvent 和 postEvent 的区别。说实话，写几个 demo 体验一下事件传递确实够用了，但工程项目里事件系统的使用远不止「覆写 paintEvent」这么简单。自定义事件类型的注册、跨线程事件投递的线程安全、事件过滤器的优先级链——这些才是真正让你能驾驭 Qt 事件驱动模型的关键知识。

我之前在一个实时数据可视化项目里踩过一个坑：工作线程高频 postEvent 到主线程，事件队列堆积导致 UI 卡顿，最后不得不做事件压缩（合并同类事件）才解决。还有一次是事件过滤器的安装顺序搞反了，导致快捷键被错误拦截，排查了整整一天。这些经验教训，我们今天一并分享。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。事件系统的核心 API（QEvent、QCoreApplication::postEvent、QObject::eventFilter 等）在 Qt 5 和 Qt 6 之间没有大的 API 变动。示例只依赖 QtCore 模块。所有示例可用控制台程序验证。

## 3. 核心概念讲解

### 3.1 自定义事件类型——QEvent 子类与类型注册

Qt 预定义了大量事件类型（QEvent::Type 枚举），从鼠标键盘到布局请求到定时器事件。但当你需要跨组件传递自定义数据时，就需要创建自己的事件类型。

创建自定义事件的第一步是继承 QEvent 并注册一个唯一的事件类型 ID。注册用 `QEvent::registerEventType()` 完成，它返回一个保证不与 Qt 内置类型冲突的整数 ID。

```cpp
class DataReadyEvent : public QEvent
{
public:
    static const QEvent::Type kType =
        static_cast<QEvent::Type>(QEvent::registerEventType());

    explicit DataReadyEvent(const QVector<double>& data)
        : QEvent(kType), m_data(data)
    {
    }

    QVector<double> data() const { return m_data; }

private:
    QVector<double> m_data;
};
```

使用时通过 postEvent 投递到目标对象的事件队列。目标对象覆写 `event()` 方法来处理。postEvent 接管了事件对象的所有权，在事件处理完毕后自动 delete——这意味着你必须用 new 创建事件对象，不能用栈上的对象。

### 3.2 事件过滤器——installEventFilter 与优先级

事件过滤器是 Qt 提供的一种强大机制：在事件到达目标对象的 event() 方法之前，先经过过滤器的拦截处理。一个对象可以安装多个过滤器，形成一条过滤链。

返回 true 表示事件被消费，不再传递到目标对象。返回 false 表示事件继续传递。过滤链的执行顺序是后安装的先执行（LIFO）——如果你先安装了过滤器 A 再安装 B，事件到达时先经过 B 再经过 A。这在安装多个过滤器时需要特别注意顺序。

### 3.3 跨线程事件投递与事件压缩

QCoreApplication::postEvent 是线程安全的，你可以从任何线程向任何线程的对象投递事件。但 postEvent 有一个性能陷阱：如果投递速度远快于处理速度，事件队列会无限增长。

现在有一道调试题。下面这段代码有什么问题？

```cpp
// 工作线程：高频投递数据事件
void DataThread::run()
{
    while (m_running) {
        auto* data = readSensor();
        QCoreApplication::postEvent(m_target, new DataReadyEvent(data));
        // 没有任何延迟或压缩
    }
}
```

问题是：如果 readSensor 每秒返回 10000 次，而主线程的事件循环每秒只能处理 1000 个事件，事件队列每秒增长 9000 个。几分钟后程序就会因为内存耗尽而崩溃。解决方案是做事件压缩——在 postEvent 之前检查队列中是否已有同类型未处理的事件，如果有就合并数据而不是新增事件。

## 4. 踩坑预防

第一个坑是 postEvent 的内存所有权混乱。postEvent 接管了事件对象的所有权，处理完后自动 delete。如果你对同一个事件对象既 postEvent 又手动 delete，就是 double free。后果是程序崩溃，崩溃点在事件循环内部的 delete 操作中。解决方案是：postEvent 之后永远不要再操作那个事件对象。如果事件处理前需要取消，用 `QCoreApplication::removePostedEvents()` 而不是手动 delete。

第二个坑是事件过滤器返回值的语义搞反。true = 吃掉事件（拦截），false = 放行。很多初学者把 true 当成「处理了但继续传递」，结果导致控件收不到本该收到的事件。后果是键盘输入失效、鼠标点击无响应。记住：true = 吃掉，false = 放行。

第三个坑是 sendEvent 在错误线程中使用。sendEvent 是同步调用，直接调用目标对象的 event() 方法。如果工作线程中用 sendEvent 发送 Paint 事件给 QWidget，paintEvent 就在工作线程执行——但 GUI 操作必须在主线程。后果是不可预测——可能正常，可能画面错乱，可能直接崩溃。涉及 GUI 的事件必须用 postEvent 或信号槽。

## 5. 练习项目

练习项目：带事件压缩的数据采集显示器。模拟高频数据采集场景，工作线程每秒产生 1000 个数据事件，主线程实时显示但不卡顿。

具体要求是：DataProducer 在工作线程每毫秒产生一个数据点，通过 postEvent 发送到 DataConsumer。DataConsumer 实现事件压缩——队列中已有未处理的同类事件时合并数据而不是新增。完成标准是内存占用恒定不增长、显示更新频率稳定在每秒 60 次。

提示几个关键点：用原子布尔标志位标记「有待处理事件」，处理时清除标志位。用 QElapsedTimer 统计实际处理频率。

## 6. 官方文档参考链接

[Qt 文档 · QEvent](https://doc.qt.io/qt-6/qevent.html) -- 事件基类参考

[Qt 文档 · QCoreApplication::postEvent](https://doc.qt.io/qt-6/qcoreapplication.html#postEvent) -- 异步事件投递

[Qt 文档 · QObject::installEventFilter](https://doc.qt.io/qt-6/qobject.html#installEventFilter) -- 事件过滤器安装

---

到这里，事件系统的进阶知识就拆完了。自定义事件、事件过滤器链、跨线程事件安全、高频事件压缩——这些是构建事件驱动架构的必备技能。下一篇我们来看多线程进阶。
