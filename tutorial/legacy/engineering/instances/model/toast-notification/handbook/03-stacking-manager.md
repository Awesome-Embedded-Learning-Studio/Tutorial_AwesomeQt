---
title: "Step 3：单例堆叠管理"
description: "ToastManager 单例维护活动队列，按屏幕角落堆叠定位新 Toast，对象自毁时 destroyed 信号触发重排。"
---

# Step 3：单例堆叠管理

← [手册首页](./index.md) · 上一步 [Step 2 淡入淡出 + 定时自毁](./02-fade-and-self-destruct.md) →

## 目标

连续弹出多条 Toast，它们**按屏幕角落逐条堆叠不重叠**（右下角默认，新条向上排）；**先入的先消失后，余下自动回填角落**不留空洞。用单例 `ToastManager` 统一管。

## 提示

### 单例

- `ToastManager` 继承 `QObject`，`static ToastManager* getInstance()` 惰性创建
- 单例对象挂在 `qApp`（QCoreApplication）上：`instance->setParent(qApp)`，随进程退出销毁。别用裸静态局部 `ToastManager instance;`——QObject 构造时 `qApp` 可能还没就绪
- 提供 `showToast(text, type, duration, corner, parent)` 作为唯一公开入口——**禁止**外部直接 `new Toast`（堆叠定位无人管）

### 屏幕可用区

- `QScreen::availableGeometry()` 给出排除任务栏的可用矩形，是堆叠定位的基准
- 优先取 `parent->windowHandle()->screen()`（parent 所在屏幕），取不到回退 `QGuiApplication::primaryScreen()`，都没有兜底一个 `QRect(0,0,1920,1080)`（无屏环境如 CI）

### 堆叠定位（placeToast）

- 维护 `QList<Toast*> toasts_` 活动队列，顺序 = 屏幕从外（先入）到内（后入）
- 新条入队后，先按角落算「不堆叠时」的基准锚点（屏幕角落内缩 `margin_`）
- 再遍历队列里**位于本 toast 之前的**条目，累计 `offset += prev->height() + spacing_`
- 按角落方向应用偏移：底部角落 `y -= offset`（向上堆叠），顶部角落 `y += offset`（向下堆叠）
- 最后 `toast->move(x, y)`

### 自毁重排（relayout）

- showToast 里挂 `connect(toast, &QObject::destroyed, this, [this, corner, toast]() { onToastDestroyed(toast, corner); })`
- `onToastDestroyed`：`toasts_.removeAll(toast)` 把指针移出，再 `relayout(corner, available)` 重排
- `relayout` 遍历**仍存活的**队列（被销毁的已移出），按 placeToast 同样的累计逻辑重算每条 y
- 注意：`destroyed` 信号发出时被销毁对象已在析构，**不能访问其成员**——所以先 `removeAll`（只比指针，安全），再遍历余下（都还活着）。lambda 里按值捕获 `toast` 指针即可，别 `static_cast` 一个正在析构的 `QObject*`

## 检查点

连发五条，**五条从角落向上依次排开不重叠** = 堆叠定位对了；等先入的淡出消失后，**后面的自动下移回填**不留空洞 = 重排对了。

> 对象生命周期 / 元对象系统不熟？读 [QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)；动画进阶读 [动画框架进阶](../../../../../advanced/03-qtwidgets/09-animation-advanced.md)。

## 对照答案

- 单例惰性创建 + 挂 qApp：`src/toast-notification.cpp:143-151`
- showToast 入队 + 定位 + 挂 destroyed：`src/toast-notification.cpp:155-186`
- placeToast 累计偏移定位：`src/toast-notification.cpp:188-240`
- onToastDestroyed 移出 + relayout：`src/toast-notification.cpp:243-253`、`src/toast-notification.cpp:255-280`

---

搓完了。成品能跑、能堆叠、能自毁。想再深一层（多角落分桶 / 手动关闭 / 动画曲线）回 [手册首页](./index.md) 看进阶挑战。
