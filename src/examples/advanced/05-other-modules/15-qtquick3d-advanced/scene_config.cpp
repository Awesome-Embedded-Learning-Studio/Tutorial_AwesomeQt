/// @file    scene_config.cpp
/// @brief   Implementation of SceneConfig — C++ backend for PBR material properties.
///
/// Provides clamped property setters so QML bindings always receive valid
/// values within the [0.0, 1.0] range expected by PrincipledMaterial.

#include "scene_config.h"

#include <QColor>

// Steel blue — a recognizable default that contrasts well with 3D lighting
static constexpr qreal kDefaultMetalness = 0.8;
static constexpr qreal kDefaultRoughness = 0.3;

SceneConfig::SceneConfig(QObject* parent)
    : QObject(parent)
    , m_metalness(kDefaultMetalness)
    , m_roughness(kDefaultRoughness)
    , m_baseColor(70, 130, 180)  // steel blue
{
}

qreal SceneConfig::metalness() const
{
    return m_metalness;
}

qreal SceneConfig::roughness() const
{
    return m_roughness;
}

QColor SceneConfig::baseColor() const
{
    return m_baseColor;
}

void SceneConfig::setMetalness(qreal value)
{
    // Clamp to [0.0, 1.0] — values outside this range are meaningless for PBR
    const qreal clamped = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(m_metalness, clamped)) {
        return;
    }
    m_metalness = clamped;
    emit metalnessChanged();
}

void SceneConfig::setRoughness(qreal value)
{
    const qreal clamped = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(m_roughness, clamped)) {
        return;
    }
    m_roughness = clamped;
    emit roughnessChanged();
}

void SceneConfig::setBaseColor(const QColor& color)
{
    if (m_baseColor == color) {
        return;
    }
    m_baseColor = color;
    emit baseColorChanged();
}
