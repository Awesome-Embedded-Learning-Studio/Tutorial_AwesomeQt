/// @file    audio_scene.h
/// @brief   Spatial audio scene manager using Qt Spatial Audio API.
///
/// Demonstrates QAudioEngine, QSpatialSound, QAudioRoom, and QAudioListener
/// to create a 3D sound field with distance attenuation and room acoustics.

#pragma once

#include <QAudioEngine>
#include <QAudioListener>
#include <QAudioRoom>
#include <QObject>
#include <QSpatialSound>
#include <QVector3D>

#include <memory>
#include <vector>

/// @brief Manages a 3D spatial audio scene with multiple sound sources,
///        a listener, and an acoustic room.
class AudioScene : public QObject
{
    Q_OBJECT

public:
    /// @brief Construct the scene with a parent for Qt object tree ownership.
    /// @param[in] parent Parent QObject for lifecycle management.
    explicit AudioScene(QObject* parent = nullptr);

    /// @brief Destructor — Qt object tree cleans up child objects.
    ~AudioScene() override = default;

    // Disable copying — QObject-based classes are not copyable
    AudioScene(const AudioScene&) = delete;
    AudioScene& operator=(const AudioScene&) = delete;

    /// @brief Configure the audio engine, room, listener, and sound sources.
    /// @note  Must be called before any moveListener/moveSource calls.
    ///        Without a real audio file, sources won't produce sound,
    ///        but the scene geometry and attenuation model are fully configured.
    void setupScene();

    /// @brief Move the listener to a new position in 3D space.
    /// @param[in] pos New listener position (centimeters by default).
    void moveListener(const QVector3D& pos);

    /// @brief Move a specific sound source to a new position.
    /// @param[in] index Zero-based index into the source list.
    /// @param[in] pos   New source position (centimeters by default).
    /// @note  Asserts that index is within valid range.
    void moveSource(int index, const QVector3D& pos);

    /// @brief Print all scene configuration: engine settings, room properties,
    ///        listener position, and every source's position and attenuation model.
    void printSceneInfo() const;

    /// @brief Get the current listener position.
    /// @return Listener position as QVector3D.
    QVector3D listenerPosition() const;

    /// @brief Get the position of a specific sound source.
    /// @param[in] index Zero-based source index.
    /// @return Source position as QVector3D.
    QVector3D sourcePosition(int index) const;

    /// @brief Get the number of sound sources in the scene.
    /// @return Number of configured sources.
    int sourceCount() const;

private:
    /// @brief Create and configure the audio engine with output mode and distance scale.
    void createEngine();

    /// @brief Create a room with dimensions and wall materials for acoustic simulation.
    void createRoom();

    /// @brief Create the listener positioned at the origin.
    void createListener();

    /// @brief Create multiple spatial sound sources at different 3D positions
    ///        with varied distance attenuation models.
    void createSources();

    // Owned via Qt parent-child tree (m_engine owns all children)
    QAudioEngine* m_engine{nullptr};                       ///< Central 3D audio engine
    QAudioRoom* m_room{nullptr};                           ///< Acoustic room
    QAudioListener* m_listener{nullptr};                   ///< Listener (single per engine)
    std::vector<QSpatialSound*> m_sources;                 ///< Spatial sound sources

    static constexpr float kDistanceScaleCm{1.0f};         ///< Default: 1 unit = 1 cm
    static constexpr float kRoomWidth{1000.0f};            ///< Room width  (x) in cm (10 m)
    static constexpr float kRoomHeight{400.0f};            ///< Room height (y) in cm (4 m)
    static constexpr float kRoomDepth{1000.0f};            ///< Room depth  (z) in cm (10 m)
};
