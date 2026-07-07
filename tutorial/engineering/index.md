---
title: "项目实战 · 实例库"
description: "实例库——拿来学结构、练手、做发现的教学成品（非可发布产品）。每个控件/应用是独立可编译 STATIC 库 + demo，配成品导览 + 手搓手册两套文档。"
---

# 项目实战 · 实例库

实例库——拿来学结构、练手、做发现的教学成品（非可发布产品）。每个控件 / 应用是独立可编译 STATIC 库 + demo，配成品导览（架构 / 决策 / 踩坑）+ 手搓手册（跟着搓）两套文档，闭合「学 → 做」。

## Widget 控件

<CardGrid>
<CardLink href="/engineering/instances/widget/status-led/" desc="状态指示灯：四态颜色平滑过渡 + 呼吸/明灭闪烁 + 完整 Q_PROPERTY">StatusLED</CardLink>
<CardLink href="/engineering/instances/widget/toggle-switch/" desc="滑动开关：点击/拖动切换 + 滑块滑动动画 + 轨道变色">ToggleSwitch</CardLink>
<CardLink href="/engineering/instances/widget/circle-progress/" desc="圆形进度环：value/progress 解耦 + 平滑过渡 + 中心百分比">CircleProgress</CardLink>
<CardLink href="/engineering/instances/widget/speed-meter/" desc="速度仪表盘：动画指针 + 主/次刻度 + 双角度体系自洽">SpeedMeter</CardLink>
<CardLink href="/engineering/instances/widget/range-slider/" desc="双柄范围滑块：拖拽选区间 + 键盘微调两侧">RangeSlider</CardLink>
<CardLink href="/engineering/instances/widget/line-chart/" desc="折线图：纯 QPainter 自绘 + Y 轴自动缩放 + 零外部依赖">LineChart</CardLink>
<CardLink href="/engineering/instances/widget/editable-table/" desc="可编辑表格：委托校验 + 列类型编辑器 + 数据往返">EditableTable</CardLink>
<CardLink href="/engineering/instances/widget/checkbox-tree/" desc="树形复选框：三态 + 父子联动 + 向上回算">CheckboxTree</CardLink>
<CardLink href="/engineering/instances/widget/checkbox-list/" desc="复选框列表：QListWidget 勾选 + 全选/级联 + 批量守卫">CheckboxList</CardLink>
<CardLink href="/engineering/instances/widget/log-viewer/" desc="滚动日志：级别染色 + 自动滚底 + 行数上限裁旧">LogViewer</CardLink>
<CardLink href="/engineering/instances/widget/password-edit/" desc="密码框：显隐切换 + 实时强度指示（弱/中/强）">PasswordEdit</CardLink>
<CardLink href="/engineering/instances/widget/ip-edit/" desc="IPv4 输入框：4 段跳焦 + 0-255 校验 + 点号吃掉">IpEdit</CardLink>
<CardLink href="/engineering/instances/widget/fade-animation/" desc="淡入淡出容器：OpacityEffect 承载 + 动画驱动">FadeAnimation</CardLink>
</CardGrid>

## App 整机成品

<CardGrid>
<CardLink href="/engineering/instances/app/image-viewer/" desc="图片查看器：缩放/旋转/翻页/幻灯片 + QTransform 一次合成">image-viewer</CardLink>
<CardLink href="/engineering/instances/app/json-editor/" desc="JSON 编辑器：格式化/校验 + 树形浏览 + 递归">json-editor</CardLink>
<CardLink href="/engineering/instances/app/sqlite-browser/" desc="SQLite 浏览器：表列表 + 可编辑表格 + 任意 SQL">sqlite-browser</CardLink>
<CardLink href="/engineering/instances/app/serial-tool/" desc="串口调试助手：配置/收发 + Hex/ASCII + 错误收敛">serial-tool</CardLink>
<CardLink href="/engineering/instances/app/network-tool/" desc="TCP/UDP 调试：Server/Client + 收发 + loopback 可验">network-tool</CardLink>
<CardLink href="/engineering/instances/app/tetris/" desc="俄罗斯方块：自绘 + 7 形态/旋转/消行/计分">tetris</CardLink>
<CardLink href="/engineering/instances/app/cpu-memory-monitor/" desc="CPU/内存监控：进度条 + 历史曲线 + 跨平台">cpu-memory-monitor</CardLink>
</CardGrid>

> model / industrial 栏骨架已立（undo-redo-framework / hmi-dashboard），文档随成品补。
