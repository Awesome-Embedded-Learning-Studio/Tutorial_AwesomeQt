/// @file    main.cpp
/// @brief   Console demo for QtTextToSpeech advanced features.
///
/// 对应教程：进阶层 05-其他模块/24-QtTextToSpeech。
/// 四个演示：引擎枚举、参数配置、SSML 示例、SSML 组装。
/// 在 WSL2 下可能无可用 TTS 引擎，程序会优雅退出。

#include "speech_manager.h"

#include <QCoreApplication>
#include <QTimer>

#include <QDebug>

// ---------------------------------------------------------------------------
// Demo 1 — Enumerate engines and voices
// ---------------------------------------------------------------------------

/// @brief Lists every available TTS engine and, for the active engine,
///        every available voice with human-readable metadata.
/// @param[in] mgr  The SpeechManager to query.
static void demoEnumerate(SpeechManager& mgr)
{
    qDebug() << "=== Demo 1: Engine & Voice Enumeration ===";

    const auto engines = mgr.enumerateEngines();
    if (engines.isEmpty())
    {
        qWarning() << "No TTS engines found on this system.";
        qWarning() << "This is expected on WSL2 or headless environments.";
        return;
    }

    qDebug() << "Available engines:" << engines.size();
    for (int i = 0; i < engines.size(); ++i)
    {
        qDebug() << "  [" << i << "]" << engines.at(i);
    }

    const auto voices = mgr.enumerateVoices();
    qDebug() << "\nVoices for default engine:" << voices.size();
    for (int i = 0; i < voices.size(); ++i)
    {
        qDebug() << "  [" << i << "]" << voices.at(i);
    }
}

// ---------------------------------------------------------------------------
// Demo 2 — Configure voice parameters
// ---------------------------------------------------------------------------

/// @brief Shows how to select a voice and adjust rate / pitch / volume.
/// @param[in] mgr  The SpeechManager to configure.
static void demoConfigure(SpeechManager& mgr)
{
    qDebug() << "\n=== Demo 2: Voice Parameter Configuration ===";

    const auto voices = mgr.enumerateVoices();
    if (voices.isEmpty())
    {
        qDebug() << "No voices available — skipping configuration demo.";
        return;
    }

    // Pick the first available voice as a safe default.
    mgr.selectVoice(0);
    qDebug() << "Selected voice index 0.";

    // Demonstrate parameter setters. These are additive: each call replaces
    // only the targeted property.
    mgr.setRate(0.2);     // Slightly faster than normal
    mgr.setPitch(-0.1);   // Slightly lower pitch
    mgr.setVolume(0.8);   // 80% volume

    qDebug() << "Rate:   0.2  (slightly fast)";
    qDebug() << "Pitch: -0.1  (slightly low)";
    qDebug() << "Volume: 0.8  (80%)";

    mgr.speak(QStringLiteral("Voice parameters have been configured."));
}

// ---------------------------------------------------------------------------
// Demo 3 — SSML example with prosody control
// ---------------------------------------------------------------------------

/// @brief Demonstrates a hand-crafted SSML string with prosody adjustments.
/// @param[in] mgr  The SpeechManager to speak through.
/// @note SSML support depends on the backend. flite (common on Linux) ignores
///       SSML tags; speech-dispatcher may pass them through.
static void demoSsmlExample(SpeechManager& mgr)
{
    qDebug() << "\n=== Demo 3: SSML with Prosody ===";

    // A simple SSML document with rate and pitch overrides.
    // The <prosody> element adjusts delivery for a single phrase.
    const QString ssml = QStringLiteral(
        "<speak>"
        "Normal voice. "
        "<prosody rate=\"slow\" pitch=\"+5st\">"
        "Slow and high-pitched voice."
        "</prosody> "
        "<prosody rate=\"fast\" pitch=\"-5st\">"
        "Fast and low-pitched voice."
        "</prosody>"
        "</speak>"
    );

    qDebug() << "SSML payload:" << ssml;
    mgr.speakSsml(ssml);
}

// ---------------------------------------------------------------------------
// Demo 4 — Build SSML programmatically
// ---------------------------------------------------------------------------

/// @brief Builds an SSML string using break, emphasis, and prosody tags,
///        then submits it for synthesis.
/// @param[in] mgr  The SpeechManager to speak through.
static void demoSsmlBuilder(SpeechManager& mgr)
{
    qDebug() << "\n=== Demo 4: Programmatic SSML Construction ===";

    QString ssml = QStringLiteral("<speak>");

    // <break> inserts a silence pause; time is in milliseconds or seconds.
    ssml += QStringLiteral("Hello world.");
    ssml += QStringLiteral("<break time=\"500ms\"/>");

    // <emphasis> marks a word or phrase with rhetorical emphasis.
    ssml += QStringLiteral("<emphasis level=\"strong\">This is important.</emphasis>");
    ssml += QStringLiteral("<break time=\"300ms\"/>");

    // <prosody> controls rate, pitch, and volume together.
    ssml += QStringLiteral(
        "<prosody rate=\"0.8\" pitch=\"+3st\" volume=\"loud\">"
        "Slower, higher, and louder."
        "</prosody>"
    );
    ssml += QStringLiteral("<break time=\"200ms\"/>");

    // Return to normal delivery for the closing.
    ssml += QStringLiteral("Back to normal. Goodbye.");
    ssml += QStringLiteral("</speak>");

    qDebug() << "Built SSML (" << ssml.length() << "chars):";
    qDebug().noquote() << ssml;

    mgr.speakSsml(ssml);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("24-qttexttospeech-advanced"));

    SpeechManager mgr;

    // Run all demos immediately, then schedule an auto-quit.
    // We use a single-shot timer so the event loop has a chance to process
    // any queued TTS state transitions before the process exits.
    demoEnumerate(mgr);
    demoConfigure(mgr);
    demoSsmlExample(mgr);
    demoSsmlBuilder(mgr);

    // If the engine is actually speaking, wait briefly for the state machine
    // to settle; otherwise quit immediately. In WSL2 there is typically no
    // engine, so state stays Ready and we exit at once.
    QTimer::singleShot(3000, &app, &QCoreApplication::quit);

    return app.exec();
}
