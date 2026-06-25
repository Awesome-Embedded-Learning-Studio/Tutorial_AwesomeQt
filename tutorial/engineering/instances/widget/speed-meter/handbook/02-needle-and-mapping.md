---
title: "Step 2：指针 + value→角度映射 + 数字读数（静态指值）"
description: "实现 angleForValue 把 value 映射成指针角度，用 save/translate/rotate/restore + QPolygonF 画根粗尖细指针（注意 rotate +90° 修正），补底部数字读数。这步指针静态指值，动画下一步加。"
---

# Step 2：指针 + value→角度映射 + 数字读数（静态指值）

← [Step 1](./01-static-gauge.md) · [手册首页](./index.md) · 下一步 [Step 3 平滑动画](./03-smooth-animation.md) →

## 目标

加一个 `setValue(int)`，针能静态指到那个值；底部居中绘出当前数值。**这步指针不旋转**（直接 set 角度重绘），动画留到 step 3——先把「value 怎么变成角度、指针怎么画」两件事做对。

## 提示（按顺序）

**1. 实现 `angleForValue(int v)`**——value 到数学角度的映射：

- 夹值 `v` 到 `[0, maxValue]`，防越界
- `max = std::max(1, maxValue)` 防 maxValue≤0 除零
- 返回 `225 - (v / max) * 270`（和 step 1 主刻度的角度公式同源，保证针和刻度对齐）

**2. 加成员 `qreal needle_angle_`** 存当前指针角度（这步直接 `needle_angle_ = angleForValue(value_)` 赋值，下一步动画才每帧改它）。`setValue` 里改 `value_`、更新 `needle_angle_`、`update()`——**这步先做突变**。

**3. 画指针**——这是这步的核心。用坐标变换比手算两端 drawLine 省事，但有个坑：

- `p.save(); p.translate(center); p.rotate(angle); ... p.restore();`
- **坑**：`rotate` 的基准是 **12 点钟方向、顺时针为正**，而我们 `needle_angle_` 是数学角度（0°=3 点钟）。所以旋转角 = `needle_angle_ + 90`（把数学 0° 摆正到 3 点，再 +90° 适配 rotate 的 12 点基准）
- 验证：value=0(225°) → rotate 315° 指左下；value=max(-45°) → rotate 45° 指右下——对得上

**4. 指针用 QPolygonF 画根粗尖细**：在 rotate 后的坐标系里画多边形（水平指向 +x），根部半宽大、尖部半宽小、尾端往中心后方伸一点。`drawPolygon` + `NoPen` + 指针色 brush。画完 restore。

**5. 中心轴帽**：一个小实心圆盖在指针根部（`drawEllipse(center, cap_r, cap_r)`），用 `needleColor().darker(130)` 加深一点有层次。

**6. 底部数字读数**：当前 value 居中绘在开口下方（`center.y() + gauge_r * 0.55` 附近）。字号 `side * 0.08`（至少 8pt），加粗，用 `QFontMetrics` 水平居中。

## 关键认知——rotate 的 +90° 修正从哪来

`painter.rotate` 的 0° 指向 12 点钟、顺时针递增；我们的 `needle_angle_` 沿用的是 step 1 的数学角度（0°=3 点钟、逆时针为正，但因 Y 朝下视觉顺时针）。两套「0° 朝向」差 90°，所以 rotate 时必须 `needle_angle_ + 90`。如果你不想记这个换算，也可以改用三角函数手算指针两端点 `drawLine`——但那样画不出根粗尖细的多边形质感。这套 +90 修正成品里写在注释里，别删。

## 检查点

- `setValue(0)` 针指左下（225° 那端），`setValue(220)` 针指右下（-45° 那端），中间值对应指在弧上正确位置
- 指针根粗尖细、根部有实心小圆帽
- 底部数字和 setValue 的值一致、居中、随控件缩放
- 针的延长线穿过中心（不歪）= rotate 修正对了

对了进下一步。如果指针整体偏 90°、或指错方向，翻 [troubleshooting](./troubleshooting.md) 的「指针方向偏 90°」。

> QPainter 坐标变换不熟？读 [坐标变换](../../../../../beginner/02-qtgui/02-coordinate-transform-beginner.md)、[QFontMetrics 文本测量](../../../../../beginner/02-qtgui/04-font-text-rendering-beginner.md)。

## 对照答案

- `angleForValue` 映射（225° 起、270° 扫、除零保护）：`src/speed_meter.cpp:56-61`
- setValue（这步的突变版可对照成品的逻辑骨架，动画部分 step 3 才加）：`src/speed_meter.cpp:66-78`
- 指针坐标变换（save/translate/rotate/restore，注意 +90）：`src/speed_meter.cpp:252-274`
- 根粗尖细多边形顶点：`src/speed_meter.cpp:261-269`
- 中心轴帽：`src/speed_meter.cpp:276-282`
- 底部数字读数：`src/speed_meter.cpp:284-298`

---

下一步：[Step 3 给指针加平滑旋转动画（value/needleAngle 解耦）](./03-smooth-animation.md)——这是整套控件的核心，让指针转起来还不崩。
