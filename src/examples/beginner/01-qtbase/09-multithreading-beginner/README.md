# 多线程基础示例

## 编译运行

```bash
mkdir build && cd build
cmake ..
cmake --build .
./bin/multithreading_beginner
```

## 输出说明

如果 qDebug 没有输出，可以设置环境变量：

```bash
QT_LOGGING_RULES="*.debug=true" ./bin/multithreading_beginner
```

或使用 stderr 直接输出：

```bash
./bin/multithreading_beginner 2>&1
```

## 示例内容

1. QThread + moveToThread（推荐方式）
2. QThreadPool 线程池
3. QtConcurrent::run 简单异步任务
4. QMutex 线程安全
5. 跨线程通信
