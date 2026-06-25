---
title: "Step 2：强度算法 + 色块染色（核心）"
description: "实现 computeStrength 纯静态函数（长度 + 字符种类数），下方加 3 个 QLabel 色块用 setStyleSheet 染色，onTextChanged 只在档位真变时刷新。"
---

# Step 2：强度算法 + 色块染色（核心）

← [Step 1](./01-compose-and-toggle.md) · [手册首页](./index.md) · 下一步 [Step 3 信号透传 + setter 去重](./03-signals-and-properties.md) →

这一步是整个控件的核心——给 PasswordEdit 加上实时强度反馈。诀窍不在染色本身（那是 stylesheet 套路），而在**把强度算法做成纯静态函数独立可测**，外加**只在档位真变时才刷新色块**这个去重——打字过程中强度档没变就不重复染色、不发信号，避免无意义的刷新风暴。

## Step 2：computeStrength + 3 色块 + onTextChanged 去重

### 目标

实现四个东西：

1. **`computeStrength(const QString&)` 纯静态函数**：统计字符种类（小写/大写/数字/符号各算一类），按「长度 < 6 或种类数 <= 1 → kWeak；种类数 == 2 → kMedium；种类数 >= 3 且长度 >= 8 → kStrong，否则 kMedium」出档。空文本安全归 kWeak
2. **3 个 QLabel 色块**：在编辑行下方加一行 QHBoxLayout，塞 3 个 `QLabel`，固定高度 6px，靠左排列右侧弹性留白
3. **`updateStrengthIndicator()` 染色**：按 `strength_` 给 3 块染色——亮色随档变（弱红/中黄/强绿）、未亮全灰。用 `setStyleSheet`，不自绘
4. **`onTextChanged` 去重**：连 `QLineEdit::textChanged` 到私有 `onTextChanged`，里面调 `computeStrength`，**只有新档 != `strength_` 才更新成员 + 刷新色块 + emit `strengthChanged`**

### 提示（按顺序）

1. **computeStrength 做成静态纯函数**：`static Strength computeStrength(const QString& text)`，不碰任何成员（`include/password_edit.h:70`）。好处是能独立写单元测试、不依赖控件实例。空文本入口直接 `return Strength::kWeak` 显式兜底（`src/password_edit.cpp:111`），不依赖后续逻辑碰巧覆盖
2. **种类统计四个布尔**：`bool has_lower, has_upper, has_digit, has_symbol`，遍历 `for (const QChar& ch : text)` 用 `ch.isLower()` / `isUpper()` / `isDigit()` 判，其余一律算符号（`src/password_edit.cpp:118`）。最后 `classes = (has_lower?1:0) + ...` 加起来
3. **档位判定顺序**：先 `length < 6 || classes <= 1 → kWeak`，再 `classes == 2 → kMedium`，最后 `classes >= 3 && length >= 8 → kStrong`，兜底 return kMedium（`src/password_edit.cpp:133`）。注意「种类 >= 3 但长度 < 8」落 kMedium，不是 kStrong——长度也是强密码的必要条件
4. **色块样式四套常量**：`static constexpr const char* kStyleOff / kStyleWeak / kStyleMedium / kStyleStrong`，每套是 `"QLabel { background-color: #xxx; border-radius: 2px; }"`（`src/password_edit.cpp:18`）。灰 `#c0c0c0` / 红 `#e53935` / 黄 `#fdd835` / 绿 `#43a047`。固定字号色块，保持组合控件本色
5. **updateStrengthIndicator 映射亮几块**：`int lit = static_cast<int>(strength_) + 1`（kWeak=1、kMedium=2、kStrong=3），先 switch 选定「亮色」样式串，再 `for (i=0..2) strength_labels_[i]->setStyleSheet(i < lit ? on_style : kStyleOff)`（`src/password_edit.cpp:160`）。前 `lit` 块亮、其余灰
6. **onTextChanged 去重**：`Strength new_strength = computeStrength(text); if (new_strength != strength_) { strength_ = new_strength; updateStrengthIndicator(); emit strengthChanged(strength_); }`（`src/password_edit.cpp:151`）。**只有档变了才动**——打字过程中档没变（比如从 5 个小写加到 6 个小写，还是 kWeak）就不刷新、不发信号
7. **构造末尾初始化色块**：成员 `strength_{Strength::kWeak}` 给初值，构造末尾调一次 `updateStrengthIndicator()` 把初始 3 块全染灰（`src/password_edit.cpp:67`）。别让首屏色块状态不确定

### 关键认知——为什么 computeStrength 做静态、为什么 onTextChanged 要去重

**静态纯函数**：强度算法是纯逻辑（输入字符串、输出档位），不碰任何控件状态。把它做成 `static`、不访问成员，意味着你能脱离 Qt 控件环境直接写单测——`computeStrength("abc")` 该是 kWeak、`computeStrength("Abc12345")` 该是 kStrong，断言一跑就知道算法对不对。如果它访问了 `edit_->text()` 之类的成员，就非得构造整个控件才能测，测试成本暴涨。这也是 editable-table 把列类型转换做成独立函数的同款思路。

**onTextChanged 去重**：用户每打一个字符 `QLineEdit::textChanged` 就发一次。如果你的 `onTextChanged` 每次都无条件 `updateStrengthIndicator()` + `emit strengthChanged()`，那么从「5 个小写」打到「8 个小写」这全程强度档一直是 kWeak，却会触发 3 次无意义的色块重染 + 3 条无意义的 `strengthChanged` 信号——色块刷新虽然便宜，但信号会让外部监听者做无谓响应。去重的成本就一个 `!=` 比较，收益是档位稳定期间零刷新、零信号。这是「派生状态只在源状态真变时才传播」的经典套路。

### 检查点

- 输入 `abc`（3 位纯小写）：色块亮红 1 块、灰 2 块 = kWeak 对了
- 输入 `abcdef`（6 位纯小写，种类 1）：还是红 1 块 = 「种类 <= 1 → kWeak」对了（长度够但种类单一）
- 输入 `abcdef12`（小写 + 数字，种类 2，长度 8）：亮黄 2 块 = kMedium 对了
- 输入 `Abcdef12!`（小写 + 大写 + 数字 + 符号，种类 4，长度 9）：亮绿 3 块 = kStrong 对了
- 输入 `Ab1!`（种类 4 但长度 4 < 8）：亮黄 2 块 = 「种类 >= 3 但长度 < 8 → kMedium」对了
- 清空输入框：3 块全灰 = 空文本归 kWeak 对了
- 从 `abc` 打到 `abcdef`（全程 kWeak）过程中：色块不闪烁、不接到重复 strengthChanged = 去重对了

> stylesheet 染色不熟？[Qt Style Sheets](https://doc.qt.io/qt-6/stylesheet.html)。字符判定？[QChar](https://doc.qt.io/qt-6/qchar.html)。

### 对照答案

- computeStrength 纯静态声明：`include/password_edit.h:70`
- computeStrength 实现（空文本兜底 + 种类统计 + 档位判定）：`src/password_edit.cpp:109`
- 四套色块样式常量：`src/password_edit.cpp:18`
- 构造建 3 个 QLabel 色块 + 布局：`src/password_edit.cpp:43`
- updateStrengthIndicator 染色：`src/password_edit.cpp:160`
- onTextChanged 去重 + emit：`src/password_edit.cpp:151`
- 构造末尾初始化染色：`src/password_edit.cpp:67`

---

下一步：[Step 3 信号透传 + setter 去重 + Q_PROPERTY](./03-signals-and-properties.md)。
