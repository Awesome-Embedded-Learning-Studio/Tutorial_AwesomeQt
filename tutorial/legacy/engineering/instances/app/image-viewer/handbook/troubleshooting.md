---
title: "卡住怎么办"
description: "按症状查：图不显示、旋转中心错、缩放没滚动条、翻页游标乱、幻灯片退出窗口变小、moc 报错——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `app/05-image-tools/image-viewer/`，对照着看。

## 打开图但画布空白

- `loadImage` 返回值是不是 true？QImageReader 加载失败要查 `errorString()`。→ `demo/image_view.cpp:25-32`
- paintEvent 里**有没有真 `drawImage`**？`image_.isNull()` 时是不是直接 return 了？→ `demo/image_view.cpp:123-127`
- 进阶排查：[图像与像素图](../../../../../beginner/02-qtgui/03-image-pixmap-beginner.md)

## 缩放 / 旋转后图像飞出去或绕错中心

- QTransform 四行的**顺序对不对**？右乘累积，最后一行 translate（移图像中心到原点）要**先作用**。→ `demo/image_view.cpp:133-137`
- 进阶排查：[坐标变换基础](../../../../../beginner/02-qtgui/02-coordinate-transform-beginner.md)

## 缩放变大后没有滚动条 / 滚不动

- QScrollArea **widgetResizable 是不是 false**？true 会把画布拉伸到视口、永远不滚。→ `demo/image_viewer_window.cpp:143`
- 画布 sizeHint 是不是**变换后尺寸**？transformChanged 信号里有没有 `resize(sizeHint())`？没 resize 滚动范围不更新。→ `demo/image_viewer_window.cpp:160-162`
- 旋转后尺寸互换做了吗？→ `demo/image_view.cpp:95-97`

## 翻页时游标和显示对不上 / 跳图

- navigate 是不是**加载成功才提交 current_index_**？先推游标再加载、坏图失败会导致脱钩。→ `demo/image_viewer_window.cpp:231-245`
- 循环的 `idx = (current_index_ + offset*step) % n` 负数有没有 `+n` 兜底？

## 幻灯片退出后窗口变小（最大化丢了）

- 进幻灯片前**记了 was_maximized_ 吗**？退出时按它选 showMaximized / showNormal。→ `demo/image_viewer_window.cpp:299`、`demo/image_viewer_window.cpp:312-316`

## 幻灯片遇坏图卡住 / 弹框打断

- loadFailed 槽在幻灯片态**有没有静默**？全屏弹模态框会阻塞 timer。→ `demo/image_viewer_window.cpp:170-176`
- 进阶排查：[定时器基础](../../../../../beginner/01-qtbase/11-timer-beginner.md)

## moc 报错（Q_OBJECT / 信号不认）

- ImageView 头里**有没有 Q_OBJECT**？
- CMake **开了 AUTOMOC 吗**（app/CMakeLists.txt 的 `set(CMAKE_AUTOMOC ON)`）？
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
