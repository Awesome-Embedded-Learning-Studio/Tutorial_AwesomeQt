/// @file    physics_config.h
/// @brief   C++ backend for Qt Quick 3D physics simulation parameters.
///
/// Exposes gravity, force strength, and a trigger mechanism so the QML
/// scene can apply impulse forces to dynamic bodies. Designed as a QML
/// element for seamless property binding from the QML layer.

#pragma once

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

/// @brief Manages physics simulation parameters exposed to QML.
///
/// PhysicsConfig provides gravity, force strength, and an invocable
/// method to trigger impulse application. The QML layer binds to these
/// properties and reacts to the applyForce signal.
class PhysicsConfig : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qreal gravity READ gravity WRITE setGravity NOTIFY gravityChanged)
    Q_PROPERTY(qreal forceStrength READ forceStrength WRITE setForceStrength
                   NOTIFY forceStrengthChanged)

public:
    /// @brief Constructs a PhysicsConfig with default physics values.
    /// @param[in] parent Parent QObject for ownership via Qt object tree.
    /// @note Defaults approximate Earth gravity (-9.81 m/s^2) and a
    ///       moderate impulse strength (100 N) for visible motion.
    explicit PhysicsConfig(QObject* parent = nullptr);

    /// @brief Returns the current gravity acceleration (m/s^2).
    qreal gravity() const;

    /// @brief Returns the current impulse force strength.
    qreal forceStrength() const;

    /// @brief Triggers an impulse force application on dynamic bodies.
    /// @note Emits forceApplied() so the QML layer can react by applying
    ///       velocity changes to the simulated bodies.
    Q_INVOKABLE void applyForce();

public slots:
    /// @brief Sets the gravity acceleration value.
    /// @param[in] value New gravity in m/s^2 (negative = downward).
    void setGravity(qreal value);

    /// @brief Sets the impulse force strength.
    /// @param[in] value New force magnitude (arbitrary units).
    void setForceStrength(qreal value);

signals:
    /// @brief Emitted when the gravity value changes.
    void gravityChanged();

    /// @brief Emitted when the force strength value changes.
    void forceStrengthChanged();

    /// @brief Emitted when applyForce() is called.
    /// @note QML handlers receive this signal and apply the impulse to
    ///       each dynamic body using the current forceStrength.
    void forceApplied();

private:
    qreal m_gravity;        ///< Gravity acceleration in m/s^2
    qreal m_forceStrength;  ///< Impulse force magnitude
};
