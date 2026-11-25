---
title: 卡住怎么办
description: CircleProgress 手搓常见错——弧画反、连点跳变、栈溢出、压极小崩——指向成品，不给完整答案
---

# 卡住怎么办

按症状找，只指方向。答案在成品 `widget/circle-progress/`。

## 弧逆时针铺开 / 从 3 点钟起

drawArc 角度体系和直觉相反：0°=3 点钟、正值逆时针、单位 1/16°。你大概把数学角度直接塞进去了。进度弧要「12 点钟起、顺时针」，所以起始角 `90*16`、扫角取**负**。对照 `src/circle_progress.cpp:22` 的约定注释和 `:191` 的 drawArc 调用。

## setValue 后弧突变，不平滑

progress 的 WRITE 回调 `setDisplayProgress` 里启动了动画，或者根本没接 QPropertyAnimation。setValue 才是启动动画的地方，setDisplayProgress 只管纯赋值+update。对照 `src/circle_progress.cpp:55`（setValue 启动画）和 `:77`（setDisplayProgress 纯赋值）。

## 连点 setValue 弧在旧目标和新目标间乱跳

动画的 setStartValue 用了 0 或旧 value，没用「当前显示进度」。连点时上次动画还在半路，新动画却从 0 起步，就跳。setStartValue 喂当前 `progress_`（可能是中间值）接力。对照 `src/circle_progress.cpp:55` 的三件套。

## 把 progress 的 WRITE 指向 setValue，一跑就栈溢出

动画驱动 setValue → setValue 又启动画 → 无限递归。progress 的 WRITE 必须是纯赋值的 setDisplayProgress，不能启动任何动画。对照 `include/circle_progress.h:30` 和 `src/circle_progress.cpp:77`。

## 控件拖到极小或线宽拉极大，弧消失 / 崩

半径 `min(w,h)/2 - stroke/2 - 2` 算成了负数或 0，drawArc 对负宽高矩形行为未定义。所有几何尺寸包 `std::max(1.0, ...)` 兜底。对照 `src/circle_progress.cpp:165`。

## 动画一卡一卡的

回调里用了 `repaint()`——它同步立即重绘、不等事件循环，动画自然掉帧。一律 `update()`（异步合并）。对照 `src/circle_progress.cpp` 的 setDisplayProgress。

## progress_anim_ 频繁 setValue 后崩 / 悬空

动画对象用了 `DeleteWhenStopped`，停了就被 delete，下次 stop() 持的是悬空指针。用持久成员指针 + parent=this 托管，反复 stop()/重配/start() 复用，不用 DeleteWhenStopped。对照 `src/circle_progress.cpp:47`。

---

实在卡死，成品 `widget/circle-progress/src/circle_progress.cpp` 就是答案——但先自己拼。
