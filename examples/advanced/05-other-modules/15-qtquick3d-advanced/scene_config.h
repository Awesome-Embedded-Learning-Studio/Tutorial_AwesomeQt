/// @file    scene_config.h
/// @brief   C++ backend for Qt Quick 3D PBR material and scene configuration.
///
/// Exposes metalness, roughness, and baseColor as QML-bindable properties
/// so that the QML scene can reactively update PrincipledMaterial parameters.

#pragma once

#include <QObject>
#include <QColor>
#include <QtQmlIntegration/qqmlintegration.h>

class SceneConfig : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal metalness READ metalness WRITE setMetalness NOTIFY metalnessChanged)
    Q_PROPERTY(qreal roughness READ roughness WRITE setRoughness NOTIFY roughnessChanged)
    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)

public:
    /// @brief Constructs a SceneConfig with default PBR values.
    /// @param[in] parent Parent QObject for ownership via Qt object tree.
    /// @note Default values approximate a polished steel look:
    ///       metalness=0.8, roughness=0.3, baseColor=steel blue.
    explicit SceneConfig(QObject* parent = nullptr);

    /// @brief Returns the current metalness factor [0.0, 1.0].
    qreal metalness() const;

    /// @brief Returns the current roughness factor [0.0, 1.0].
    qreal roughness() const;

    /// @brief Returns the current base color for the PBR material.
    QColor baseColor() const;

public slots:
    /// @brief Sets the metalness factor.
    /// @param[in] value New metalness in [0.0, 1.0].
    void setMetalness(qreal value);

    /// @brief Sets the roughness factor.
    /// @param[in] value New roughness in [0.0, 1.0].
    void setRoughness(qreal value);

    /// @brief Sets the base color for the PBR material.
    /// @param[in] color New base color.
    void setBaseColor(const QColor& color);

signals:
    /// @brief Emitted when metalness changes.
    void metalnessChanged();

    /// @brief Emitted when roughness changes.
    void roughnessChanged();

    /// @brief Emitted when baseColor changes.
    void baseColorChanged();

private:
    qreal m_metalness;   ///< PBR metalness factor
    qreal m_roughness;   ///< PBR roughness factor
    QColor m_baseColor;  ///< PBR base albedo color
};
