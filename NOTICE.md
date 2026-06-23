# 许可声明 / Licensing

本仓库包含三类内容，分别适用不同许可。

## 1. 示例与实例库代码 —— MIT

`examples/`、`widget/`、`app/`、`model/`、`industrial/`、`scripts/` 下的源代码遵循
[MIT License](LICENSE)（仓库根 LICENSE 文件）。可自由使用、修改、分发，附带版权声明即可。

## 2. 教程文档 —— CC BY-SA 4.0

`tutorial/` 下的教程正文遵循
[Creative Commons Attribution-ShareAlike 4.0](https://creativecommons.org/licenses/by-sa/4.0/deed.zh-hans)：

- ✅ 可分享、改编（含商用）
- ⚠️ 须署名（注明出处），衍生作品须以相同许可发布（ShareAlike）

> 如希望更宽松（仅署名、不要求相同许可），可改用 CC BY 4.0；如希望全仓统一 MIT，
> 也可去掉本节。CC BY-SA 4.0 是教程类内容的常见选择。

## 3. 专家层引用的 Qt 源码 —— The Qt Company 许可

专家层（`tutorial/expert/`）在讲解中会引用 Qt 源码片段并标注「文件:行号」。**Qt 源码本身**
遵循 [The Qt Company 的 LGPL v3 / 商用双授权](https://www.qt.io/licensing/)；本仓库对这些片段
的引用属教学性评注，不重新声明 Qt 源码的许可。读者若使用 Qt 源码，请遵守 Qt 自身条款。

## 第三方资源

- `reference/`（本地参考，不入库）：第三方 Qt 项目，各保留原作者许可，不随本仓库分发。致谢见 [README](README.md)。
- `third_party/googletest`（v1.17.0）、`third_party/benchmark`（v1.9.5）：以 git submodule 引入，遵循各自的开源许可。
- `qt_src/qt6.9.1/`：Qt 6.9.1 源码，本地取证用，不入库，遵循 The Qt Company 许可。
