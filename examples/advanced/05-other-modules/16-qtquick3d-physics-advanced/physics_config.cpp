/// @file    physics_config.cpp
/// @brief   Implementation of PhysicsConfig — C++ backend for physics parameters.
///
/// Provides default values and property setters with change notification.
/// The applyForce() method emits a signal that the QML physics simulation
/// listens to for applying impulses to dynamic bodies.

#include "physics_config.h"

// Earth standard gravity in m/s^2 (negative = downward direction)
static constexpr qreal kDefaultGravity = -9.81;

// Moderate impulse strength — large enough for visible motion in the scene
static constexpr qreal kDefaultForceStrength = 100.0;

PhysicsConfig::PhysicsConfig(QObject* parent)
    : QObject(parent)
    , m_gravity(kDefaultGravity)
    , m_forceStrength(kDefaultForceStrength)
{
}

qreal PhysicsConfig::gravity() const
{
    return m_gravity;
}

qreal PhysicsConfig::forceStrength() const
{
    return m_forceStrength;
}

void PhysicsConfig::applyForce()
{
    // Simply notify the QML layer — actual velocity changes happen in QML
    // where direct access to body transforms is available.
    emit forceApplied();
}

void PhysicsConfig::setGravity(qreal value)
{
    if (qFuzzyCompare(m_gravity, value)) {
        return;
    }
    m_gravity = value;
    emit gravityChanged();
}

void PhysicsConfig::setForceStrength(qreal value)
{
    if (qFuzzyCompare(m_forceStrength, value)) {
        return;
    }
    m_forceStrength = value;
    emit forceStrengthChanged();
}
