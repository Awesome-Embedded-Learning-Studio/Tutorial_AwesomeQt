# 实例库批量推进 · 累计清单（2026-06-27 起，作者远门期间）

> AI 自主推进 app/model 栏成品。本文件两栏累计，作者回来一眼审完：
> - **🟡 待拍板**：AI 拿不准、用默认推进了的设计决策（功能边界 / 依赖取舍 / 命名），作者定夺
> - **🟢 已验证**：每件的构建 / offscreen / 对抗 review / 链接 结论
>
> 推进纪律：每件过构建门(零 warning) + offscreen/review 验证 + 链接 0 broken + 本地 commit(无 AI 署名，不 push)。

## 🟢 已验证

### image-viewer ✅（app 栏·commit 20238db·分支 instance/app-image-viewer）
- 构建：从 app/ 层 cmake，零 warning 零 error
- offscreen 像素验证：原图上红下蓝 ✓ / 旋转 90° 左蓝右红（顺时针方向正确）✓ / 缩放 2x 画布 800x600 ✓
- 对抗 review：3 维度（Qt API / 交互边界 / 生命周期），核心渲染变换 + 对象树 + 生命周期确认正确无崩溃级 bug；修正 6 处交互/边界 risk（navigate 循环跳坏图·成功才提交游标 / 幻灯片态坏图静默 / 构造顺序 setupCentral 提前 / 手动翻页重置 timer / enterSlideshow 守 current_index_ / exitSlideshow 恢复 maximized）
- 链接：check_links 全库 0 broken
- 文档：Full 导览 + Handbook 5 文件（app 栏双文档范式首立）

### json-editor ✅（app 栏 01-dev-tools·本批·构建+offscreen+review 全过）
- 构建：gate 零 warning（对抗 review 修 7 处后重建）
- offscreen：树节点数/Format round-trip/Compact/bad-JSON 清空 全 PASS，无 stderr warning
- 对抗 review（2 维度）：修 3 真 bug——①整数判定 qFuzzyCompare 对 |d|≥1e12 量级失效→改精确判定 d==floor(d)&&isfinite + qint64 范围检查(避 UB) ②static_cast<qint64> 越界 UB ③Save 在 QTextStream flush 前 close QFile(缓冲截断)；+3 risk(Save 补 .json 后缀/parseError offset 标 @byte/光标注释)+1 nit(构造注释)

### sqlite-browser ✅（app 栏 10-database-tools·本批·构建+offscreen+review 全过）
- 构建：gate 零 warning（对抗 review 修 7 处 + static 函数 this 误用修正后）
- offscreen：表列表 [other,t]/选中 t 行数 3/状态栏 PASS；**removeDatabase "connection still in use" warning 已消除**（作用域修复生效，stderr 干净）
- 对抗 review（2 维度，带 qt_src 实证）：修 2 真 bug——①closeDatabase+openDatabase 的 removeDatabase 时局部 QSqlDatabase 副本仍活(Qt 反模式,实证 qsqldatabase.cpp:140)→嵌套作用域 ②rowCount 对 >255 行偏低(SQLite QuerySize=false,prefetch 255)→fetchMore 拉全量；+3 risk(连接名加 QAtomicInt 序号防多窗口撞名/OnFieldChange 切表前 submitAll/错误文本统一 err.text())+1 nit

## 🟡 待拍板（AI 用默认推进，作者回来定夺）

### json-editor
- **树结构**：用虚拟 (root) 顶层（顶层恒 1，doc 互链「顶层节点数」语义稳定）vs 扁平树（object 键直接当顶层）。AI 取虚拟 root；作者若要扁平改 populateTree 去 root 一层（offscreen expected_top 从 1 改 N）
- **Format/Compact 工具栏图标**：QStyle 无贴切 SP_ 图标，只有文字（菜单/工具栏都能用）。作者若要图标需引外部 svg（本成品零外部资源）
- **公共驱动接口** loadTextAndValidate / topLevelNodeCount / editorText：harness + 文档演示用。editorText 纯 harness 粗校验，可删（删则 harness 不测 compact 换行）

### sqlite-browser
- **public openDatabase(filePath)**：harness + 程序化打开用（非为测试硬塞的 private）。作者若不想整机 demo 暴露此 API 可改 private（harness 改走 QSqlDatabase 直验）
- **10-database-tools 新类目命名**：AI 按 registry 10-database-tools 立。作者若有命名约定（如 qt-sql/）可改路径
- **编辑回写 + 任意 SQL 执行未 offscreen 测**：靠 build 门 + 代码审覆盖（offscreen 模拟单元格编辑/SQL 执行较脆）。状态栏「3 row(s)」已证 loadTable→select 真链路通
- **多语句 SQL 被 SQLite 驱动拒绝**（SQLITE_MISUSE）：非 bug，文档/troubleshooting 已注明「仅支持单条 SQL」

### image-viewer（补充，commit 时未记）
- 无遗留待拍板（已审结合入范式）

---
