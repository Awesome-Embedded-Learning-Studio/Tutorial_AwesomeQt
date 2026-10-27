/// @file    speech_manager.cpp
/// @brief   Implementation of SpeechManager — TTS engine enumeration, voice
///          selection, SSML synthesis, and state management.
///
/// 对应教程：进阶层 05-其他模块/24-QtTextToSpeech。

#include "speech_manager.h"

#include <QDebug>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SpeechManager::SpeechManager(QObject* parent)
    : QObject(parent)
    , m_tts(new QTextToSpeech(this))
{
    // Cache the default engine's voice list once at construction time.
    // Reloading on every enumerateVoices() call would be wasteful.
    m_voices = m_tts->availableVoices();

    // Forward the underlying engine's state changes so external code does not
    // need to know about the QTextToSpeech pointer.
    connect(m_tts, &QTextToSpeech::stateChanged, this,
            &SpeechManager::stateChanged);
}

// ---------------------------------------------------------------------------
// Engine & voice introspection
// ---------------------------------------------------------------------------

QVector<QString> SpeechManager::enumerateEngines() const
{
    QVector<QString> result;

    // QTextToSpeech::availableEngines() returns QStringList (QLatin1String
    // values). We copy into our return container explicitly.
    const auto engines = QTextToSpeech::availableEngines();
    for (const auto& name : engines)
    {
        result.append(name);
    }
    return result;
}

QVector<QString> SpeechManager::enumerateVoices() const
{
    QVector<QString> result;

    for (const auto& voice : m_voices)
    {
        result.append(formatVoiceInfo(voice));
    }
    return result;
}

bool SpeechManager::selectVoice(int index)
{
    if (index < 0 || index >= m_voices.size())
    {
        qWarning() << "Voice index out of range:" << index
                   << "(available:" << m_voices.size() << ")";
        return false;
    }

    m_tts->setVoice(m_voices.at(index));
    qDebug() << "Voice selected:" << m_voices.at(index).name();
    return true;
}

// ---------------------------------------------------------------------------
// Synthesis parameters
// ---------------------------------------------------------------------------

void SpeechManager::setRate(double rate)
{
    // Clamp to the documented range [-1.0, 1.0] to avoid undefined behaviour
    // with backends that do not perform their own clamping.
    rate = qBound(-1.0, rate, 1.0);
    m_tts->setRate(rate);
}

void SpeechManager::setPitch(double pitch)
{
    pitch = qBound(-1.0, pitch, 1.0);
    m_tts->setPitch(pitch);
}

void SpeechManager::setVolume(double volume)
{
    volume = qBound(0.0, volume, 1.0);
    m_tts->setVolume(volume);
}

// ---------------------------------------------------------------------------
// Synthesis control
// ---------------------------------------------------------------------------

void SpeechManager::speak(const QString& text)
{
    if (text.isEmpty())
    {
        qDebug() << "speak(): empty text, ignoring.";
        return;
    }
    m_tts->say(text);
}

void SpeechManager::speakSsml(const QString& ssml)
{
    if (ssml.isEmpty())
    {
        qDebug() << "speakSsml(): empty SSML, ignoring.";
        return;
    }

    // Qt 6.3+ supports SSML natively via QTextToSpeech::say().
    // Older versions may silently strip tags; we log for diagnostics.
    qDebug() << "Speaking SSML (" << ssml.length() << "chars)";
    m_tts->say(ssml);
}

void SpeechManager::pause()
{
    m_tts->pause();
}

void SpeechManager::resume()
{
    m_tts->resume();
}

void SpeechManager::stop()
{
    m_tts->stop();
}

QTextToSpeech::State SpeechManager::state() const
{
    return m_tts->state();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

QString SpeechManager::formatVoiceInfo(const QVoice& voice)
{
    // Build a concise, human-readable label for a voice entry.
    // QVoice does not provide a toString(), so we compose one manually.
    // Note: voice.language() returns QLocale::Language (an enum), not QLocale.
    //       Use voice.locale() to get the full QLocale with territory info.
    const QString name      = voice.name();
    const QLocale   loc     = voice.locale();
    const QString   lang     = QLocale::languageToString(loc.language());
    const QString   territory = QLocale::territoryToString(loc.territory());
    const QString langTag  = lang + QLatin1String(" (") + territory + QLatin1String(")");

    // Map the enum to a readable gender string.
    QString gender;
    switch (voice.gender())
    {
    case QVoice::Male:
        gender = QStringLiteral("Male");
        break;
    case QVoice::Female:
        gender = QStringLiteral("Female");
        break;
    case QVoice::Unknown:
    default:
        gender = QStringLiteral("Unknown");
        break;
    }

    return QStringLiteral("%1 [%2 - %3]").arg(name, langTag, gender);
}
