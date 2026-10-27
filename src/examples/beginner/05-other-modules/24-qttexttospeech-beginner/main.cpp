/**
 * Qt TextToSpeech 文字转语音示例
 *
 * 本示例演示 Qt TextToSpeech 模块的核心功能：
 * - QTextToSpeech::say() 朗读文本
 * - availableLocales() / availableVoices() 枚举可用引擎
 * - setRate() / setPitch() / setVolume() 语音参数调节
 * - 中英文切换朗读演示
 *
 * 启动后依次朗读三段文本：
 *   1. 正常语速中文
 *   2. 快速英语
 *   3. 慢速低音调中文
 *
 * 运行需要系统安装 TTS 引擎：
 *   Windows: SAPI 5（系统自带）
 *   macOS: NSSpeechSynthesizer（系统自带）
 *   Linux: speech-dispatcher
 */

#include <QDebug>
#include <QLocale>
#include <QCoreApplication>
#include <QTextToSpeech>

#include "ttsutils.h"

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
    for (const QLocale &locale : locales) {
        if (locale.language() == QLocale::Chinese) {
            tts.setLocale(locale);
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
    // 阶段 2/3：等朗读完成后依次切换
    // ========================================

    QObject::connect(&tts, &QTextToSpeech::stateChanged,
        [&](QTextToSpeech::State state) {
            if (state != QTextToSpeech::Ready) {
                return;
            }

            static int step = 0;
            step++;

            if (step == 1) {
                // 阶段 2：切换到英语，快速朗读
                for (const QLocale &loc : tts.availableLocales()) {
                    if (loc.language() == QLocale::English) {
                        tts.setLocale(loc);
                        break;
                    }
                }
                voices = tts.availableVoices();
                if (!voices.isEmpty()) {
                    tts.setVoice(voices.first());
                }

                tts.setRate(0.4);   // 稍快
                tts.setPitch(0.1);  // 稍高

                QString text2 = QStringLiteral(
                    "This is an English speech demonstration "
                    "using Qt TextToSpeech module. "
                    "The speech rate is set to 40 percent faster "
                    "than normal speed.");

                qDebug() << "[朗读 2] 快速英语（rate=0.4, pitch=0.1）";
                tts.say(text2);

            } else if (step == 2) {
                // 阶段 3：回到中文，慢速朗读
                for (const QLocale &loc : tts.availableLocales()) {
                    if (loc.language() == QLocale::Chinese) {
                        tts.setLocale(loc);
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
