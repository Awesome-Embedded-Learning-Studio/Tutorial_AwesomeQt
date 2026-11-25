---
title: "1.9 多线程进阶：线程池、并发与锁"
description: "入门篇我们聊了 QThread 的基本用法——子类化或 moveToThread 两种方式，加上基础的互斥锁和条件变量。说实话，会创建线程、会加锁，只代表你知道了工具。"
---

# 现代Qt开发教程（进阶篇）1.9——多线程进阶：线程池、并发与锁

## 1. 前言 / 线程不是越多越快

入门篇我们聊了 QThread 的基本用法——子类化或 moveToThread 两种方式，加上基础的互斥锁和条件变量。说实话，会创建线程、会加锁，只代表你知道了工具。什么时候该用线程池、什么时候该用读写锁而不是互斥锁、QtConcurrent 怎么用才真正省心——这些才是工程项目中天天要做的决策。

我之前在一个图像处理项目里踩过一个经典坑：创建了 16 个 QThread 来并行处理图片，结果因为锁竞争太严重，16 个线程跑得比单线程还慢。后来切换到 QThreadPool + QtConcurrent，让线程池根据 CPU 核心数自动管理并发度，性能反而提升了 3 倍。线程数量和性能之间不是线性关系——到了一定程度，线程越多越慢。这篇我们把这些知识彻底讲透。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QThreadPool 和 QtConcurrent 属于 QtCore 模块（Qt 6 中 QtConcurrent 已并入 Core），不需要额外链接。所有示例可用控制台程序验证。

## 3. 核心概念讲解

### 3.1 QThreadPool——比手动管理线程更聪明的选择

QThreadPool 维护一个线程池，根据 CPU 核心数自动管理并发线程数量。你只需要把任务（QRunnable）提交给线程池，它会在有空闲线程时执行任务。QRunnable 的 setAutoDelete(true) 让线程池在任务执行完后自动删除任务对象。

### 3.2 QtConcurrent——更高层的并行抽象

QtConcurrent 提供了高级 API，让你不需要手动管理 QRunnable 就能实现并行计算。最常用的是 `QtConcurrent::map`、`QtConcurrent::filter` 和 `QtConcurrent::run`。

```cpp
QList<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

QFuture<void> future = QtConcurrent::map(numbers, [](int& n) {
    n = n * n;
});

future.waitForFinished();
```

`QtConcurrent::run` 可以在后台线程执行任意函数，返回 QFuture 对象用于获取结果。Qt 6 中 QFuture 支持 `.then()` 链式调用，实现非阻塞的异步结果处理。

### 3.3 QReadWriteLock——读多写少场景的最优解

QReadWriteLock 提供了比 QMutex 更好的并发性：多个线程可以同时持有读锁，但写锁是独占的。在读多写少的场景下（配置数据、缓存数据），性能优势明显。

```cpp
QReadWriteLock lock;

// 读者：允许多个线程同时读
void readConfig()
{
    QReadLocker locker(&lock);
    // 安全读取...
}

// 写者：独占访问
void updateConfig()
{
    QWriteLocker locker(&lock);
    // 安全修改...
}
```

现在有一道调试题。下面这段代码有什么问题？

```cpp
QReadWriteLock lock;
QList<int> data;

// 线程 2：用读锁但实际上在写数据
{
    QReadLocker locker(&lock);
    data[0] = 42;
}
```

问题在于线程 2 用了读锁但实际上在写数据——写操作必须用 QWriteLocker。后果是数据竞争。

## 4. 踩坑预防

第一个坑是 QThreadPool 任务队列无限增长。start() 会把任务放入队列，如果生产速度远快于消费速度，内存持续上升。解决方案是用 `tryStart()` 代替 `start()`——tryStart 只在有空闲线程时才接受任务。

第二个坑是 QtConcurrent::map 修改共享状态。map 的 Lambda 在多线程并行执行，如果 Lambda 内部访问了共享数据而没有加锁，就是数据竞争。解决方案是确保 Lambda 是纯函数。

第三个坑是 QFuture::result() 在 GUI 线程中阻塞等待。这会冻结 UI。解决方案是使用 `.then()` 链式调用或 QFutureWatcher 的信号机制。

## 5. 练习项目

练习项目：并行图片滤镜处理器。用 QtConcurrent 并行处理目录中的图片。

具体要求是：扫描目录中所有图片文件，使用 QtConcurrent::map 并行应用灰度滤镜，保存到输出目录。支持进度追踪和并发度调节。完成标准是处理 100 张图片比单线程快至少 2 倍。

提示几个关键点：QDir::entryInfoList 扫描文件，QImage 加载处理，QFutureWatcher 追踪进度。

## 6. 官方文档参考链接

[Qt 文档 · QThreadPool](https://doc.qt.io/qt-6/qthreadpool.html) -- 线程池类参考

[Qt 文档 · QtConcurrent](https://doc.qt.io/qt-6/qtconcurrent.html) -- 并发计算高级 API

[Qt 文档 · QFuture](https://doc.qt.io/qt-6/qfuture.html) -- 异步结果容器

[Qt 文档 · QReadWriteLock](https://doc.qt.io/qt-6/qreadwritelock.html) -- 读写锁参考

---

到这里，多线程的进阶知识就拆完了。线程池调度、QtConcurrent 高级抽象、QFuture 异步链式调用、读写锁场景选择——掌握这些就能做出正确的并发设计决策。下一篇我们来看 QProcess 进阶。
