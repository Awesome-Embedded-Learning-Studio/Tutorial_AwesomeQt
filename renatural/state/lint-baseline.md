# lint 基线（lint-baseline）

- 归档日期：2026-09-28（骨架版）；2026-09-28 主控本地补测回填（原 lint agent 撞 429 限流，补测为本地脚本直跑，无 LLM 参与）
- 工具：`.claude/content_forge/tools/humanizer_lint.py --format json`（默认全量规则，error 级即阻断 READY）
- 状态：**已测（全量，2026-09-28 `scripts/renatural/plan.py --with-lint` 落地后补齐）。**

## 0. 补测说明

原工作流中 lint agent 因 API 限流失败，档案员如实留了空骨架。lint 是纯本地脚本，不需要 agent——主控直接对真迹组、试点组、旧文样本组各跑了一遍，结果如下。所有数字为实测，无估算。

## 1. 作者真迹组（测规则误伤）

| file | exit_code | 命中数 | top_rules |
|---|---|---|---|
| Tutorial_AwesomeModernCPP/documents/vol1-fundamentals/ch00/00-preface.md | 1 | 148 | clause_without_warmth 117 / banned_semicolon 7 / banned_first 5 / fake_article 6 |
| Tutorial_AwesomeModernCPP/documents/vol1-fundamentals/ch09/00-why-generics.md | 1 | 120 | clause_without_warmth 91 / banned_semicolon 14 / fake_article 6 / banned_first 1 |
| content_forge/tools/humanizer-zh/corpus/COMPARE.md | 1 | 173 | clause_without_warmth 88 / paragraph_without_people 27 / banned_semicolon 20 / banned_first 11 |
| Cinux-Book/document/journey/00-armory/05-assert.md | 1 | 30 | clause_without_warmth 26 / patched_ne_density 2 |
| （postwaver 第 4 篇成稿） | — | 待 registry 恢复 | — |

口径说明：

- 前两篇是 `approved.toml` 里 `author_verbatim` 条目的**来源文件本体**——即平台自己认证过的作者金句，其来源全文在今天的规则下全不过门；
- COMPARE.md 是「前 / 后」改稿工作坊对子，**「前」半是反面教材**，整文件直跑必然虚高，173 只说明「后」版本也非零命中（含 banned_rules_word 5——文件本身在讨论规则用语，属结构性误伤）；
- 05-assert.md 明显干净一档（30 vs 148/120），说明命中密度与体裁强相关——不是所有作者文本都在同一档。

## 2. 本仓新声/试点组（agent 新写，00-env 三篇 + 01-qtbase 两篇）

| file | exit_code | 命中数 | top_rules |
|---|---|---|---|
| tutorial/beginner/00-env/00-qt6-install-beginner.md | 1 | 96 | clause_without_warmth 71 / paragraph_without_people 6 / banned_semicolon 5 / ledger_family 2 / banned_pit 2 |
| tutorial/beginner/00-env/01-ide-setup-beginner.md | 1 | 109 | clause_without_warmth 占首 / banned_semicolon / banned_first |
| tutorial/beginner/00-env/02-cmake-first-project-beginner.md | 1 | 111 | clause_without_warmth 占首 / paragraph_without_people / fake_article |
| tutorial/beginner/01-qtbase/01-qobject-meta-system-beginner.md | 1 | 131 | clause_without_warmth 占首 / paragraph_without_people / banned_pit |
| tutorial/beginner/01-qtbase/02-signal-slot-beginner.md | 1 | 112 | clause_without_warmth 占首 / paragraph_without_people / banned_semicolon |

五篇合计 559：clause_without_warmth 410、paragraph_without_people 46、banned_semicolon 22、breathless_sentence 18、banned_first 17、fake_article 14、banned_pit 11。

## 3. 旧文样本组（测存量债务；全量待调度器）

| file | exit_code | 命中数 | top_rules |
|---|---|---|---|
| tutorial/beginner/03-widgets/20-qcheckbox-beginner.md | 1 | 181 | clause_without_warmth 占首 / paragraph_without_people / breathless_sentence |
| tutorial/beginner/06-qml/01-qml-syntax-basics-beginner.md | 1 | 186 | clause_without_warmth 占首 / paragraph_without_people / breathless_sentence |

两篇均值 184，约为新声组篇均（112）的 1.6 倍——方向符合预期（旧文更冷），但**新声组离零也极远**。

**全量计数（2026-09-28 补，71 条规则 = 65 平台 + 6 条 04-tutorial 增补）**：137 篇正文 + 8 index 全量 lint，error 命中总计 **23,316**（篇均 ~170），0 篇执行失败；逐篇计数存 `renatural/state/queue.json` 各条 reason。命中 top5：62-qmessagebox 247、69-qwizard 242、60-qdialog 237、22-qlineedit 230、50-qtablewidget 225——全部是 03 章长条目篇，与「一篇一控件、模板最重」的审计判断一致。另：echo_lint 黑名单 6638 条 n-gram，top 回声正是已废除的旧 H1 标题模板（「开发教程（新手篇）」系化石在 145 篇中 136 篇命中），其次「官方文档参」130、「核心概念讲」119、「第N个坑是」81-89、「综合练习」75——五段模板残渣全部被量化坐实。

## 4. 结论

### 4.1 核心发现：现行规则严于作者本人的金句实践

「真迹零误伤」校准门槛**已证伪**。证据链：

1. `approved.toml` 认证的作者逐字样本，其来源全文在今天的规则下 148 / 120 个 error 级命中——按 `review_gate.py:119`（只放 error 级进 READY 阻断），**作者本人进语料库的文章今天过不了自己平台的 READY 门**；
2. 连「硬禁」类规则也命中作者真迹：`banned_honestly`（说实话）×1、`banned_first`（单字先）×5+1、`banned_semicolon`（分号）×7+14——说明规则集与语料库在时间上脱节：语料按旧口径收录，规则此后多轮收紧（含「呢不再单独过关」这类明确标注的收紧）；
3. `clause_without_warmth` 的合格模式 = 子句须含人称/承接词/语气词（咱们/其实/当然/的/了/而/吧…；**「呢」已除名，「它」不在人称表内**）。作者金句约八成子句过关、两成短事实子句不过；教程体裁事实子句占比更高，试点五篇 410 处。规则要 100%，作者实践 ~80%。

**判读（2026-09-28 作者澄清后修正）**：规则严于作者实践是**有意设计，不是脱节**——humanizer 规则的约束对象是 AI 初稿，不是作者本人；作者真迹是方向的参照样本，不是及格线。「拿作者语料校准规则、给 AI 放预算」是把两类文本的靶子搞混了。全零路径也不是塞语气词：规则自己的 message 写明「优先调整主谓与承接，不得补一个『呢』了事」——要的是把冷子句重写成有主语、有承接的句子。

### 4.2 处置裁决（2026-09-28 作者澄清，覆盖主控早前的两档方案）

作者原话：「这个初稿是约束您的，我自己更清楚如何产出我的风味的。」据此：

- **单档门槛**：AI 新写文本对 humanizer **全部 error 级规则归零**（含 clause_without_warmth / paragraph_without_people / breathless_sentence / fake_article 家族）。主控早前裁的「预算档 ≤22/篇 + 显式豁免」**撤回，任何脚本与 SOP 不得实现预算 / 豁免逻辑**。
- **真迹组测量降级为背景数据**：§1 数字保留（规则与作者实践的差值客观存在，是有意设计的一部分），但不再作为任何门槛的校准输入；不登记 content_forge 上游校准 issue。
- **语气变形的防线**：规则自身「调主谓承接、禁补呢」指引 + 声音走查盯变形（问题单单列变形类）+ 作者保留否决权与 M3 终验过目——不靠放宽规则防变形。
- **试点五篇（§2）按全零口径回炉**：01-qtbase 两篇随 W3 波重写归零；00-env 三篇随 W8 波。

### 4.3 存量债务规模（全量实测，2026-09-28 调度器落地后回填）

71 规则口径（65 平台 + 6 条 04-tutorial 增补）：137 篇正文 error 合计 **23,316**、篇均 ~170，0 篇执行失败；逐篇计数在 `renatural/state/queue.json` 各条 reason。样本段预估的 1.5–2.5 万区间被证实偏保守。此数仅作重写排期与 M1 后对照，**不作验收数字**——验收口径是 §4.2 的单档全零。

## 5. 回填流程（更新）

1. ~~先跑真迹组~~ 已完成（本文件 §1–§4）；
2. ~~调度器落地后全量跑~~ 已完成（`plan.py --with-lint` 全量 137+8 篇落 queue.json，§3/§4.3 已按全量口径回填）；
3. lint 增补规则（套话/报数/引号/教程体裁）校准口径：对 AI 新稿全 error 归零（§4.2 作者澄清后的单档标准）；
4. 本文件升级 lint-baseline-v1 时附 M1 批 A 六篇的全零实测记录。
