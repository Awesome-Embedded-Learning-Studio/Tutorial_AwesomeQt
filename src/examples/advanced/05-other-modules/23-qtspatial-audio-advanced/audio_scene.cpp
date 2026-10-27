/// @file    audio_scene.cpp
/// @brief   Implementation of the AudioScene spatial audio manager.
///
/// Configures QAudioEngine, QAudioRoom, QAudioListener, and QSpatialSound
/// objects to demonstrate 3D spatial audio with distance attenuation and
/// room acoustics using the Qt Spatial Audio module.

#include "audio_scene.h"

#include <QAudioEngine>
#include <QAudioListener>
#include <QAudioRoom>
#include <QDebug>
#include <QSpatialSound>

#include <cassert>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

AudioScene::AudioScene(QObject* parent)
    : QObject(parent)
{
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void AudioScene::setupScene()
{
    createEngine();
    createRoom();
    createListener();
    createSources();

    // Enable room effects (reflections + reverb) so the room actually matters
    m_engine->setRoomEffectsEnabled(true);

    qDebug() << "[AudioScene] Scene setup complete.";
}

void AudioScene::moveListener(const QVector3D& pos)
{
    if (m_listener == nullptr) {
        qWarning() << "[AudioScene] Listener not created yet. Call setupScene() first.";
        return;
    }

    m_listener->setPosition(pos);
    qDebug() << "[AudioScene] Listener moved to:" << pos;
}

void AudioScene::moveSource(int index, const QVector3D& pos)
{
    assert(index >= 0 && static_cast<std::size_t>(index) < m_sources.size()
           && "Source index out of range");

    m_sources[static_cast<std::size_t>(index)]->setPosition(pos);
    qDebug() << "[AudioScene] Source" << index << "moved to:" << pos;
}

void AudioScene::printSceneInfo() const
{
    qDebug() << "\n========== Spatial Audio Scene Info ==========";

    // --- Engine ---
    qDebug() << "\n--- Engine ---";
    qDebug() << "  Sample rate      :" << m_engine->sampleRate();
    qDebug() << "  Distance scale   :" << m_engine->distanceScale();
    qDebug() << "  Master volume    :" << m_engine->masterVolume();
    qDebug() << "  Output mode      :"
             << (m_engine->outputMode() == QAudioEngine::Headphone  ? "Headphone"
                 : m_engine->outputMode() == QAudioEngine::Stereo   ? "Stereo"
                                                                     : "Surround");
    qDebug() << "  Room effects     :" << (m_engine->roomEffectsEnabled() ? "ON" : "OFF");

    // --- Room ---
    qDebug() << "\n--- Room ---";
    qDebug() << "  Position (center):" << m_room->position();
    qDebug() << "  Dimensions       :" << m_room->dimensions();
    qDebug() << "  Reflection gain  :" << m_room->reflectionGain();
    qDebug() << "  Reverb gain      :" << m_room->reverbGain();
    qDebug() << "  Reverb time      :" << m_room->reverbTime();
    qDebug() << "  Reverb brightness:" << m_room->reverbBrightness();

    // Wall materials
    const auto wallNames = std::vector<std::pair<QAudioRoom::Wall, const char*>>{
        {QAudioRoom::LeftWall,  "Left wall "},
        {QAudioRoom::RightWall, "Right wall"},
        {QAudioRoom::Floor,     "Floor     "},
        {QAudioRoom::Ceiling,   "Ceiling   "},
        {QAudioRoom::FrontWall, "Front wall"},
        {QAudioRoom::BackWall,  "Back wall "}
    };

    for (const auto& [wall, name] : wallNames) {
        const auto material = m_room->wallMaterial(wall);
        qDebug() << "  " << name << "material:" << static_cast<int>(material);
    }

    // --- Listener ---
    qDebug() << "\n--- Listener ---";
    qDebug() << "  Position         :" << m_listener->position();

    // --- Sources ---
    qDebug() << "\n--- Sound Sources (" << m_sources.size() << ") ---";

    const auto modelToString = [](QSpatialSound::DistanceModel model) {
        switch (model) {
        case QSpatialSound::DistanceModel::Logarithmic:
            return "Logarithmic";
        case QSpatialSound::DistanceModel::Linear:
            return "Linear";
        case QSpatialSound::DistanceModel::ManualAttenuation:
            return "Manual";
        }
        return "Unknown";
    };

    for (std::size_t i = 0; i < m_sources.size(); ++i) {
        const auto* src = m_sources[i];
        qDebug() << "  Source" << i << ":";
        qDebug() << "    Position       :" << src->position();
        qDebug() << "    Volume         :" << src->volume();
        qDebug() << "    Distance model :" << modelToString(src->distanceModel());
        qDebug() << "    Distance cutoff:" << src->distanceCutoff();
        qDebug() << "    Size           :" << src->size();
        qDebug() << "    Directivity    :" << src->directivity();
        qDebug() << "    Occlusion      :" << src->occlusionIntensity();
    }

    qDebug() << "\n===============================================\n";
}

QVector3D AudioScene::listenerPosition() const
{
    return (m_listener != nullptr) ? m_listener->position() : QVector3D{};
}

QVector3D AudioScene::sourcePosition(int index) const
{
    assert(index >= 0 && static_cast<std::size_t>(index) < m_sources.size()
           && "Source index out of range");
    return m_sources[static_cast<std::size_t>(index)]->position();
}

int AudioScene::sourceCount() const
{
    return static_cast<int>(m_sources.size());
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void AudioScene::createEngine()
{
    // Parent to this AudioScene so Qt object tree handles cleanup
    m_engine = new QAudioEngine(this);

    // Use headphone mode for best 3D spatialization (HRTF)
    m_engine->setOutputMode(QAudioEngine::Headphone);

    // Keep default distance scale (1 unit = 1 cm), matching Qt Quick 3D convention
    m_engine->setDistanceScale(kDistanceScaleCm);

    // Reasonable master volume
    m_engine->setMasterVolume(0.8f);

    qDebug() << "[AudioScene] Engine created (sample rate:" << m_engine->sampleRate()
             << "Hz)";
}

void AudioScene::createRoom()
{
    m_room = new QAudioRoom(m_engine);

    // Room centered at origin
    m_room->setPosition(QVector3D(0.0f, 0.0f, 0.0f));

    // 10m x 4m x 10m room (in cm)
    m_room->setDimensions(QVector3D(kRoomWidth, kRoomHeight, kRoomDepth));

    // Acoustic properties — simulate a medium-sized conference room
    m_room->setReflectionGain(0.7f);      // Moderate reflections
    m_room->setReverbGain(0.5f);          // Moderate reverb
    m_room->setReverbTime(1.2f);          // Slightly longer reverb (sounds larger)
    m_room->setReverbBrightness(0.1f);    // Slightly bright reverb

    // Wall materials — mix to simulate a realistic conference room
    m_room->setWallMaterial(QAudioRoom::LeftWall,   QAudioRoom::PlasterSmooth);
    m_room->setWallMaterial(QAudioRoom::RightWall,  QAudioRoom::PlasterSmooth);
    m_room->setWallMaterial(QAudioRoom::Floor,      QAudioRoom::ParquetOnConcrete);
    m_room->setWallMaterial(QAudioRoom::Ceiling,    QAudioRoom::AcousticCeilingTiles);
    m_room->setWallMaterial(QAudioRoom::FrontWall,  QAudioRoom::GlassThick);
    m_room->setWallMaterial(QAudioRoom::BackWall,   QAudioRoom::WoodPanel);

    qDebug() << "[AudioScene] Room created ("
             << kRoomWidth / 100.0f << "m x"
             << kRoomHeight / 100.0f << "m x"
             << kRoomDepth / 100.0f << "m)";
}

void AudioScene::createListener()
{
    m_listener = new QAudioListener(m_engine);

    // Listener at the origin, facing forward (default orientation)
    m_listener->setPosition(QVector3D(0.0f, 0.0f, 0.0f));

    qDebug() << "[AudioScene] Listener created at origin.";
}

void AudioScene::createSources()
{
    // Source 0 — nearby, in front of listener (simulates a speaker)
    //   Using logarithmic attenuation for natural-sounding distance falloff
    auto* source0 = new QSpatialSound(m_engine);
    source0->setPosition(QVector3D(0.0f, 100.0f, -200.0f));  // 2m in front
    source0->setDistanceModel(QSpatialSound::DistanceModel::Logarithmic);
    source0->setDistanceCutoff(1000.0f);    // Cutoff at 10m
    source0->setSize(50.0f);                // 0.5m radius — constant volume inside
    source0->setVolume(1.0f);
    source0->setDirectivity(0.0f);          // Omnidirectional
    m_sources.push_back(source0);

    // Source 1 — far left (simulates ambient sound)
    //   Using linear attenuation for predictable volume decrease
    auto* source1 = new QSpatialSound(m_engine);
    source1->setPosition(QVector3D(-400.0f, 150.0f, 0.0f));  // 4m left, 1.5m up
    source1->setDistanceModel(QSpatialSound::DistanceModel::Linear);
    source1->setDistanceCutoff(800.0f);
    source1->setSize(100.0f);
    source1->setVolume(0.8f);
    source1->setDirectivity(0.5f);          // Partially directional
    source1->setDirectivityOrder(2.0f);     // Sharper cone
    m_sources.push_back(source1);

    // Source 2 — behind and above the listener (simulates overhead source)
    //   Using manual attenuation for fixed volume control
    auto* source2 = new QSpatialSound(m_engine);
    source2->setPosition(QVector3D(100.0f, 300.0f, 300.0f));  // Behind-right, 3m up
    source2->setDistanceModel(QSpatialSound::DistanceModel::ManualAttenuation);
    source2->setManualAttenuation(0.4f);    // Fixed 40% attenuation
    source2->setDistanceCutoff(1500.0f);
    source2->setSize(80.0f);
    source2->setVolume(0.6f);
    source2->setOcclusionIntensity(0.3f);   // Partially occluded
    m_sources.push_back(source2);

    // Source 3 — right side, close to listener (simulates a nearby object)
    //   Demonstrates near-field gain effect
    auto* source3 = new QSpatialSound(m_engine);
    source3->setPosition(QVector3D(150.0f, 100.0f, -50.0f));  // 1.5m right
    source3->setDistanceModel(QSpatialSound::DistanceModel::Logarithmic);
    source3->setDistanceCutoff(500.0f);
    source3->setSize(30.0f);
    source3->setVolume(1.0f);
    source3->setNearFieldGain(0.7f);        // Boost when very close
    m_sources.push_back(source3);

    // Source 4 — far back (simulates a distant event)
    //   Logarithmic model with large cutoff to show distance range
    auto* source4 = new QSpatialSound(m_engine);
    source4->setPosition(QVector3D(0.0f, 200.0f, 800.0f));  // 8m behind
    source4->setDistanceModel(QSpatialSound::DistanceModel::Logarithmic);
    source4->setDistanceCutoff(2000.0f);    // Cutoff at 20m
    source4->setSize(200.0f);               // Large source
    source4->setVolume(0.5f);
    m_sources.push_back(source4);

    qDebug() << "[AudioScene]" << m_sources.size()
             << "spatial sound sources created.";
}
