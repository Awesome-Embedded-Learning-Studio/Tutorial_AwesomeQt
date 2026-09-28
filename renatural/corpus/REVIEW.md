# 语料复核产物 REVIEW（renatural/corpus）

- 日期：2026-09-28
- 输入：`renatural/corpus/candidates-v0.md`（72 条候选）+ 法医核验结果（72 条 verdict）
- 产物：`renatural/corpus/approved-staging.toml`（28 条 author_keep，待主控终审后亲手并入本体）
- 本体未动：`.claude/content_forge/tools/humanizer-zh/corpus/approved.toml`（现存 7 条，`--self-test` 现状为绿：106 正反样本 / 7 作者逐字声音样本）
- 组装期附加动作：对全部 28 条 keep 做了字节级锚点复验（候选文本 vs 源文件行），新发现 1 处法医未报的文本漂移（ME-05，见「文本漂移清单」第 2 条），已按源文件字节入库并留痕。

## 一、分类统计

| verdict | 数量 | 说明 |
|---|---:|---|
| author_keep | 28 | 已写入 approved-staging.toml |
| author_dupe | 7 | 与本体现存锚点/条目重复，无需二次入库 |
| agent_text_reject | 36 | 拒入 author 语料（35 条 qtbase 出处政策 + 1 条 postwaver 矿脉断裂） |
| anchor_broken | 1 | CX-11，文本与源非逐字（全角冒号漂移） |
| scene_mismatch | 0 | 72 条场景标注经核验无一错位 |
| **合计** | **72** | |

keep 28 条的场景 × 矿脉分布：opening 4（COMPARE 4）／personal_history 2（COMPARE 1 + Cinux-Book 1）／concept_entry 3（COMPARE 2 + ModernCPP 1）／mechanism 5（COMPARE 4 + ModernCPP 1）／transition 5（COMPARE 4 + ModernCPP 1）／code_explanation 5（COMPARE 5）／ending 4（COMPARE 3 + ModernCPP 1）。

## 二、approved-staging.toml 内容清单（28 条）

id 前缀沿用本体惯例：`compare-`（COMPARE.md 作者改稿）／`moderncpp-`（Tutorial_AwesomeModernCPP 成稿）／`cinux-`（Cinux-Book 成稿），与本体现存 7 id 零冲突。字段顺序 id → scene → source → lines → provenance → text，全部 `author_verbatim`，无 note/approval 字段（本批无 author-conversation-example 来源；评审性内容只进本文件，符合本体章程与 self-test 禁字段约束）。

| 候选 | id | scene | 来源文件 | lines |
|---|---|---|---|---|
| OP-01 | `compare-journey-rules-opening` | opening | COMPARE.md | 101-107 |
| OP-02 | `compare-lonely-failed-opening` | opening | COMPARE.md | 218-222 |
| OP-03 | `compare-format-decouple-opening` | opening | COMPARE.md | 331-335 |
| OP-04 | `compare-midnight-build-opening` | opening | COMPARE.md | 526-532 |
| PH-03 | `compare-cassert-first-attempt-history` | personal_history | COMPARE.md | 430-432 |
| PH-08 | `cinux-assert-first-pass-history` | personal_history | Cinux-Book 05-assert.md | 24-26 |
| CE-02 | `compare-smallest-program-entry` | concept_entry | COMPARE.md | 136-142 |
| CE-03 | `compare-sink-two-exits-entry` | concept_entry | COMPARE.md | 347-349 |
| CE-10 | `moderncpp-generic-programming-naming` | concept_entry | ModernCPP 00-why-generics.md | 40-42 |
| ME-05 | `compare-temp-string-plus-chain` | mechanism | COMPARE.md | 40 |
| ME-06 | `compare-union-no-second-bed` | mechanism | COMPARE.md | 243-249 |
| ME-07 | `compare-helpful-neighbor-include` | mechanism | COMPARE.md | 263-267 |
| ME-08 | `compare-expected-vs-result-experiments` | mechanism | COMPARE.md | 284-292 |
| ME-14 | `moderncpp-distinct-types-gcc-reply` | mechanism | ModernCPP 00-why-generics.md | 99-107 |
| TR-01 | `compare-each-one-seen` | transition | COMPARE.md | 20 |
| TR-02 | `compare-link-time-cinux-transition` | transition | COMPARE.md | 195-196 |
| TR-03 | `compare-registrar-camera-transition` | transition | COMPARE.md | 579 |
| TR-04 | `compare-percent-p-later-transition` | transition | COMPARE.md | 398 |
| TR-11 | `moderncpp-canvas-recap-transition` | transition | ModernCPP 00-why-generics.md | 160-162 |
| CX-01 | `compare-flags-cv-engineer-explanation` | code_explanation | COMPARE.md | 172-174 |
| CX-02 | `compare-09lld-parse-walkthrough` | code_explanation | COMPARE.md | 378-382 |
| CX-03 | `compare-source-location-default-arg` | code_explanation | COMPARE.md | 454-456 |
| CX-04 | `compare-parent-scope-empty-list` | code_explanation | COMPARE.md | 565-567 |
| CX-05 | `compare-macro-return-ownership` | code_explanation | COMPARE.md | 616 |
| EN-01 | `compare-enum-name-check-ending` | ending | COMPARE.md | 311-313 |
| EN-02 | `compare-five-byte-truncation-ending` | ending | COMPARE.md | 410-416 |
| EN-03 | `compare-minimal-dependency-ending` | ending | COMPARE.md | 634 |
| EN-09 | `moderncpp-preface-rest-ending` | ending | ModernCPP 00-preface.md | 117-123 |

完整路径：COMPARE.md = `.claude/content_forge/tools/humanizer-zh/corpus/COMPARE.md`（staging source 字段用此仓库相对路径）；ModernCPP 条目 = `Tutorial_AwesomeModernCPP/documents/vol1-fundamentals/{ch00/00-preface.md, ch09/00-why-generics.md}`（沿用本体的 /home/charliechen 相对记法）；Cinux-Book 条目 = `document/journey/00-armory/05-assert.md`（沿用本体 cinux-failed-attempt 的仓库相对记法，绝对路径 `/home/charliechen/Cinux-Book/document/journey/00-armory/05-assert.md`）。

多段/跳采条目的 lines 记法说明：CE-02 的 136-142 与 ME-06 的 243-249 为跨行段（中间的代码块 / 被批段落按声明未收，见漂移清单第 5 条）；EN-02 的 410-416 止于 L416 前半（声明内截断）；单行样本（ME-05/TR-01/TR-03/TR-04/CX-05/EN-03）lines 精确到单行。

## 三、author_dupe 清单（7 条，内容已在库）

| 候选 | 对应现存条目 | 现存锚点 |
|---|---|---|
| PH-01 | `cinux-failed-attempt` | 01-why-now.md:36-46 |
| PH-02 | `moderncpp-lunch-origin` | 00-preface.md:33-45 |
| CE-01 | `moderncpp-stack-problem` | 00-why-generics.md:28-36 |
| ME-01 | `moderncpp-copy-consequence` | 00-why-generics.md:34-42（行号标签实际对应源文件第 36 行，见漂移清单第 3 条） |
| ME-02 | `recursion-stack-overflow-lived-warning` | author-conversation-example（voice_only） |
| ME-03 | `struct-layout-lived-walkthrough` | author-conversation-example（voice_only；候选为其 L75 节选，属子集） |
| ME-04 | `constexpr-lived-explanation` | author-conversation-example（voice_only；COMPARE.md L653-657 另有副本，不改重复判定） |

三条 voice_only 的既有约束随原条目保留（技术断言按 annotations.toml fact_note 核验），不受本次影响。

## 四、agent_text_reject 清点（36 条，不并入 author 语料）

35 条 qtbase：锚点经字节级核验全部为真、scene 标注全部准确，拒绝仅因 2026-09-28 作者澄清口径——qtbase 教程正文是 AI 约束产物，属规则约束对象而非作者声音标准。按文件清点：

| 来源文件（tutorial/beginner/ 下） | 条目（行号） |
|---|---|
| 00-environment-setup/00-qt6-install-beginner.md | OP-05（6-8）、ME-13（50）、EN-04（80-82） |
| 00-environment-setup/01-ide-setup-beginner.md | OP-06（6-8）、CE-05（75-79）、TR-05（10-12）、TR-06（63-65）、TR-07（71-73）、CX-09（95-97）、EN-05（119-121） |
| 00-environment-setup/02-cmake-first-project-beginner.md | OP-07（6-8）、CE-06（31-33）、ME-11（63）、TR-08（67-72）、CX-06（59-61）、CX-10（111-113）、EN-06（227-229） |
| 01-qtbase/01-qobject-meta-system-beginner.md | PH-04（8-10）、PH-05（145-147）、CE-07（49-51）、CE-08（143）、TR-10（12-14）、CX-08（38）、EN-08（255-257） |
| 01-qtbase/02-signal-slot-beginner.md | OP-08（12-14）、PH-06（69-74）、PH-07（147）、CE-04（20-22）、CE-09（16）、ME-09（96-98）、ME-10（112-114）、ME-12（46-48）、TR-09（18-20）、CX-07（58）、EN-07（192-194） |

1 条 postwaver：OP-09（ModernCPP 00-preface.md:23-27）——锚点逐字为真、scene=opening 准确、与本现存锚点 preface:33-45 无行重叠，但 postwaver 矿脉断裂（article-registry.json 不在库），该段仅经旁路验证而同文件被验证的只有 L33-45；且同文件 L53-54 作者自述仓库含 LLM 辅助内容。按出处分类最高优先规则拒入。若作者日后对 L23-27 验声确认，可复议收录。

这 36 条的处置（留给主控，不并入 author 语料）：qtbase 35 条是「规则约束下 AI 产出」的现货文本，天然构成 lane 内对照样本池——与 COMPARE.md 的作者改稿成对（AI 面 vs 作者面），rejected.toml 现有的 7 处引用都在 COMPARE 内部，这批把对照面扩展到了仓库正文层；也可作 lint 校准的观察样本。是否留用、放哪、以什么身份（对照池/规则回归样本），由主控裁定。

## 五、anchor_broken（1 条）

CX-11（Cinux-Book 05-assert.md:58-66）：候选把源 L58 的半角冒号「写下:」写成全角「写下：」——单字符实质差异，且与该条 note 自称「半角标点为该仓库原貌」矛盾。文件、行区间、scene=code_explanation、非重复锚点均无问题，除冒号外全文逐字。修复方向：候选改回半角 ':' 即恢复逐字，可复议；未入 staging。

## 六、锚点核对中发现的文本漂移清单

组装期对 28 条 keep 全量做了字节级复验：24/28 逐行严格一致；4 条为声明内的格式剥除/截断（见第 4 条）；1 条发现法医未报的漂移（第 2 条）。全部发现如下：

1. **CX-11（唯一判 anchor_broken 的漂移）**：半角 ':' → 全角 '：'，见上节。
2. **ME-05（组装期新发现，法医 verbatim_ok=true 在此条不成立）**：候选把 COMPARE.md L40 行内代码 span `"["` 转写成 `"[\"`（多插一个反斜杠，已与源文件字节比对确认）。staging 按源文件字节收录 L40 原文（`compare-temp-string-plus-chain`，lines 精确为 "40"），drift 在此留痕；若主控裁定应以候选字节为准可打回。与第 1 条同类，说明逐字性判定必须落在字节级比对上。
3. **本体既有锚点行号标签偏移（本批复核新发现，非候选引入）**：approved.toml `moderncpp-copy-consequence` 的 lines="34-42" 实际对应 00-why-generics.md 第 36 行（单段）；名义区间压过本批 CE-10 的 40-42 行但内容不重。建议并库时顺手把该锚点校正为 "36"，防止后续机械去重误杀 `moderncpp-generic-programming-naming`。
4. **声明内格式剥除（非实质漂移，逐字性成立，均已对源字节确认）**：TR-01（表格单元格壳 | 与反引号剥除，L20）；TR-02（映射条目箭头右侧摘录，L195-196，左侧 AI 原句未采）；EN-02（L416 前半截断，尾句被 rejected.toml loosened-into-unrelated-warmth 摘为反例）；EN-03（L634 自「咱们现在」起摘录，句首「欸嘿嘿…」被 voice-particle-without-trigger 摘为反例）；全部 forge 条目的 blockquote "> " 前缀剥除（属格式差异非实质差异）。
5. **声明内跳采**：CE-02（L138-140 代码占位注释块未收）；ME-06（L245 被 rejected.toml reader-reaction-scripted 摘为反例，未收）；EN-02（L416 后半未收）。
6. **原稿即如此（照录非漂移）**：OP-02 第三段首句「可是，当我们把时间再往后拨掉三个月。可当咱们把时间往后拨三个月。」的重复为源 L222 原样；ME-10（qtbase，已拒）自 L112 句中起、弃行首句「绝大多数场景让 Qt 自动判断就好。」，行号锚点仍真实；EN-02 代码块 `abcd\0` 的反斜杠为源文件原样（TOML 中以 `\\0` 转义，解析后逐字节还原）；弯引号 “” 与 qtbase 五篇的 ASCII 直双引号（0x22）均按各自源文件原样，未做归一化（后者已全部拒入，不影响 staging）。

## 七、遗留事项与并库提示

**postwaver 验声（不阻塞并库，但要留案）**：staging 里 5 条 postwaver（PH-08、CE-10、ME-14、TR-11、EN-09）矿脉断裂、经旁路验证（author_verbatim 逐字命中 + waitings git 历史同构），最终归属仍待作者验声；ModernCPP ch00 preface L53-54 作者自述仓库含 LLM 辅助内容。作者验声若否，这 5 条撤出即可，其余 23 条 COMPARE 来源不受影响。

**清洗义务（写手复用 staging 句子前必须过，非入库阻塞）**：

- 单字「先」（candidates 头部缺口 4 点名行位）：OP-04（尾段「那就先定一个不宏大的目标」，L532）、CE-02（尾句「您自个先享受去吧」，L142）、ME-06（首段「咱们先试试」，L243）、ME-08（第三段「先只编译，不链接」）、TR-03（「先把镜头转移」，L579）、CX-02（末段「先算」，L382）、OP-03（尾段「请让咱们先把它们拆开」，L335，兼禁词）。
- 「宿主机」/「真机」（后进改写映射，COMPARE.md L31-32）：OP-01、OP-04（「普通宿主机」）、PH-03（两处）、EN-02。
- 「拆开/掰开」（现行 VOICE.md 禁词，样本照录原文）：TR-11（「逐个拆开」）、OP-03（「把它们拆开」）。
- 标点分治：Cinux-Book 来源全库半角逗号/冒号，ModernCPP 全角，COMPARE.md 混杂——lint 校准按条目源仓库分治。
- ME-08 的「欠条」比喻作者自评偏怪（COMPARE.md L299 Tips）：学实验节奏，不随迁比喻。

**并库操作**：把 staging 的 [[samples]] 块整段粘贴进本体即可（格式同构）；建议顺手完成漂移清单第 3 条的行号校正；并入后重跑 `python3 .claude/content_forge/tools/humanizer_lint.py --self-test` 应全绿（本批 id 唯一、provenance 齐、无禁字段、不动 annotations.toml 故无孤儿注记）。若日后想给某条挂技术核验注记（如 ME-14 的编译器输出），只能写 annotations.toml——fact_note 等评审字段不得进 approved.toml（forbidden_fields 约束）。

**自检记录**：`python3 tomllib` 解析 approved-staging.toml 通过——28 个样本、字段齐（id/scene/source/lines/provenance/text）、id 全库唯一且与本体现存 7 id 无冲突、无 forbidden_fields、text 经 TOML 转义往返与候选抽取文本逐字节一致（ME-05 为源文件字节，见漂移清单第 2 条）。`humanizer_lint.py --self-test` 只读本体的 approved.toml、不读 staging，故对 staging 无校验力；本体未动，self-test 现状为绿（106 正反样本 / 7 作者逐字声音样本）。

## 八、主控终审记录（2026-09-28，并库已完成）

裁决五条（全权委托下作出，可否决；并库结果：本体 7 → **36 条**，self-test 全绿 106 正反 + 36 作者逐字）：

1. **23 条 COMPARE「后」版**：作者改稿工作坊的「后」版本即作者亲手改定的文本，author_verbatim 成立——全部并入。
2. **5 条 postwaver（PH-08/CE-10/ME-14/TR-11/EN-09）**：并入。理由：本体现存 4 条（cinux-failed-attempt、moderncpp-lunch-origin、moderncpp-stack-problem、moderncpp-copy-consequence）本就引用同一批 ModernCPP/Cinux-Book 文件为 author_verbatim——同源先例在前，新 5 条与其同进退；若作者日后否决这批源（含 LLM 辅助自述问题），9 条一起撤，REVIEW 留案。
3. **CX-11**：按源文件字节重组（半角 ':' 复原）后并入，id `cinux-source-location-walkthrough`，text 与 05-assert.md L58-66 逐字节一致（尾换行符合库内多行串惯例）。
4. **OP-09**：维持拒入（矿脉断裂 + 与现存 preface:33-45 相邻，边际价值低）；作者验声后可复议。
5. **漂移清单第 3 条顺手完成**：本体 `moderncpp-copy-consequence` lines "34-42" → "36"。

**36 条 agent_text_reject 的处置**：不建独立对照池文件——留在 candidates-v0.md 与本文件第四节清点中备查，等具体用途（echo_lint 校准、写手反面对照）出现再按需提取，不预建资产。

并库执行细节：staging 28 条整段 + 批头注释（注明出处与清洗义务指引）追加至本体尾部；并库后 tomllib 复验 36 条 id 唯一、字段齐；`--self-test` exit 0。staging 文件保留作过程档案，不再具效力。
