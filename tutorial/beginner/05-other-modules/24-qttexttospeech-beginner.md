# 现代Qt开发教程（新手篇）5.24--Qt TextToSpeech 文字转语音

## 1. 前言：让 Qt 应用"开口说话"

文字转语音（Text-to-Speech，TTS）在桌面应用中看起来是个挺小众的需求，但实际场景远比想象中多。工业上位机软件在报警时需要语音播报故障信息——操作员不一定时刻盯着屏幕，但听到声音一定会注意到。车载信息娱乐系统要把导航指令、短信内容转成语音读出来。无障碍辅助工具需要把界面上的文字读给视障用户。甚至一些效率工具也会用 TTS 来朗读通知内容，让你在干别的事情时也能收到提醒。

Qt TextToSpeech 模块封装了各平台的原生 TTS 引擎——Windows 上用 SAPI，macOS 上用 NSSpeechSynthesizer，Linux 上用 speech-dispatcher。你的代码只需要调用统一的 Qt API，不需要关心底层调用的是哪个引擎。接口设计得很简洁：创建一个 QTextToSpeech 对象，调用 say() 传入文本，引擎就会异步地把文本转换成语音并通过音频设备播放出来。你还可以枚举系统上可用的语言和声音，调整语速、音调和音量。

这篇我们要做的是用 QTextToSpeech 朗读文本，枚举可用的语言和声音，调整语速、音调和音量参数，并实现一个简单的命令行朗读工具。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 TextToSpeech 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS TextToSpeech)
```

Qt TextToSpeech 从 Qt 5.8 开始提供，在 Qt 6 中属于附加模块。它在不同平台上的后端依赖不一样：Windows 10/11 自带 SAPI 5 语音合成引擎，无需额外安装；macOS 自带 NSSpeechSynthesizer；Linux 上需要安装 speech-dispatcher（大部分桌面发行版默认已安装，如果没有可以手动安装 `sudo apt install speech-dispatcher`）。移动端 Android 使用系统的 TTS 引擎，iOS 使用 AVSpeechSynthesizer。

工具链方面没有特殊要求：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+。运行时需要有音频输出设备。有一点要特别注意：Linux 上的 speech-dispatcher 服务需要正在运行，否则 QTextToSpeech 初始化会失败。可以用 `spd-say "hello"` 命令测试 speech-dispatcher 是否正常工作。

## 3. 核心概念讲解

### 3.1 QTextToSpeech::say() 朗读文本

QTextToSpeech 是整个模块的核心类。最基本的用法只需要三行代码：创建对象、设置语言、调用 say()。

```cpp
#include <QTextToSpeech>

QTextToSpeech tts;

// 设置语言为中文（如果系统支持）
tts.setLocale(QLocale(QLocale::Chinese, QLocale::China));

// 朗读文本
tts.say(QStringLiteral("你好，这是一个语音测试"));
```

say() 的调用是异步的——它把文本提交给 TTS 引擎后立即返回，引擎在后台线程中合成语音并通过音频设备播放。你可以通过 stateChanged 信号来跟踪朗读状态：QTextToSpeech::Ready 表示引擎空闲（朗读完成或尚未开始），QTextToSpeech::Speaking 表示正在朗读，QTextToSpeech::Paused 表示暂停，QTextToSpeech::Error 表示出错。

如果你在引擎正在朗读时再次调用 say()，当前的朗读会被中断，新的文本立即开始播放。这个行为类似于浏览器的 speechSynthesis.speak()——后调的 say() 会取消前一次还没读完的内容。如果你需要排队朗读多段文本，需要自己维护一个队列，在 stateChanged 信号变为 Ready 时取出下一段文本调用 say()。

```cpp
QStringList pendingTexts = {
    QStringLiteral("第一段文字"),
    QStringLiteral("第二段文字"),
    QStringLiteral("第三段文字")
};

QObject::connect(&tts, &QTextToSpeech::stateChanged,
    [&](QTextToSpeech::State state) {
        if (state == QTextToSpeech::Ready && !pendingTexts.isEmpty()) {
            tts.say(pendingTexts.takeFirst());
        }
    });

// 开始朗读第一段
if (!pendingTexts.isEmpty()) {
    tts.say(pendingTexts.takeFirst());
}
```

还有一个 stop() 方法可以立即停止当前朗读，pause() 暂停朗读，resume() 从暂停位置继续。pause() 和 resume() 不是所有平台后端都支持——如果不支持，pause() 可能被当作 stop() 处理。

### 3.2 availableLocales() / availableVoices() 枚举引擎

QTextToSpeech 提供了两个方法来枚举系统上可用的语言和声音。availableLocales() 返回支持的语言列表，availableVoices() 返回当前语言下可用的声音列表。这里的"声音"指的是不同的音色——同一个语言可能有多个声音可选，比如一个男声一个女声。

```cpp
// 枚举所有支持的语言
QList<QLocale> locales = tts.availableLocales();
for (const QLocale &locale : locales) {
    qDebug() << "语言:" << locale.name()
             << locale.nativeLanguageName()
             << locale.nativeCountryName();
}

// 选择中文语言
tts.setLocale(QLocale(QLocale::Chinese, QLocale::China));

// 枚举当前语言下的可用声音
QList<QVoice> voices = tts.availableVoices();
for (const QVoice &voice : voices) {
    qDebug() << "声音:" << voice.name()
             << "性别:" << voice.gender()
             << "年龄:" << voice.age();
}

// 使用第一个可用声音
if (!voices.isEmpty()) {
    tts.setVoice(voices.first());
}
```

QVoice 对象包含几个属性：name() 是声音的人类可读名称（比如 "Microsoft Huihui" 或者 "Ting-Ting"），gender() 返回 QVoice::Male 或 QVoice::Female，age() 返回 QVoice::Child、QVoice::Adult、QVoice::Senior 等枚举值。需要注意的是，不是所有平台后端都能准确提供 gender 和 age 信息——有些后端会返回 QVoice::Unknown。

一个容易踩的坑是：setLocale() 之后必须重新调用 availableVoices()，因为声音列表是和语言绑定的。如果你在 setLocale() 之前获取了 voices 列表，然后 setLocale() 切换了语言，之前的 voices 列表就作废了——对那些属于其他语言的 voice 调用 setVoice() 会被引擎忽略。

### 3.3 setRate() / setPitch() / setVolume() 语音参数

QTextToSpeech 提供了三个参数来调节语音输出效果。setRate() 控制语速，setPitch() 控制音调，setVolume() 控制音量。三个参数都接受一个 double 值，范围和含义如下：

setRate() 的参数范围是 -1.0 到 1.0。0.0 是正常语速，-1.0 是最慢（大约正常语速的一半），1.0 是最快（大约正常语速的两倍）。实际效果取决于底层引擎——有些引擎的极限范围可能比标称值小。

```cpp
tts.setRate(0.0);   // 正常语速
tts.setRate(-0.5);  // 慢速（大约 75% 速度）
tts.setRate(0.5);   // 快速（大约 125% 速度）
```

setPitch() 的参数范围同样是 -1.0 到 1.0。0.0 是正常音调，正值提高音调，负值降低音调。这个参数的效果比较微妙——它影响的是基频（fundamental frequency）的偏移，不是简单地升调或降调。不同引擎的实现差异也比较大，有些引擎可能完全忽略这个参数。

setVolume() 的参数范围是 0.0 到 1.0。0.0 是静音，1.0 是最大音量。这个参数控制的是 TTS 引擎输出的音量，和系统音量是独立的——如果你的系统音量本身很小，即使 setVolume(1.0) 听起来也不会大声。

```cpp
tts.setRate(0.3);    // 稍快一点
tts.setPitch(-0.1);  // 稍微低沉一些
tts.setVolume(0.8);  // 80% 音量
tts.say(QStringLiteral("这是一段调节过参数的语音"));
```

三个参数都可以在 say() 调用之前或者朗读过程中动态修改——修改后会立即影响后续的语音合成。如果你在朗读过程中改变 rate，当前正在播放的那句话不会受影响，但引擎在合成下一句话时会使用新的 rate 值（因为 TTS 通常是逐句合成的）。

### 3.4 无障碍功能集成

Qt TextToSpeech 在无障碍（Accessibility）场景中扮演重要角色。Qt 的无障碍框架（QAccessible）提供了界面元素的可访问性信息——角色、名称、描述、值等。你可以结合 QAccessible 和 QTextToSpeech 实现一个简单的屏幕阅读器：当焦点切换到某个控件时，自动朗读该控件的 accessible name 和 description。

```cpp
// 监听焦点变化事件
QCoreApplication::instance()->installEventFilter(myEventFilter);

// 在 eventFilter 中处理焦点变化
bool MyEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        QAccessibleInterface* iface = QAccessible::queryAccessibleInterface(watched);
        if (iface) {
            QString text = iface->text(QAccessible::Name);
            if (!text.isEmpty()) {
                tts.say(text);
            }
        }
    }
    return QObject::eventFilter(watched, event);
}
```

这种方式适合自定义的无障碍功能。如果你的应用只需要基本的 TTS 能力（比如朗读通知、播报警报），直接用 say() 就够了，不需要涉及 QAccessible 的复杂接口。但如果你要做一个面向视障用户的应用，建议结合 QAccessible 为所有交互控件设置合理的 accessible name 和 description。

## 4. 综合示例：命令行文本朗读工具

我们把前面的内容整合成一个命令行 TTS 工具。程序启动后列出系统支持的语言和声音，然后使用默认语言和声音朗读一段中文文本和一段英文文本，同时展示语速调节的效果。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS TextToSpeech)

qt_add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::TextToSpeech)
```

main.cpp 的完整代码：

```cpp
#include <QCoreApplication>
#include <QDebug>
#include <QLocale>
#include <QTimer>
#include <QTextToSpeech>

/// @brief 打印系统可用的语言和声音信息
static void printAvailableEngineInfo(QTextToSpeech &tts)
{
    qDebug() << "=== 可用语言列表 ===";
    QList<QLocale> locales = tts.availableLocales();
    for (const QLocale &locale : locales) {
        qDebug() << " " << locale.name()
                 << locale.nativeLanguageName()
                 << locale.nativeCountryName();
    }

    qDebug() << "";
    qDebug() << "=== 当前语言下的可用声音 ===";
    QList<QVoice> voices = tts.availableVoices();
    for (const QVoice &voice : voices) {
        qDebug() << " " << voice.name()
                 << "gender:" << voice.gender()
                 << "age:" << voice.age();
    }
    qDebug() << "";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt TextToSpeech 文字转语音示例";
    qDebug() << "本示例演示 say() 朗读 + 语言声音枚举 + 语速音调调节";
    qDebug() << "";

    QTextToSpeech tts;

    // 如果初始化失败，直接退出
    if (tts.state() == QTextToSpeech::Error) {
        qCritical() << "TTS 引擎初始化失败";
        qCritical() << "请确认系统安装了 TTS 引擎：";
        qCritical() << "  Windows: SAPI 5（系统自带）";
        qCritical() << "  macOS: NSSpeechSynthesizer（系统自带）";
        qCritical() << "  Linux: speech-dispatcher";
        return -1;
    }

    // 打印引擎信息
    printAvailableEngineInfo(tts);

    // 选择中文语言（如果可用）
    QList<QLocale> locales = tts.availableLocales();
    bool hasChinese = false;
    for (const QLocale &locale : locales) {
        if (locale.language() == QLocale::Chinese) {
            tts.setLocale(locale);
            hasChinese = true;
            qDebug() << "已选择语言:" << locale.name()
                     << locale.nativeLanguageName();
            break;
        }
    }

    // 选择第一个可用声音
    QList<QVoice> voices = tts.availableVoices();
    if (!voices.isEmpty()) {
        tts.setVoice(voices.first());
        qDebug() << "已选择声音:" << voices.first().name();
    }

    qDebug() << "";

    // ========================================
    // 阶段 1：正常语速朗读中文
    // ========================================

    tts.setRate(0.0);
    tts.setPitch(0.0);
    tts.setVolume(0.9);

    QString text1 = QStringLiteral(
        "你好，这是 Qt 文字转语音功能的演示。"
        "Qt 的 TTS 模块封装了各平台的原生语音引擎，"
        "你只需要调用 say 方法，就能让应用开口说话。");

    qDebug() << "[朗读 1] 正常语速中文";
    tts.say(text1);

    // ========================================
    // 阶段 2：等朗读完成后，用快速朗读英文
    // ========================================

    QObject::connect(&tts, &QTextToSpeech::stateChanged,
        [&](QTextToSpeech::State state) {
            if (state != QTextToSpeech::Ready) {
                return;
            }

            // 阶段 2：切换到英语，快速朗读
            static int step = 0;
            step++;

            if (step == 1) {
                // 切换到英语
                for (const QLocale &locale : tts.availableLocales()) {
                    if (locale.language() == QLocale::English) {
                        tts.setLocale(locale);
                        break;
                    }
                }
                voices = tts.availableVoices();
                if (!voices.isEmpty()) {
                    tts.setVoice(voices.first());
                }

                tts.setRate(0.4);  // 稍快
                tts.setPitch(0.1); // 稍高

                QString text2 = QStringLiteral(
                    "This is an English speech demonstration "
                    "using Qt TextToSpeech module. "
                    "The speech rate is set to 40 percent faster "
                    "than normal speed.");

                qDebug() << "[朗读 2] 快速英语（rate=0.4, pitch=0.1）";
                tts.say(text2);

            } else if (step == 2) {
                // 阶段 3：回到中文，慢速朗读
                for (const QLocale &locale : tts.availableLocales()) {
                    if (locale.language() == QLocale::Chinese) {
                        tts.setLocale(locale);
                        break;
                    }
                }
                voices = tts.availableVoices();
                if (!voices.isEmpty()) {
                    tts.setVoice(voices.first());
                }

                tts.setRate(-0.5);  // 慢速
                tts.setPitch(-0.2); // 低沉

                QString text3 = QStringLiteral(
                    "这段话使用了较慢的语速和较低的音调，"
                    "适合需要逐字辨听的场景。");

                qDebug() << "[朗读 3] 慢速中文（rate=-0.5, pitch=-0.2）";
                tts.say(text3);

            } else {
                // 所有朗读完成，退出程序
                qDebug() << "";
                qDebug() << "所有朗读演示完成";
                QCoreApplication::quit();
            }
        });

    return app.exec();
}
```

运行程序后，控制台会先打印系统支持的语言和声音列表，然后依次朗读三段文本：第一段是正常语速的中文，第二段是快速英语，第三段是慢速低音调的中文。每段朗读完成后自动切换到下一段。

几个实现细节解释一下。程序使用了一个静态局部变量 step 来跟踪朗读进度——每次 stateChanged 信号触发且状态为 Ready 时，step 递增，根据 step 的值决定播放哪一段文本。这种方式比嵌套回调更清晰，但要注意 step 是一个静态变量，如果你多次调用这个函数会有状态残留的问题，这个示例里不存在这个情况。

语言切换通过遍历 availableLocales() 找到目标语言然后 setLocale() 实现。切换语言后必须重新获取 availableVoices()——因为声音列表和语言绑定，不同语言下可用的声音完全不同。如果你切换到英语后还使用中文的 QVoice 对象调用 setVoice()，引擎会忽略这个调用并使用默认声音。

如果你的系统没有安装中文 TTS 声音，中文部分的朗读会静默失败或者使用默认语言播放。在 Windows 上通常自带中文声音（如 Microsoft Huihui），macOS 自带 Ting-Ting，Linux 上需要安装 espeak-ng 和中文语音数据包。

## 5. 练习项目

练习项目：命令行朗读器。

在示例基础上实现一个交互式命令行朗读工具：程序启动后进入命令循环，用户输入文本后按回车立即朗读，支持以下命令：`:lang zh_CN` 切换语言、`:voice N` 切换到第 N 个声音、`:rate 0.5` 设置语速、`:pitch -0.3` 设置音调、`:volume 0.8` 设置音量、`:list` 列出当前语言和声音、`:quit` 退出程序。普通文本直接朗读。

完成标准是这样的：输入 "Hello World" 按回车后立即开始朗读；输入 `:rate 0.5` 后下一段朗读明显变快；输入 `:lang en_US` 后切换到英语声音；连续快速输入多段文本时，前一段被中断，最后一段完整朗读。

几个实现提示：命令循环可以用 QTextStream 从 stdin 逐行读取，在非 GUI 线程中运行。用 `:` 前缀区分命令和普通文本。QTextToSpeech 的 say() 调用会自动中断前一次朗读，不需要你手动 stop()。如果你需要在 GUI 应用中做同样的事情，可以把 QTextStream 的读取放在 QThread 中，通过信号把文本传到主线程调用 say()。

## 6. 官方文档参考

[Qt 文档 · QTextToSpeech](https://doc.qt.io/qt-6/qtexttospeech.html) -- 文字转语音核心类，提供 say / pause / stop / resume 接口

[Qt 文档 · QVoice](https://doc.qt.io/qt-6/qvoice.html) -- 声音描述类，包含名称、性别、年龄等属性

[Qt 文档 · Qt TextToSpeech 模块](https://doc.qt.io/qt-6/qttexttospeech-index.html) -- TTS 模块总览与平台支持说明

[Qt 文档 · QAccessible](https://doc.qt.io/qt-6/qaccessible.html) -- Qt 无障碍框架，与 TTS 结合实现屏幕阅读

---

到这里就大功告成了。Qt TextToSpeech 把文字转语音这件事降到了"创建对象、调用 say()"的难度——底层各平台的 TTS 引擎差异被完全屏蔽，统一的 Qt API 让同一份代码在 Windows、macOS、Linux 上都能朗读。setRate / setPitch / setVolume 提供了足够的参数调节空间，availableLocales / availableVoices 让你能在运行时动态选择语言和声音。对于工业报警、车载信息、无障碍辅助这些场景，这个模块提供了刚刚好的抽象层次——不需要对接各平台原生 API，也不需要引入重量级的第三方 TTS 服务。
