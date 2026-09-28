# renatural lane SOP —— awesomeqt-base（content_forge 零 fork 执行规程）

- lane：`awesomeqt-base`（content_forge `--mode editorial`）；本文件是本仓对平台 SOP 的补充与两处显式修订，冲突时以本文件为准（依据 content_forge 指令优先级：用户最新指令 > SOP）。
- 版本冻结：`renatural/state/lane.json` 登记 content_forge SHA / 规则哈希 / prompt 版本 / PERSONA 版本；**变更只落批间窗口**，批进行中不动。

## 1. 单批 runbook

1. `python3 scripts/renatural/batch.py next` 取批 → 逐篇 `python3 scripts/renatural/preflight.py <path>`（example 存在 + cmake 直跑 + 行为断言核对 + 邻篇摘要 / glossary / 开场走法指派拼装）；失败篇降级 blocked 不阻塞批，**族内 preflight 失败率 >50% 触发族级暂停线**。
2. 建分支 `renat/<wave>/<batch>-<slug>`；批清单落账本（`ledger.py`）并写入阶段汇报。
3. 逐篇闭环：`review_gate.py init --lane awesomeqt-base --mode editorial --source <base 快照>`（`CONTENT_FORGE_REVIEW_HOME=renatural/state/`）→ 写手按家族 prompt 整文件重写 → 四走查（声音 / 保真 / 连续性 / 指代）问题单 → 修订连读接缝 → recheck。
4. `python3 scripts/renatural/accept.py <path>`：humanizer error 级归零（零豁免、零预算）+ invariants + 战役红线，全过 exit 0 并写账本。
5. PR（label `renatural-batch`）→ CI（`.github/workflows/renatural.yml`）全绿；抽样包（最险篇全读 + 随机 2 开头）由全新上下文 agent 冷读出具问题单贴 PR，供作者随时翻阅、不阻塞。
6. agent 抽样通过 → squash 合并；否决 → revert + requeue + prompt 升版，只有未合并的族内兄弟批重跑。

## 2. 共用红线（所有家族 prompt 引用此节，写手动读）

**机器红线（accept.py / CI 把关，全零）**：humanizer 全部 error 级规则归零（含 clause_without_warmth 家族——修法是调主谓与承接结构，**不得补「呢」了事**）；「」直角引号 =0（正文用中文弯引号 “”，代码块与行内代码内一律不动）；`^## \d+.` 报数标题 =0；H1 卷名前缀（「现代Qt开发教程（新手篇）N.N——」）=0；套话黑名单 =0（你会发现 / 你可能会问 / 这个问题非常好 / 说实话我第一次 / 四个核心维度 / 从四个方面展开 / 血压 / 真香 / 大功告成 / 链接已验证 / ━≥3）；「对吧」全篇 ≤3；单字「先」全禁；正文完整可运行代码 =0（完整工程住 `examples/`，每工程五件套且 cmake 直跑成功）；手写卷级进度 =0（进度只住 index 与账本）。

**人称（VOICE.md 语义级）**：咱们 = 桌边一起动手；您 = 读者自己的操作与判断；笔者 = 作者独扛的事（亲历翻车、拍板立规）；「我」只活在引号内拟人对话与代码字符串。人称替换是语义级，不是 sed。

**开场与调度**：开场五走法（抛技术问题 / 维护遭遇倒放 / 报错信息起手 / 最小实验起手 / 历史演进起手）由 preflight 指派，相邻篇不得同走法；register 三档不告知写手（知道「这篇要活泼」就会表演活泼）。

**断言三锚**：每条 Qt 行为断言至少落一锚——配套 example 实测（A 类检查点断言必须先 `cmake -B build && cmake --build build` 实测再写）/ `qt_src/qt6.9.1` 文件:行号 / doc.qt.io 标 Qt 版本（基线 6.9.1）。断言与所贴代码必须互证：贴出的代码撑不起论证就改代码或改论证（基线冷读的 qcheckbox 篇即栽在这里）。

**坑三要素**：场景、具体崩法（信号不触发 / vtable 链接错 / 静默失败等可观察现象）、从根因长出的解法。只报雷名不给爆炸现场 = 没写完（基线 qml 篇反面教材）。坑穿插行文，不设独立栏目。

**语料复用与清洗义务**：方向参照 `.claude/content_forge/tools/humanizer-zh/corpus/approved.toml`（36 条作者逐字）——学节奏与句型，**不逐字搬**；若确要逐字借用，先过 `renatural/corpus/REVIEW.md` §七清洗义务（含单字「先」「宿主机」「拆开」的句子必须清洗后用，Cinux 条目半角标点不迁入全角仓库，ME-08「欠条」比喻不随迁）。AI 产出的 qtbase 旧文不是语料，任何句子不得从中搬运。

**echo 纪律**：新稿不得命中 `renatural/state/echo-blacklist.json`（`echo_lint.py --check` 零命中）；不复活旧文句式，哪怕它很顺手。

**导航与术语**：篇尾导航块四行（上一篇 / 下一篇 / 同族下一篇或同主题跨层深读——无对应篇整行省略，禁硬凑 / 本部 index），一律内容描述式链接，禁「站 NN」编号指代；术语用 preflight 拼装的 glossary 定名（槽函数不换写「回调的等价物」），不自造同义替换。

**description 工序**：从「正文第一句复制」改为关键词密度摘要（机制要点 + 各坑崩法 + 示例路径 + cmake 命令），写手出初稿、走查单独签收，不搭车。

## 3. 对平台 SOP 的两处显式修订（D6 全权委托落地）

1. **「作者认可前不得宣称终稿」→ 修订为 agent 审门**：2026-09-28 作者全权委托（原话「从头到尾都是您和您的Agent接手干哈」），签收改为全新上下文 agent 双盲冷读 + echo_lint + 控制图 + 阶段汇报留痕；作者保留随时否决权与 M3 终验过目权。
2. **冷读判卷**：按 `renatural/PERSONA.md` §3 执行——三分制（消化复述 / 照抄复述 / 空心对），只有消化复述计通过；空心对必须在问题单点名「文章没给，读者补的」；问答饱和不构成通过依据（基线教训）。

## 4. 家族 prompt 清单

| 文件 | 家族 | 适用 |
|---|---|---|
| `theme.md` | 主题篇 | 01-qtbase 概念篇、02-qtgui、03 章 01–10 主题篇、04-qtnetwork（M1A 用此） |
| `detail.md` | 条目篇 | 03 章控件条目 11–74（含 `--chief` 族长变体；M1B 用此） |
| `qml.md` | QML 篇 | 06-qml 7 篇（W7；含命名豁免占位） |
| `light.md` | 轻改篇 | W6 mechanical 剥离 + editorial、W8 env/index 部首 |

版本随 lane.json 冻结；升版在批间窗口，理由写进阶段汇报。
