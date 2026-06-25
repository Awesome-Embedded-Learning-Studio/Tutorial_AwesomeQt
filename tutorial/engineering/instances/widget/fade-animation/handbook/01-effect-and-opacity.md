---
title: "Step 1-2：挂 Effect + opacity 升 Q_PROPERTY"
description: "给控件挂 QGraphicsOpacityEffect，setOpacity 改透明度；再把 opacity 升级为 Q_PROPERTY，让它能被元系统/Designer/动画按名字驱动。"
---

# Step 1-2：挂 Effect + opacity 升 Q_PROPERTY

← [手册首页](./index.md) · 下一步 [Step 3-4 淡入淡出动画](./02-fade-animation.md) →

这两步先把「透明度」这件事搭起来：先让一个 effect 扛住它，再把 `opacity` 升级成 Qt 认识的属性。动画留到 step 3。

## Step 1：挂 QGraphicsOpacityEffect，setOpacity 改透明度

### 目标

控件出现后，调一个 `setOpacity()`，透明度立刻变（半透明、全透明都能做到）。**这步先做瞬时改值**（直接 set，不要动画），动画下一步。

### 提示

- 继承 `QWidget`，加私有成员 `QGraphicsOpacityEffect* effect_`
- 构造里 `effect_ = new QGraphicsOpacityEffect(this)`，`effect_->setOpacity(1.0)`（初值全不透明），再 `setGraphicsEffect(effect_)` 把它挂到自身
- 写 `void setOpacity(qreal value)`：先 clamp 到 [0,1]，再 `effect_->setOpacity(value)`。effect 自带 update，不用手动刷
- 写 `qreal opacity() const { return effect_->opacity(); }`
- 别重写 `paintEvent`——透明度全交给 effect，控件该画啥画啥

### 检查点

调 `setOpacity(0.3)` 控件变七成透明 = effect 挂对了。

> QGraphicsOpacityEffect 不熟？[QPropertyAnimation 属性动画](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md) 章节里有特效配合动画的示例。

### 对照答案

- effect new + setOpacity(1.0) + setGraphicsEffect：`src/fade_animation.cpp:16-18`
- setOpacity 纯赋值 + clamp：`src/fade_animation.cpp:70-81`

---

## Step 2：opacity 升级为 Q_PROPERTY

### 目标

给类加一行 `Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)`，补上 `opacityChanged` 信号。功能上和 step 1 一样，但 opacity 现在是「Qt 认识的属性」——能被元系统枚举、被 Designer 编辑、**被动画按名字驱动**（这是 step 3 的前提）。

### 提示

- Q_PROPERTY 三件：READ 一个 getter（step 1 写好了）、WRITE 一个 setter（step 1 写好了）、NOTIFY 一个信号
- setOpacity 末尾 `emit opacityChanged(value)`
- 顺手把 `fadeDuration` 也做成 Q_PROPERTY：`Q_PROPERTY(int fadeDuration READ fadeDuration WRITE setFadeDuration NOTIFY fadeDurationChanged)`，setFadeDuration 里 clamp 兜底（<1 改 1）+ emit
- **关键认知**：WRITE 指 setOpacity（纯赋值），**不是动画入口**。动画由 step 3 的 runFade 启动——别把动画逻辑塞进 setOpacity，否则外部滑块拖一下也会触发动画（见 [troubleshooting](./troubleshooting.md) 的「以为 setOpacity 会触发动画」）

### 检查点

编译过（moc 不报错）+ `metaObject()->propertyCount()` 能数到 opacity 和 fadeDuration = Q_PROPERTY 生效了。外部调 `setOpacity(0.5)` 是**瞬时变值、无动画**（动画还没写）。

> Q_PROPERTY 机制不熟？[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)、进阶 [属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。

### 对照答案

- Q_PROPERTY(opacity / fadeDuration) 声明：`include/fade_animation.h:32-33`
- opacityChanged / fadeDurationChanged 信号：`include/fade_animation.h:61-62`
- setFadeDuration clamp 兜底 + emit：`src/fade_animation.cpp:51-60`

---

下一步是重头戏：[Step 3-4 让 QPropertyAnimation 驱动 opacity 做淡入淡出](./02-fade-animation.md)。
