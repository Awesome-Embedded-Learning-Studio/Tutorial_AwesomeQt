# 基建（填缝：P0/P0.5/P1/P4 + widget 化简 + 地基债）

> 单点细节批量处理：扫出清单 → 逐个修。

## widget/ 构建化简（主活·决策已定·未落盘）

把 widget/ 从「发布库式」瘦身为「教学最小形态」。9 步（A4 须先于 A3，其余可调序）：
1. **A4** 移 demo 子目录+重命名：`StatusLEDWindow.{h,cpp}`→`demo/status_led_window.{h,cpp}`、`main.cpp`→`demo/main.cpp`，改 include（类名不动）
2. **A5** `src/status_led.cpp` 补 `#include <algorithm>`（真隐患·换发行版编不过）
3. **A3** `statusled/CMakeLists.txt` 换 STATIC 库+demo+子目录自洽 AUTOMOC
4. **A2** 根 `widget/CMakeLists.txt` 换 root-owns-config（砍 MERGE_MODE/install/cache 变量）
5. **A1** 删 `cmake/AwesomeQtWidgets.cmake`(9.5k)+空 cmake/ 目录
6. **B1** `.claude/example_style.md:110` 成员命名改尾下划线（`m_timer`→`led_size_`，代码零改动只改文档）
7. **B2** 文件头统一 `@file`/`@brief`（禁 @author/@version/@date）
8. **C1** 新增 `widget/README.md`（无「专门样本」提法）
9. `.gitignore` 上移 widget/ 根 + 清旧产物 → **E1** `cd widget && cmake -B build && cmake --build build` 一次成功（/tmp 两轮实测过）

边界：不做 SHARED/install/MERGE_MODE/双.so；不迁移 cmake 模块为专门样本；不改局部变量（camelCase 维持）。

## P0 清债
- [ ] 死链校验（全库）
- [ ] 「需要注意的是」全库替换
- [ ] 专家 sidebar 收敛防扑空
- [ ] 入门结构漂移修正

## P0.5 全库扫描批（结构漂移/死链/风格泄漏/重复段/单点bug → 清单后逐个修）
- [ ] `examples/CMakePresets.json` + `cmake/Qt6Defaults.cmake` 共享 + 空 app//model/ 骨架
- [ ] `qt_src/` 进 `.gitignore` + `scripts/fetch_qt_source.sh` + CLAUDE.md 说明

## P1 加固
- [ ] 三层交叉链接 + lint
- [ ] H2 措辞统一
- [ ] C++17·23 统一
- [ ] multithreading 踩坑章重构
- [ ] QML6.2 进阶重写

## P4 时效
- [ ] QtCharts 弃用横幅（迁 QtGraphs）
- [ ] QtGraphs
- [ ] QML AOT
- [ ] ⚠QProperty·QBindable 专家章（待决策：并入 102→103，或单列。见 [expert.md](expert.md) 待决策）

## 地基债
- [ ] 建 `tutorial/expert/00-environment-setup/` 空目录占位（实测缺失）
- [ ] `examples/expert/` 根 CMakeLists.txt（实测缺失）
- 今天可建空占位，内容随「开 00 环境篇 / 产 example」补
