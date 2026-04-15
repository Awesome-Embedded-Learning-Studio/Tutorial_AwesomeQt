---
id: "079"
title: "工业模板：汽车数字仪表盘"
category: code-library
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["008", "024"]
blocks: []
estimated_effort: epic
---

## 目标

创建汽车数字仪表盘项目模板，包含速度表、转速表、导航地图、媒体信息等，带流畅动画效果。

## 验收标准

- [ ] 速度表和转速表（带平滑动画）
- [ ] 燃油/水温/电池状态指示
- [ ] 导航信息面板
- [ ] 媒体播放信息
- [ ] 告警指示灯系统
- [ ] 流畅的 60fps 动画效果
- [ ] 日/夜间模式切换

## 实施说明

1. **仪表盘布局**：
   - 中央大速度表 + 两侧辅助信息区
   - 使用 QML 实现流畅动画（Qt Quick Scene Graph GPU 加速）
   - 也可提供 Qt Widget 版本（QPainter 实现）
2. **速度表/转速表**：
   - 弧形刻度盘 + 指针动画
   - 数字显示叠加
   - 红区警告
   - 使用 `SpringAnimation` 实现指针物理回弹效果
3. **指示灯系统**：
   - 发动机故障、ABS、气囊、机油压力等标准指示灯
   - SVG 图标 + 闪烁动画
   - 优先级排序（紧急告警置顶）
4. **导航面板**：
   - 模拟导航地图区域
   - 转向指示
   - 预计到达时间
5. **媒体信息**：
   - 歌曲名/艺术家/专辑
   - 播放控制
   - 专辑封面
6. **主题**：
   - 日间模式：白底深色指针
   - 夜间模式：黑底发光指针（模拟背光效果）
   - 平滑过渡动画
7. 使用 QML ShaderEffect 实现发光/模糊等视觉效果

## 涉及文件

- `examples/templates/automotive-dashboard/`（新建，完整项目目录）
- `examples/templates/automotive-dashboard/qml/`
- `examples/templates/automotive-dashboard/resources/`

## 参考资料

- Qt Quick: https://doc.qt.io/qt-6/qtquick-index.html
- QML Animation: https://doc.qt.io/qt-6/qtquick-statesanimations-animations.html
- Qt Shader Effect: https://doc.qt.io/qt-6/qml-qtquick-shadereffect.html
