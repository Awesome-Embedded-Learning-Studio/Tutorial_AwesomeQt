---
title: "5.12 状态机进阶：层次状态机、历史状态、并行状态"
description: "入门篇我们把 QStateMachine 的基本状态转移跑通了——创建状态、添加转移、启动机器。写个简单的开关控制确实够用了。但 Qt 状态机框架的真正威力在于三个高级特性：层次状态机（状态可以嵌套）、历史状态（记住退出时的子状态）、并行状态（多个状态组同时活跃）。"
---

# 现代Qt开发教程（进阶篇）5.12——状态机进阶：层次状态机、历史状态、并行状态

## 1. 前言

入门篇我们把 QStateMachine 的基本状态转移跑通了——创建状态、添加转移、启动机器。写个简单的开关控制确实够用了。但 Qt 状态机框架的真正威力在于三个高级特性：层次状态机（状态可以嵌套）、历史状态（记住退出时的子状态）、并行状态（多个状态组同时活跃）。

这篇我们把这三个高级特性全部拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::StateMachine 模块（Qt 6 中从 QtCore 独立出来），CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS StateMachine)` 引入。

## 3. 核心概念讲解

### 3.1 层次状态机——状态嵌套与事件传播

QState 支持设置子状态。当一个状态有子状态时，进入父状态意味着进入初始子状态。子状态之间的转移不会离开父状态的边界——外部观察者只看到「系统在父状态中」。

```cpp
QStateMachine machine;

// 操作模式（顶层状态）
QState *operational = new QState();
machine.addState(operational);

// 操作模式下有两个子状态：自动和手动
QState *autoMode = new QState(operational);
QState *manualMode = new QState(operational);
operational->setInitialState(autoMode);

// 自动模式下又有子状态
QState *autoRunning = new QState(autoMode);
QState *autoPaused = new QState(autoMode);
autoMode->setInitialState(autoRunning);

// 子状态间的转移
autoRunning->addTransition(pauseSignal, autoPaused);
autoPaused->addTransition(resumeSignal, autoRunning);
autoMode->addTransition(switchToManual, manualMode);
manualMode->addTransition(switchToAuto, autoMode);

// 错误状态（顶层）
QState *errorState = new QState();
machine.addState(errorState);

// 从 operational 的任意子状态都能转移到 error
operational->addTransition(errorSignal, errorState);
errorState->addTransition(resetSignal, operational);

machine.setInitialState(operational);
machine.start();
```

层次结构的优势是事件传播。如果一个转移的目标是父状态（`operational`），状态机会先检查子状态有没有匹配的转移。如果子状态能处理就由子状态处理，否则传播到父状态。这让你可以在父状态上定义「对所有子状态都有效的通用转移」——比如上面的 `errorSignal` 转移，无论是 `autoRunning`、`autoPaused` 还是 `manualMode` 都能响应。

### 3.2 历史状态——记住退出时的位置

历史状态是一个特殊的状态占位符——当父状态被重新进入时，状态机自动恢复到上次退出时的子状态，而不是重新进入初始子状态。

```cpp
QState *operational = new QState();

// 子状态
QState *idle = new QState(operational);
QState *processing = new QState(operational);
QState *waiting = new QState(operational);
operational->setInitialState(idle);

// 创建历史状态
QHistoryState *history = new QHistoryState(operational);
// 默认历史类型：浅历史（只记住直接子状态）
// 深历史（DeepHistory）会记住整个嵌套层级

// 从其他状态重新进入 operational 时，跳到历史状态
QState *maintenance = new QState();
maintenance->addTransition(doneSignal, history);  // 回到上次的位置
```

典型应用场景：设备在运行中进入了维护模式（比如清洁、校准），维护完成后通过历史状态自动恢复到维护前的运行状态，而不是回到默认的空闲状态。

### 3.3 并行状态——多个独立的状态组同时运行

当 `QState` 的子状态模式设置为 `ParallelStates` 时，进入这个状态会同时进入它的所有子状态组。每个子状态组独立运行，互不干扰。

```cpp
QState *parallel = new QState();
parallel->setChildMode(QState::ParallelStates);

// 温度控制组
QState *tempGroup = new QState(parallel);
QState *tempNormal = new QState(tempGroup);
QState *tempHigh = new QState(tempGroup);
tempGroup->setInitialState(tempNormal);

// 压力控制组
QState *pressureGroup = new QState(parallel);
QState *pressureNormal = new QState(pressureGroup);
QState *pressureHigh = new QState(pressureGroup);
pressureGroup->setInitialState(pressureNormal);
```

进入 `parallel` 后，`tempNormal` 和 `pressureNormal` 同时活跃。温度和压力可以独立地触发各自的转移，不需要协调。并行状态的退出条件是所有子状态组都到达了最终状态（或者父状态上的转移被触发）。

现在有一道思考题。你的层次状态机中，子状态 `autoRunning` 上有一条 `stopSignal → idle` 的转移，父状态 `operational` 上也有一条 `stopSignal → maintenance` 的转移。当 `stopSignal` 发射时，哪条转移会生效？

子状态的转移优先。状态机的查找顺序是：从最深层活跃状态开始向上查找，找到第一条匹配的转移就执行。所以 `autoRunning` 的 `stopSignal → idle` 会先被匹配，父状态的 `stopSignal → maintenance` 不会被触发。如果你希望父状态的转移覆盖子状态，需要在子状态上不定义同名信号的转移。

## 4. 踩坑预防

第一个坑是并行状态中的信号冲突。并行状态的多组子状态可能同时响应同一个信号。比如 `tempHigh` 和 `pressureHigh` 都响应 `shutdownSignal`——两个转移会同时执行，可能导致意料之外的状态组合。确保并行状态组使用的信号是互不干扰的。

第二个坑是历史状态与并行状态的组合。`QHistoryState` 默认使用浅历史（只记住直接子状态），如果并行状态组内部有嵌套，浅历史不会恢复嵌套层级。需要使用 `QHistoryState::setHistoryType(QHistoryState::DeepHistory)` 来恢复完整的嵌套状态。

## 5. 练习项目

练习项目：工业洗衣机控制器。用层次状态机实现：顶层有「待机」「运行」「维护」三个状态。「运行」有并行子状态组：滚筒控制（停止/低速/高速）+ 水温控制（冷水/温水/热水）+ 门锁状态（锁定/解锁）。维护模式下使用历史状态，维护完成后恢复到之前的运行状态。

## 6. 官方文档参考链接

[Qt 文档 · QStateMachine](https://doc.qt.io/qt-6/qstatemachine.html) -- 状态机核心类

[Qt 文档 · QState](https://doc.qt.io/qt-6/qstate.html) -- 状态类，包含子状态和历史状态配置

[Qt 文档 · QHistoryState](https://doc.qt.io/qt-6/qhistorystate.html) -- 历史状态，支持浅/深历史恢复

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。层次状态、历史状态、并行状态——这三个高级特性让你能用状态机建模任何复杂的系统行为。
