---
title: "5.24 TTS 进阶：SSML 标记语言与语音合成控制"
description: "入门篇我们让 QTextToSpeech 说话了——输入文本，它就读出来。进阶篇要深入语音合成的精细控制：SSML 标记语言可以精确控制语速、音调、停顿；异步状态追踪让你知道合成到哪一步了；多引擎和多语言的切换让你构建多语言 TTS 应用。"
---

# 现代Qt开发教程（进阶篇）5.24——TTS 进阶：SSML 标记语言与语音合成控制

## 1. 前言

入门篇我们用 `QTextToSpeech` 把文本转成语音了——`say("Hello World")`，它就念出来了。但入门篇只是最基础的「给文本出声音」。如果你真的要在应用中集成 TTS 功能（比如导航播报、无障碍阅读、语音助手），很快就会发现需要更精细的控制。

导航播报中「前方 500 米」需要稍作停顿后再说「右转」；语音助手中不同情绪的话语需要不同的语速和音调；多语言应用需要在不同语言之间无缝切换；长时间文本合成需要追踪进度，不能让用户以为卡死了。

这些精细控制都依赖 `QTextToSpeech` 的高级 API 和 SSML（Speech Synthesis Markup Language）标记语言。这篇把它们拆干净。

## 2. 环境说明

本文档基于 Qt 6.5+ 编写，需要 Qt6::TextToSpeech 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS TextToSpeech)` 引入。TTS 引擎取决于平台：Windows 上使用 SAPI，macOS 上使用 NSSpeechSynthesizer，Linux 上使用 speech-dispatcher。不同平台可用的语音和功能支持程度有差异。某些 Linux 发行版需要额外安装 `speech-dispatcher` 和 `espeak-ng` 包。

## 3. 核心概念讲解

### 3.1 引擎选择与语言配置

`QTextToSpeech` 在 Qt 6 中支持多个 TTS 后端引擎。你可以通过 `QTextToSpeech::availableEngines()` 查询当前平台上可用的引擎列表，然后选择最适合的一个。

```cpp
/// @brief 列出所有可用的 TTS 引擎和语音。
void list_tts_capabilities()
{
    qDebug() << "可用引擎:" << QTextToSpeech::availableEngines();

    auto* tts = new QTextToSpeech(this);

    // 列出所有可用的语音
    const QList<QVoice>& kVoices = tts->availableVoices();
    for (const QVoice& voice : kVoices) {
        qDebug() << "语音:" << voice.name()
                 << "语言:" << voice.locale().name()
                 << "性别:" << voice.gender()
                 << "年龄:" << voice.age();
    }

    // 列出所有支持的语言
    const QList<QLocale>& kLocales = tts->availableLocales();
    for (const QLocale& locale : kLocales) {
        qDebug() << "语言:" << locale.name() << locale.nativeLanguageName();
    }
}
```

选择引擎的时候需要考虑几个因素。Windows 上的 SAPI 引擎语音质量好但配置复杂；Linux 上的 speech-dispatcher + espeak-ng 语音质量一般但覆盖语言广；某些平台可能还支持第三方引擎（比如 flite、osx）。如果你的应用需要跨平台一致的语音效果，建议测试所有平台的可用语音并给用户提供选择界面。

```cpp
/// @brief 配置 TTS 引擎和语音。
/// @param[in] tts TTS 对象。
/// @param[in] engine_name 引擎名称。
/// @param[in] locale 目标语言。
/// @param[in] voice_name 语音名称。
void configure_tts(QTextToSpeech* tts, const QString& engine_name,
                   const QLocale& locale, const QString& voice_name)
{
    // 设置引擎（需要在构造时指定或重新构造）
    // QTextToSpeech tts(engine_name);

    // 设置语言
    tts->setLocale(locale);

    // 查找并设置指定语音
    const QList<QVoice>& kVoices = tts->availableVoices();
    for (const QVoice& voice : kVoices) {
        if (voice.name() == voice_name) {
            tts->setVoice(voice);
            break;
        }
    }

    // 设置语速和音调（范围 0.0 - 2.0，1.0 为默认）
    tts->setRate(1.0);
    tts->setPitch(1.0);
    tts->setVolume(1.0);
}
```

`setRate`、`setPitch`、`setVolume` 分别控制语速、音调和音量。它们的范围是 0.0 到 2.0，1.0 是默认值。`setRate(0.5)` 是半速，`setRate(1.5)` 是 1.5 倍速。`setPitch(0.8)` 是降低音调，`setPitch(1.3)` 是升高音调。

### 3.2 SSML 标记控制语速、音调、停顿

SSML（Speech Synthesis Markup Language）是 W3C 标准的语音合成标记语言。它用 XML 标签来精确控制语音合成的各个方面：停顿时长、语速变化、音调变化、发音纠正。`QTextToSpeech` 在 Qt 6.5+ 支持传入 SSML 格式的文本。

```cpp
/// @brief 使用 SSML 标记精确控制语音合成。
/// @param[in] tts TTS 对象。
void speak_with_ssml(QTextToSpeech* tts)
{
    // SSML 文本——精确控制语速、音调、停顿
    QString kSsml = R"(
        <speak>
            <prosody rate="slow" pitch="-10%">
                前方五百米，
            </prosody>
            <break time="500ms"/>
            <prosody rate="fast" pitch="+5%">
                右转！
            </prosody>
        </speak>
    )";

    tts->say(kSsml);
}
```

这段 SSML 做了三件事：前半句「前方五百米」用慢速、低音调播放，模拟导航播报的平稳语气；中间停顿 500 毫秒，给用户反应时间；后半句「右转」用快速、高音调播放，引起注意。

SSML 的常用标签有这些。`<break time="Xms"/>` 在指定位置插入停顿，时间单位是毫秒或秒。`<prosody rate="..." pitch="..." volume="...">...</prosody>` 控制语速、音调、音量，可以用百分比或关键字（`slow`、`fast`、`high`、`low`）。`<emphasis level="...">...</emphasis>` 给文字加重语气。`<say-as interpret-as="...">...</say-as>` 控制数字、日期、电话号码等的读法。

```cpp
/// @brief SSML 的更多用法示例。
void ssml_examples(QTextToSpeech* tts)
{
    // 数字读法控制——"2024" 读作 "二零二四" 而不是 "两千零二十四"
    QString kNumber = R"(
        <speak>
            电话号码：<say-as interpret-as="telephone">138-0013-8000</say-as>
            日期：<say-as interpret-as="date" format="ymd">2024-01-15</say-as>
        </speak>
    )";

    // 多段不同风格的语音
    QString kMultiStyle = R"(
        <speak>
            <prosody rate="0.8" pitch="-5%">
                现在播报天气预报。
            </prosody>
            <break time="300ms"/>
            <prosody rate="1.0">
                今天晴转多云，气温 15 到 25 度。
            </prosody>
            <break time="500ms"/>
            <emphasis level="strong">
                明天有雨，请带伞！
            </emphasis>
        </speak>
    )";

    tts->say(kMultiStyle);
}
```

`<say-as>` 标签非常有用——它能告诉 TTS 引擎如何解读特定文本。比如电话号码 `138-0013-8000`，不加标记的话引擎可能把它当作普通数字来读；加了 `interpret-as="telephone"` 后引擎会逐位读出每个数字。

不是所有 TTS 引擎都完整支持 SSML。Windows SAPI 对 SSML 的支持比较好，Linux speech-dispatcher 的 SSML 支持则取决于后端引擎（espeak-ng 支持基本的 SSML，但某些高级标签可能被忽略）。如果你的应用依赖 SSML 的特定功能，需要在不同平台上测试兼容性。

### 3.3 异步状态追踪——synthesize 信号

`QTextToSpeech` 的语音合成是异步的。调用 `say()` 后，TTS 引擎在后台处理文本并播放音频。你可以通过 `state` 属性和 `stateChanged` 信号追踪合成和播放的进度。

```cpp
/// @brief 监听 TTS 状态变化。
/// @param[in] tts TTS 对象。
void monitor_tts_state(QTextToSpeech* tts)
{
    connect(tts, &QTextToSpeech::stateChanged, this,
        [tts](QTextToSpeech::State state) {
            switch (state) {
            case QTextToSpeech::Ready:
                qDebug() << "TTS 就绪";
                break;
            case QTextToSpeech::Speaking:
                qDebug() << "正在朗读";
                break;
            case QTextToSpeech::Paused:
                qDebug() << "已暂停";
                break;
            case QTextToSpeech::Error:
                qDebug() << "TTS 错误";
                break;
            }
        });
}
```

状态流转：`Ready` -> `Speaking`（调用 `say()` 后） -> `Ready`（播放完成后）。中间可以通过 `pause()` 进入 `Paused` 状态，再通过 `resume()` 回到 `Speaking` 状态。

在 Qt 6.5+ 中，`QTextToSpeech` 还提供了 `synthesize` 信号，它会在每个音频块合成完成时触发。这允许你获取原始 PCM 音频数据，用于自定义处理（比如保存为音频文件、通过网络传输、做音频可视化）。

```cpp
/// @brief 获取合成的音频数据。
/// @param[in] tts TTS 对象。
void capture_audio_data(QTextToSpeech* tts)
{
    // Qt 6.5+ 支持
    connect(tts, &QTextToSpeech::synthesized, this,
        [](const QAudioFormat& format, const QByteArray& audio_data) {
            qDebug() << "收到音频块:"
                     << "格式:" << format.sampleFormat()
                     << "采样率:" << format.sampleRate()
                     << "数据大小:" << audio_data.size() << "字节";
            // 可以将 audio_data 写入文件或通过网络发送
        });

    // 使用 synthesize() 而不是 say() 来获取音频数据（不播放）
    tts->synthesize("这段文字会被合成为音频数据");
}
```

`synthesize()` 和 `say()` 的区别在于：`say()` 合成后直接播放，`synthesize()` 合成后通过信号输出音频数据但不播放。这让你可以做很多高级操作——保存为 WAV 文件、通过 RTP 协议发送给远程客户端、做实时音频分析等。

### 3.4 多语言切换与发音人选择

多语言 TTS 的核心挑战是：不同语言的文本需要用对应语言的语音来朗读，否则发音会非常奇怪（比如用中文语音读英文，或者反过来）。

```cpp
/// @brief 多语言 TTS 管理器。
class MultiLanguageTts : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象。
    explicit MultiLanguageTts(QObject* parent = nullptr)
        : QObject(parent)
        , m_tts(new QTextToSpeech(this))
    {
        // 预加载可用的语音，按语言分组
        for (const QVoice& voice : m_tts->availableVoices()) {
            QString kLang = voice.locale().languageCode();
            m_voices_by_lang[kLang].append(voice);
        }
    }

    /// @brief 用指定语言朗读文本。
    /// @param[in] text 要朗读的文本。
    /// @param[in] language 语言代码（如 "zh"、"en"、"ja"）。
    void speak_in_language(const QString& text, const QString& language)
    {
        if (!m_voices_by_lang.contains(language)) {
            qWarning() << "不支持的语言:" << language;
            return;
        }

        // 切换到该语言的第一个可用语音
        QVoice kVoice = m_voices_by_lang[language].first();
        m_tts->setVoice(kVoice);
        m_tts->say(text);
    }

    /// @brief 获取支持的语言列表。
    QStringList supported_languages() const
    {
        return m_voices_by_lang.keys();
    }

private:
    QTextToSpeech* m_tts;
    QMap<QString, QList<QVoice>> m_voices_by_lang;  // 语言代码 -> 语音列表
};
```

使用时：

```cpp
auto* multi_tts = new MultiLanguageTts(this);
multi_tts->speak_in_language("你好世界", "zh");
multi_tts->speak_in_language("Hello World", "en");
multi_tts->speak_in_language("こんにちは世界", "ja");
```

多语言切换的一个注意点是：不同语言的语速感受不一样。中文正常语速下每秒 3-4 个字，英文每秒 3-4 个单词。如果你在多语言之间频繁切换，可能需要根据语言自动调整语速，否则中文听起来可能太快或英文太慢。

现在有个调试题给大家。你用 SSML 设置了 `<break time="1000ms"/>` 1 秒停顿，但实际停顿时间只有 200 毫秒。可能是什么原因？

最可能的原因是 TTS 引擎不支持 SSML 的 `<break>` 标签。不同引擎对 SSML 的支持程度不同，有些引擎会忽略不支持的标签。在 Linux 上使用 speech-dispatcher + espeak-ng 时，某些版本的 espeak-ng 对 `<break>` 的支持不完整。解决方案是检查引擎的 SSML 支持文档，或者在需要停顿的地方把文本分成多段，用定时器控制间隔调用 `say()`。

## 4. 踩坑预防

第一个坑是 Linux 上没有安装 speech-dispatcher 导致 `QTextToSpeech` 构造失败。在 Ubuntu/Debian 上需要安装 `speech-dispatcher` 和 `espeak-ng` 包。如果你发现 `QTextToSpeech::availableEngines()` 返回空列表，或者构造后状态直接进入 `Error`，检查是否安装了这些依赖。另外 WSL2 环境下可能因为没有音频设备导致 TTS 完全无法使用——建议在物理机或虚拟机上测试。

第二个坑是频繁切换语言会导致音频卡顿。每次调用 `setVoice()` 或 `setLocale()` 时，TTS 引擎需要重新初始化语音合成器。如果在播放过程中切换语言，当前播放会被打断，新语言的合成器初始化需要几百毫秒。解决方案是在切换语言之前先调用 `stop()` 等待状态回到 `Ready`，然后再切换语音和语言。或者预创建多个 `QTextToSpeech` 实例（每个实例一种语言），切换时直接用不同实例的 `say()`。

第三个坑是 SSML 文本必须以 `<speak>` 标签包裹，且必须是合法的 XML。如果你的 SSML 中有未转义的特殊字符（比如 `&`、`<`），XML 解析会失败，TTS 引擎会把整个文本当作普通文本朗读。解决方案是在拼接 SSML 时用 `QString::toHtmlEscaped()` 转义用户输入的文本，确保 SSML 结构完整。

## 5. 练习项目

练习项目是一个多语言新闻朗读器。应用从本地 JSON 文件中读取多条新闻（包含标题、正文、语言标签），用户选择一条新闻后用对应语言的 TTS 语音朗读。

新闻数据中每条记录有 `title`、`body`、`language`（zh/en/ja）三个字段。应用用 SSML 格式朗读新闻：标题用强调语气的 `<emphasis>` 标签包裹，标题和正文之间停顿 1 秒，正文中的数字用 `<say-as>` 控制读法。朗读过程中显示当前状态（就绪/朗读中/暂停/错误），提供暂停和恢复按钮。朗读完成后自动切换到下一条新闻。

完成标准是多语言新闻能正确用对应语言朗读、SSML 标记正确生效（停顿、强调可感知）、暂停/恢复正常工作、状态显示正确。在 Linux 上如果没有多语言语音包，可以用单语言测试。

提示几个关键点：用 `QJsonDocument` 从文件加载新闻数据；按语言分组选择合适的语音；SSML 文本用 `QString` 拼接时注意 XML 转义；状态追踪用 `stateChanged` 信号。

## 6. 官方文档参考链接

[Qt 文档 · QTextToSpeech](https://doc.qt.io/qt-6/qtexttospeech.html) -- TTS 核心类

[Qt 文档 · QVoice](https://doc.qt.io/qt-6/qvoice.html) -- 语音描述（名称、性别、年龄、语言）

[SSML 规范 · W3C](https://www.w3.org/TR/speech-synthesis11/) -- SSML 标记语言参考

[Qt 文档 · QAudioFormat](https://doc.qt.io/qt-6/qaudioformat.html) -- 音频格式描述

---

到这里 TTS 的进阶用法就拆完了。SSML 标记控制、异步状态追踪、多语言切换——这三个能力组合起来，足以构建专业级的语音合成应用。如果你的项目需要更高级的功能（比如实时变声、语音克隆、情感合成），那就超出了 Qt TTS 的能力范围，需要用专门的 TTS 服务（比如 Azure Speech、Google TTS）或者深度学习模型。
