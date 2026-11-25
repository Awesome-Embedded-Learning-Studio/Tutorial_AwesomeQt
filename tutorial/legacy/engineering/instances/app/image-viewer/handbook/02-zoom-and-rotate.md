---
title: "Step 2-3：QTransform 缩放与旋转"
description: "加 zoom_/rotation_ 成员，paintEvent 用一个 QTransform 合成「平移到中心→缩放→旋转→平移到画布中心」，再接 zoomToFit/滚轮/状态栏。"
---

# Step 2-3：QTransform 缩放与旋转

← [手册首页](./index.md) · 上一步 [Step 1 显示](./01-load-and-display.md) · 下一步 [Step 4-5 翻页与幻灯片](./03-navigate-and-slideshow.md) →

## Step 2：QTransform 加缩放 + 旋转

### 目标

滚轮 / 按钮 / 菜单能缩放，能左旋右旋 90°，**绕图像中心**变换，不飞出画布。

### 提示

- 加成员 `double zoom_ = 1.0`、`int rotation_ = 0`（归一化到 0/90/180/270）
- `setZoom(factor)` 里 `std::clamp` 到合理范围（如 0.02~30），变了才 `update()`
- `rotateBy(deg)`：`(rotation_ + deg) % 360`，负数 `+360` 兜底
- paintEvent 里用一个 `QTransform t`：先 `translate(画布中心)` → `rotate` → `scale` → `translate(−图像中心)`，`p.setTransform(t)` 后 `drawImage(0,0,image)`
- **关键坑**：QTransform 右乘累积，**书写顺序与作用顺序相反**——最后一行 translate 先作用。顺序错了图像绕错中心 / 飞出去
- 画布尺寸要随变换变：`sizeHint()` 返回变换后图像外接尺寸，**旋转 90/270 时宽高互换**（`transpose`），否则 QScrollArea 滚动范围错

### 检查点

滚轮放大缩小、左旋右旋，**图像始终绕中心变换**、不出画布；旋转 90° 后画布宽高互换、滚动条范围对 = 变换对了。

> 坐标变换 / QTransform 不熟？[坐标变换基础](../../../../../beginner/02-qtgui/02-coordinate-transform-beginner.md)。

### 对照答案

- QTransform 四行顺序：`demo/image_view.cpp:133-137`
- setZoom clamp：`demo/image_view.cpp:48-49`
- rotateBy 归一化：`demo/image_view.cpp:65-77`
- transformedSize 旋转转置：`demo/image_view.cpp:90-100`

## Step 3：zoomToFit + 滚轮缩放 + 状态栏

### 目标

打开图自动 Fit 到窗口；滚轮缩放；状态栏显示「文件名 · 尺寸 · 缩放% · 旋转°」。

### 提示

- `zoomToFit(area)`：算 `area / 图像基准尺寸` 的 sx、sy，**取 min**（保证整张可见）；旋转后基准尺寸要 transposed
- `wheelEvent`：`angleDelta().y() / 120.0` 算档数，每档 ×1.1，`setZoom` 后 `event->accept()`
- 主窗口连画布的 `transformChanged(zoom, rotation)` 信号 → 刷状态栏；**同一信号里把画布 `resize(sizeHint())`**，QScrollArea 才更新滚动范围
- 首屏 Fit：`loadFile` 成功后 `QTimer::singleShot(0, ...)` 延一帧再 `zoomToFit(viewport()->size())`，等布局稳定

### 检查点

打开大图自动缩到整张可见；滚轮顺滑缩放；状态栏百分比实时变 = Fit + 滚轮 + 状态栏通了。

> 事件处理（wheelEvent）看 [事件处理基础](../../../../../beginner/03-qtwidgets/02-event-handling-beginner.md)。

### 对照答案

- zoomToFit 取 min：`demo/image_view.cpp:79-88`
- wheelEvent 滚轮：`demo/image_view.cpp:146-155`
- transformChanged → resize + 状态栏：`demo/image_viewer_window.cpp:158-163`

---

下一步：[Step 4-5 同目录翻页 + 幻灯片](./03-navigate-and-slideshow.md)。
