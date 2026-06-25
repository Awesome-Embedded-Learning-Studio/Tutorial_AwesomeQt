---
title: "Step 3：地址拼装回填 + 边界夹值 + 信号抑制"
description: "IpEdit 实现 text()（空段补 0 拼 a.b.c.d）、setText（SkipEmptyParts 拆分 + std::clamp 夹越界 + 缺段补 0 + QSignalBlocker 抑制）、isValid（全 0 判 false）、clear。"
---

# Step 3：地址拼装回填 + 边界夹值 + 信号抑制

← [手册首页](./index.md) · 上一步 [Step 2 跨段焦点流转](./02-focus-flow.md) →

前两步把骨架和交互做完了，这一步补上地址的双向 API——`text()` 取出来、`setText()` 塞进去、`isValid()` 判合法、`clear()` 清空。难点不在拼装本身，而在「程序化回填时别发一堆假信号」和「边界值要老老实实夹」。

## Step 3：text() + setText() + isValid() + clear()

### 目标

实现四个公有方法。`text()` 遍历 4 段，每段 `toInt()` 成功取数值、失败补 0，`join('.')` 拼 `"a.b.c.d"`。`setText(const QString& ip)` 先判空串走 `clear()` 返回；否则 `ip.split('.', Qt::SkipEmptyParts)` 拆段，循环 4 段：`i < parts.size()` 取 `parts[i].toInt()`，失败补 0、`std::clamp(val,0,255)` 夹越界、`edit->setText(QString::number(val))`；`i >= parts.size()` 缺段补 `"0"`。**关键**：每段 `setText` 前包 `QSignalBlocker blocker(edit)`，循环结束后统一 `emit textChanged(text())` 一次。`isValid()` 4 段都 0-255、非空、纯数字，额外用 `any` 标志记录是否有非 0 段，全 0 返回 false。`clear()` 每段 `QSignalBlocker` 包住 `edit->clear()`，末尾发一次 `textChanged`。

### 提示

- **`text()` 兜底失败段**：`raw.toInt(&ok)` 失败（空串、非数字）就 append `"0"` 而不是空串——否则空控件读出来是 `"..."`。这和「空段补 0」的语义一致
- **`setText` 先判空串**：`if (ip.isEmpty()) { clear(); return; }`。不判的话 `split` 返回空 list，循环全走缺段补 0 分支，结果还是全 0——能跑但多走弯路，且语义上空串就该清空而非填 0
- **`SkipEmptyParts` 而非 `KeepEmptyParts`**：`"192..1.1"` 这种双点用 `SkipEmptyParts` 拆出 `["192","1","1"]`，第 4 段缺补 0。用 `KeepEmptyParts` 会拆出空串元素，`toInt` 失败补 0 也行但多一道弯
- **`std::clamp` 夹越界**：`val = std::clamp(val, 0, 255)`——`999` 夹成 `255`、负数夹成 `0`。别忘了 cpp 顶部 `#include <algorithm>`
- **`QSignalBlocker` 是本步核心**：每段 `edit->setText()` 都会触发子段 `textChanged`，step 1 那条转发 lambda 就会各 emit 一次整体 `textChanged`——4 段就 4 次假通知。`QSignalBlocker blocker(edit)` 在作用域内屏蔽该控件所有信号，出了作用域自动恢复（RAII）。循环里每段包一个，循环外统一发一次真通知
- **`isValid` 的 `any` 标志**：常规判「4 段非空 + 0-255 + 纯数字」外，加个 `bool any=false`，任一段 `val != 0` 就置 true，最后 `return any`。这样 `"0.0.0.0"` 返回 false（视为未填写）。严格语义场景就把 `any` 摘掉、直接 `return true`

### 关键认知——为什么信号抑制必须做

不做 `QSignalBlocker`，外部接到的 `textChanged` 是这样的：调一次 `setText("192.168.1.1")`，第 1 段 `setText("192")` 触发 → 整体重拼 emit `"0.168.1.1"`（此时只有第 1 段填了，其余还是旧值或空）→ 第 2 段填完又 emit `"192.0.1.1"` …… 中间这几个「半成品」地址全是脏数据，外部回显会闪几次。`QSignalBlocker` 把中间过程全部静音，只在 4 段都填完后发一次「最终态」——干净。`clear()` 同理。

### 检查点

跑起来用 demo 的预设按钮测边界：按 `setText("999.1.1.1")` 显示 `255.1.1.1`（越界夹 255）；按 `setText("1.2.3")` 显示 `1.2.3.0`（缺段补 0）；按 `setText("a.b.c")` 显示 `0.0.0.0`（非数字补 0）；按 `setText("")` 全清空。每次预设只触发一次外部回显更新（不闪多次）。`isValid` 在填 `192.168.1.1` 时合法、全 0 时非法、任一段空时非法。`clear()` 后 4 段全空、回显清一次。

> 信号阻塞不熟？[QSignalBlocker](https://doc.qt.io/qt-6/qsignalblocker.html)。字符串拆分？[QString::split](https://doc.qt.io/qt-6/qstring.html#split)。

### 对照答案

- `text()` 空段补 0 拼装：`src/ip_edit.cpp:76`
- `setText` 空串走 clear + SkipEmptyParts 拆分：`src/ip_edit.cpp:90`
- 每段 `QSignalBlocker` + `std::clamp` 夹值 + 缺段补 0：`src/ip_edit.cpp:104`
- 末尾统一 `emit textChanged`：`src/ip_edit.cpp:117`
- `isValid` 全 0 判 false（`any` 标志）：`src/ip_edit.cpp:123`
- `clear` 每段 `QSignalBlocker` 屏蔽：`src/ip_edit.cpp:149`

---

三步搓完，对照成品 `widget/ip-edit/` 应该功能一致。想再深一层（IPv6、粘贴分发、单段子类化）回 [手册首页](./index.md) 的「进阶挑战」。成品全貌看 [成品导览](../)。
