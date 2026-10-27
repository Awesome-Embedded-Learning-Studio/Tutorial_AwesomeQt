/// @file    speech_manager.h
/// @brief   TTS engine manager: enumeration, voice selection, SSML, and state tracking.
///
/// 对应教程：进阶层 05-其他模块/24-QtTextToSpeech。
/// 演示 QTextToSpeech 的引擎枚举、语音选择、SSML 标记与合成状态管理。

#pragma once

#include <QObject>
#include <QTextToSpeech>
#include <QVoice>

#include <QString>
#include <QVector>

/// @brief Manages a QTextToSpeech instance, exposing engine/voice enumeration,
///        parameter tuning, plain-text and SSML synthesis, and state tracking.
class SpeechManager : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the manager and initialises the default TTS engine.
    /// @param[in] parent  Parent QObject for ownership via Qt object tree.
    /// @note If no TTS backend is available the object remains valid but all
    ///       speak calls are no-ops and state stays Ready.
    explicit SpeechManager(QObject* parent = nullptr);

    /// @brief Destroys the manager; the underlying QTextToSpeech is owned by this
    ///        object and destroyed automatically.
    ~SpeechManager() override = default;

    // -- Engine & voice introspection -----------------------------------------

    /// @brief Returns the names of all available TTS engines on the system.
    /// @note Useful for diagnostics; on WSL2 the list may be empty.
    QVector<QString> enumerateEngines() const;

    /// @brief Returns metadata for every voice available in the current engine.
    ///        Each entry is a human-readable string: "Name (Language - Gender)".
    QVector<QString> enumerateVoices() const;

    /// @brief Switches to the voice at the given index in the voice list.
    /// @param[in] index  Zero-based index from enumerateVoices().
    /// @return true if the index was valid and the voice was applied.
    /// @note Changing voice while speaking may cause the current utterance to
    ///       be interrupted, depending on the backend.
    bool selectVoice(int index);

    // -- Synthesis parameters -------------------------------------------------

    /// @brief Sets the speech rate.
    /// @param[in] rate  Value in [-1.0, 1.0]; 0.0 is normal speed.
    void setRate(double rate);

    /// @brief Sets the pitch.
    /// @param[in] pitch  Value in [-1.0, 1.0]; 0.0 is normal pitch.
    void setPitch(double pitch);

    /// @brief Sets the volume.
    /// @param[in] volume  Value in [0.0, 1.0]; 1.0 is maximum.
    void setVolume(double volume);

    // -- Synthesis -----------------------------------------------------------

    /// @brief Speaks plain text through the current engine.
    /// @param[in] text  The text to synthesise.
    /// @note If state is currently Speaking the new text is queued by most
    ///       backends; call stop() first to discard the queue.
    void speak(const QString& text);

    /// @brief Speaks SSML-marked-up text through the current engine.
    /// @param[in] ssml  A valid SSML fragment. The caller must ensure the
    ///                  string is well-formed; malformed SSML may be treated
    ///                  as plain text by some backends.
    /// @note Not every engine supports SSML; check engine capabilities first.
    void speakSsml(const QString& ssml);

    /// @brief Pauses the current utterance.
    void pause();

    /// @brief Resumes a paused utterance.
    void resume();

    /// @brief Stops synthesis and discards the queue.
    void stop();

    /// @brief Returns the current synthesis state.
    QTextToSpeech::State state() const;

signals:

    /// @brief Emitted whenever the synthesis state changes.
    /// @param[in] state  The new state.
    void stateChanged(QTextToSpeech::State state);

private:
    /// @brief Formats a QVoice into a human-readable description string.
    /// @param[in] voice  The voice to describe.
    /// @return A string like "Zira (en_US - Female)".
    static QString formatVoiceInfo(const QVoice& voice);

    QTextToSpeech* m_tts;           ///< Core TTS engine, owned by this object.
    QVector<QVoice> m_voices;       ///< Cached voice list for the current engine.
};
