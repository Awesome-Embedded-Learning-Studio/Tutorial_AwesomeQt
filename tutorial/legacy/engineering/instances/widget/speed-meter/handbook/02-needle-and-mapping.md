---
title: "Step 2：指针 + value→角度映射 + 数字读数（静态指值）"
description: "实现 angleForValue 把 value 映射成屏幕角 β，用 save/translate/rotate/restore + QPolygonF 画根粗尖细指针（rotate(β) 直接转，不用修正），补底部数字读数。这步指针静态指值，动画下一步加。"
---

# Step 2：指针 + value→角度映射 + 数字读数（静态指值）

← [Step 1](./01-static-gauge.md) · [手册首页](./index.md) · 下一步 [Step 3 平滑动画](./03-smooth-animation.md) →

## 目标

加一个 `setValue(int)`，针能静态指到那个值；底部居中绘出当前数值。**这步指针不旋转**（直接 set 角度重绘），动画留到 step 3——先把「value 怎么变成角度、指针怎么画」两件事做对。

## 提示（按顺序）

**1. 实现 `angleForValue(int v)`**——value 到屏幕角 β 的映射：

- 夹值 `v` 到 `[0, maxValue]`，防越界
- `max = std::max(1, maxValue)` 防 maxValue≤0 除零
- 返回 `135 + (v / max) * 270`（和 step 1 主刻度的角度公式同源，保证针和刻度对齐）

为什么是 135 起？屏幕角 β 以 3 点钟为 0°、顺时针为正（和 `rotate`、`cos/sin` 在 y 朝下的屏幕坐标系完全一致）：v=0 要指左下 = 135°（7:30 方向），v=max 指右下 = 45°（= 135+270 取模 360，4:30 方向），mid 指顶部 = 270°（12:00）。

**2. 加成员 `qreal needle_angle_`** 存当前指针角度（这步直接 `needle_angle_ = angleForValue(value_)` 赋值，下一步动画才每帧改它）。`setValue` 里改 `value_`、更新 `needle_angle_`、`update()`——**这步先做突变**。

**3. 画指针**——这是这步的核心。用坐标变换比手算两端 drawLine 省事：

- `p.save(); p.translate(center); p.rotate(angle); ... p.restore();`
- 因为 β 本身就是 rotate 用的那个屏幕角（顺时针为正），**直接 `p.rotate(needle_angle_)` 就对**，不用加减任何修正
- 验证：value=0 → rotate 135° 指左下；value=max → rotate 45° 指右下——和刻度对齐

**4. 指针用 QPolygonF 画根粗尖细**：在 rotate 后的坐标系里画多边形（水平指向 +x），根部半宽大、尖部半宽小、尾端往中心后方伸一点。`drawPolygon` + `NoPen` + 指针色 brush。画完 restore。

**5. 中心轴帽**：一个小实心圆盖在指针根部（`drawEllipse(center, cap_r, cap_r)`），用 `needleColor().darker(130)` 加深一点有层次。

**6. 底部数字读数**：当前 value 居中绘在开口下方（`center.y() + gauge_r * 0.55` 附近）。字号 `side * 0.08`（至少 8pt），加粗，用 `QFontMetrics` 水平居中。

## 关键认知——为什么 rotate(β) 不用修正

`painter.rotate` 的角度是「3 点钟为 0°、顺时针为正」——这正好和 step 1 定的屏幕角 β 同一套。所以指针只要 `rotate(needle_angle_)`，β 是几就转几度，针尖（默认指 +x = 3 点钟）就指到 β 方向，和刻度天然对齐。

这套「不用修正」是用坑换来的：早期版本给 `needle_angle_` 用了和 `drawArc` 同一套的「0°=3 点、逆时针为正」约定，结果 cos/sin 在屏幕 y 朝下时和 drawArc 的 y 朝上差一个翻转，刻度和指针被上下镜像——value=0 的针怼到了 value=max 那头。统一到屏幕角 β（顺时针为正）之后这事就消失了。踩坑细节见成品导览的踩坑②。

## 检查点

- `setValue(0)` 针指左下（135° 那端），`setValue(220)` 针指右下（45° 那端），中间值对应指在弧上正确位置
- 指针根粗尖细、根部有实心小圆帽
- 底部数字和 setValue 的值一致、居中、随控件缩放
- 针的延长线穿过中心（不歪）= rotate 角度对了

对了进下一步。如果指针指反端（value=0 指到了 max 那头）或上下镜像，翻 [troubleshooting](./troubleshooting.md) 的「指针指反端 / 上下镜像」。

> QPainter 坐标变换不熟？读 [坐标变换](../../../../../beginner/02-qtgui/02-coordinate-transform-beginner.md)、[QFontMetrics 文本测量](../../../../../beginner/02-qtgui/04-font-text-rendering-beginner.md)。

## 对照答案

- `angleForValue` 映射（135° 起、270° 扫、除零保护）：`src/speed_meter.cpp:64-68`
- setValue（这步的突变版可对照成品的逻辑骨架，动画部分 step 3 才加）：`src/speed_meter.cpp:74-86`
- 指针坐标变换（save/translate/rotate/restore，rotate(β) 直接转）：`src/speed_meter.cpp:266-288`
- 根粗尖细多边形顶点：`src/speed_meter.cpp:278-283`
- 中心轴帽：`src/speed_meter.cpp:290-296`
- 底部数字读数：`src/speed_meter.cpp:298-311`

---

下一步：[Step 3 给指针加平滑旋转动画（value/needleAngle 解耦）](./03-smooth-animation.md)——这是整套控件的核心，让指针转起来还不崩。
