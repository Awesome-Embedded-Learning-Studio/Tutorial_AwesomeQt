/// @file    main.cpp
/// @brief   Console demo for Qt Spatial Audio scene configuration.
///
/// Creates an AudioScene, demonstrates distance attenuation concepts,
/// moves the listener and sources, and prints scene info. Actual audio
/// playback may not work in WSL2 without an audio device — compilation
/// is the primary goal.

#include "audio_scene.h"

#include <QCoreApplication>
#include <QTimer>
#include <QVector3D>

#include <cmath>
#include <cstdio>

// ---------------------------------------------------------------------------
// Helper: compute a simple logarithmic attenuation factor for demo purposes.
// Qt Spatial Audio uses its own internal HRTF-based attenuation; this function
// provides a rough approximation to illustrate the concept.
// ---------------------------------------------------------------------------
static float approximateLogAttenuation(float distance, float size, float cutoff)
{
    if (distance <= size) {
        return 1.0f;    // Inside source radius — full volume
    }
    if (distance >= cutoff) {
        return 0.0f;    // Beyond cutoff — silent
    }
    // Logarithmic falloff between size and cutoff
    const float normalizedDist = (distance - size) / (cutoff - size);
    return 1.0f - std::log1p(normalizedDist) / std::log1p(1.0f);
}

// ---------------------------------------------------------------------------
// Demo 1: Setup scene and print the full configuration.
// ---------------------------------------------------------------------------
static void demoSetupAndPrint(AudioScene& scene)
{
    qDebug() << "\n===== DEMO 1: Scene Setup & Configuration =====\n";
    scene.setupScene();
    scene.printSceneInfo();
}

// ---------------------------------------------------------------------------
// Demo 2: Show approximate distance attenuation for each source.
// ---------------------------------------------------------------------------
static void demoDistanceAttenuation(const AudioScene& scene)
{
    qDebug() << "\n===== DEMO 2: Distance Attenuation =====\n";
    qDebug() << "Listener is at origin (0, 0, 0).";
    qDebug() << "Approximate logarithmic attenuation from listener to each source:\n";

    const QVector3D listenerPos{0.0f, 0.0f, 0.0f};

    for (int i = 0; i < scene.sourceCount(); ++i) {
        const QVector3D srcPos = scene.sourcePosition(i);
        const float distance = listenerPos.distanceToPoint(srcPos);
        // Use rough cutoff of 1500 cm for demonstration
        const float atten = approximateLogAttenuation(distance, 50.0f, 1500.0f);

        qDebug() << "  Source" << i << "at" << srcPos
                 << "-> distance:" << QString::number(distance, 'f', 1) << "cm"
                 << "~ attenuation:" << QString::number(atten, 'f', 3)
                 << "(~" << QString::number(atten * 100.0f, 'f', 0) + "%)";
    }
    qDebug() << "";
}

// ---------------------------------------------------------------------------
// Demo 3: Move the listener and show how effective distances change.
// ---------------------------------------------------------------------------
static void demoMoveListener(AudioScene& scene)
{
    qDebug() << "\n===== DEMO 3: Move Listener =====\n";

    // Move listener closer to source 1 (far left)
    const QVector3D newPos{-300.0f, 100.0f, 0.0f};
    scene.moveListener(newPos);
    qDebug() << "Listener moved to" << newPos;
    qDebug() << "Effective distances after move:\n";

    for (int i = 0; i < scene.sourceCount(); ++i) {
        const QVector3D srcPos = scene.sourcePosition(i);
        const float distance = newPos.distanceToPoint(srcPos);
        const float atten = approximateLogAttenuation(distance, 50.0f, 1500.0f);

        qDebug() << "  Source" << i << "-> distance:" << QString::number(distance, 'f', 1)
                 << "cm, ~attenuation:" << QString::number(atten, 'f', 3);
    }

    // Move source 3 to a new position
    const QVector3D newSrcPos{-200.0f, 100.0f, -100.0f};
    scene.moveSource(3, newSrcPos);

    const float newDist = newPos.distanceToPoint(newSrcPos);
    const float newAtten = approximateLogAttenuation(newDist, 50.0f, 1500.0f);

    qDebug() << "\n  Source 3 moved to" << newSrcPos
             << "-> distance:" << QString::number(newDist, 'f', 1) << "cm"
             << ", ~attenuation:" << QString::number(newAtten, 'f', 3);
    qDebug() << "";
}

// ---------------------------------------------------------------------------
// Main entry point
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt Spatial Audio — Advanced Example";
    qDebug() << "====================================";
    qDebug() << "Note: Actual audio playback requires an audio device.";
    qDebug() << "In WSL2 this may not be available — compilation is the goal.\n";

    AudioScene scene;

    // Run all demos sequentially
    demoSetupAndPrint(scene);
    demoDistanceAttenuation(scene);
    demoMoveListener(scene);

    qDebug() << "All demos complete. Auto-quitting in 1 second...";

    // Auto-quit after a short delay so the event loop processes any pending events
    QTimer::singleShot(1000, &app, &QCoreApplication::quit);

    return app.exec();
}
