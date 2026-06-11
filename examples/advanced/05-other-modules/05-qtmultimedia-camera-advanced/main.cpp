/// @file    main.cpp
/// @brief   Entry point for the QVideoSink / camera enumeration demo.
///
/// Runs three console-based demos and then quits via a zero-length QTimer:
///   1. Enumerate all camera and audio devices on the system.
///   2. Generate a test-pattern image and run pixel-processing pipeline.
///   3. Print default camera capabilities (if any camera is present).
///
/// No camera hardware is required — the frame-processing demo uses a
/// software-generated test pattern.
/// Corresponding tutorial: advanced layer 05-Other-Modules/05-QtMultimedia.

#include "camera_info.h"
#include "frame_processor.h"

#include <QDebug>
#include <QTimer>

#include <QCoreApplication>

/// @brief Runs all demo stages sequentially, then schedules application exit.
/// @note Using a single-shot QTimer with interval 0 ensures the event loop
///       starts (required by QMediaDevices) and then quits cleanly.
static void runDemos()
{
    // ---- Demo 1: Device enumeration ----
    qDebug() << "========================================";
    qDebug() << " Demo 1: Camera & Audio Device Enumeration";
    qDebug() << "========================================\n";

    CameraInfo cameraInfo;
    cameraInfo.enumerateCameras();
    cameraInfo.enumerateAudioInputs();
    cameraInfo.enumerateAudioOutputs();

    // ---- Demo 2: Frame processing pipeline ----
    qDebug() << "\n========================================";
    qDebug() << " Demo 2: Frame Processing Pipeline (software)";
    qDebug() << "========================================";

    FrameProcessor processor;
    processor.processAndReport();

    // ---- Demo 3: Default camera capabilities ----
    qDebug() << "\n========================================";
    qDebug() << " Demo 3: Default Camera Capabilities";
    qDebug() << "========================================\n";

    const QCameraDevice defaultCam = cameraInfo.getDefaultCamera();
    if (!defaultCam.isNull()) {
        qDebug() << "Default camera:" << defaultCam.description();
        qDebug() << "  ID  :" << QString::fromUtf8(defaultCam.id().toHex());
        qDebug() << "  Pos :" << (defaultCam.isDefault() ? "Default" : "Non-default");

        // List all supported photo resolutions.
        const QList<QSize> photoRes = defaultCam.photoResolutions();
        if (!photoRes.isEmpty()) {
            qDebug() << "  Supported photo resolutions:" << photoRes.size();
            for (const QSize& sz : photoRes) {
                qDebug().noquote() << QString("    %1x%2").arg(sz.width()).arg(sz.height());
            }
        }

        // List all supported video formats (QCameraFormat, not QVideoFrameFormat).
        const QList<QCameraFormat> videoFormats = defaultCam.videoFormats();
        if (!videoFormats.isEmpty()) {
            qDebug() << "  Supported video formats:" << videoFormats.size();
            const int showFmt = qMin(videoFormats.size(), 8);
            for (int i = 0; i < showFmt; ++i) {
                const QCameraFormat& fmt = videoFormats.at(i);
                const QSize res = fmt.resolution();
                qDebug().noquote()
                    << QString("    %1x%2  %3-%4 FPS").arg(
                           QString::number(res.width()),
                           QString::number(res.height()),
                           QString::number(fmt.minFrameRate(), 'f', 1),
                           QString::number(fmt.maxFrameRate(), 'f', 1));
            }
        }
    } else {
        qDebug() << "No default camera available.";
        qDebug() << "On a machine without a webcam this is expected and harmless.";
        qDebug() << "The frame-processing demo above still runs successfully.";
    }

    // Schedule quit — give the event loop one more spin so queued signals drain.
    QTimer::singleShot(0, qApp, &QCoreApplication::quit);
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // Invoke the demo function once the event loop is running.
    // QTimer with interval 0 is the idiomatic way to "run code after app.exec()"
    // without blocking startup.
    QTimer::singleShot(0, &app, &runDemos);

    return app.exec();
}
