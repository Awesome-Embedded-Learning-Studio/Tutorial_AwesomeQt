/// @file    camera_info.cpp
/// @brief   Implementation of CameraInfo — camera device enumeration.

#include "camera_info.h"

#include <QDebug>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QSize>

CameraInfo::CameraInfo(QObject* parent) : QObject(parent)
{
    // QMediaDevices is not a singleton in Qt 6.x; we create an instance and
    // connect its signals to detect hot-plug events (e.g. USB camera inserted).
    auto* mediaDevices = new QMediaDevices(this);
    connect(mediaDevices, &QMediaDevices::videoInputsChanged, this,
            []() { qDebug() << "[CameraInfo] Video input devices changed."; });
}

CameraInfo::~CameraInfo() = default;

void CameraInfo::enumerateCameras() const
{
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

    if (cameras.isEmpty()) {
        qDebug() << "=== No camera devices found ===";
        qDebug() << "This is normal on headless / CI machines without a webcam.";
        return;
    }

    qDebug() << "=== Camera Devices (" << cameras.size() << ") ===";

    for (int i = 0; i < cameras.size(); ++i) {
        const QCameraDevice& dev = cameras.at(i);
        qDebug().noquote() << QString("  [%1] %2").arg(i).arg(dev.description());
        qDebug().noquote() << QString("      ID  : %1").arg(
            QString::fromUtf8(dev.id().toHex()));
        qDebug().noquote() << QString("      Pos : %1").arg(
            dev.position() == QCameraDevice::BackFace      ? "Back"
            : dev.position() == QCameraDevice::FrontFace   ? "Front"
                                                            : "Unspecified");

        // Photo resolutions — list the first few to avoid flooding the console.
        const QList<QSize> photoRes = dev.photoResolutions();
        if (!photoRes.isEmpty()) {
            const int showCount = qMin(photoRes.size(), 5);
            qDebug().noquote() << QString("      Photo resolutions (showing %1 of %2):")
                                      .arg(showCount)
                                      .arg(photoRes.size());
            for (int r = 0; r < showCount; ++r) {
                qDebug().noquote() << QString("        %1x%2").arg(
                    photoRes.at(r).width()).arg(photoRes.at(r).height());
            }
        }

        // Video formats — list a summary of unique resolutions.
        const QList<QCameraFormat> formats = dev.videoFormats();
        if (!formats.isEmpty()) {
            qDebug().noquote() << QString("      Video formats: %1").arg(formats.size());
            const int showFmt = qMin(formats.size(), 5);
            for (int f = 0; f < showFmt; ++f) {
                const QCameraFormat& fmt = formats.at(f);
                qDebug().noquote()
                    << QString("        %1x%2  %3-%4 FPS").arg(
                           fmt.resolution().width())
                        .arg(fmt.resolution().height())
                        .arg(fmt.minFrameRate(), 0, 'f', 1)
                        .arg(fmt.maxFrameRate(), 0, 'f', 1);
            }
        }
    }
}

QCameraDevice CameraInfo::getDefaultCamera() const
{
    return QMediaDevices::defaultVideoInput();
}

void CameraInfo::enumerateAudioInputs() const
{
    const QList<QAudioDevice> inputs = QMediaDevices::audioInputs();

    if (inputs.isEmpty()) {
        qDebug() << "=== No audio input devices found ===";
        return;
    }

    qDebug() << "=== Audio Input Devices (" << inputs.size() << ") ===";
    for (int i = 0; i < inputs.size(); ++i) {
        const QAudioDevice& dev = inputs.at(i);
        qDebug().noquote() << QString("  [%1] %2").arg(i).arg(dev.description());
        qDebug().noquote()
            << QString("      Mode: %1").arg(
                   dev.mode() == QAudioDevice::Input ? "Input" : "Output");
    }
}

void CameraInfo::enumerateAudioOutputs() const
{
    const QList<QAudioDevice> outputs = QMediaDevices::audioOutputs();

    if (outputs.isEmpty()) {
        qDebug() << "=== No audio output devices found ===";
        return;
    }

    qDebug() << "=== Audio Output Devices (" << outputs.size() << ") ===";
    for (int i = 0; i < outputs.size(); ++i) {
        const QAudioDevice& dev = outputs.at(i);
        qDebug().noquote() << QString("  [%1] %2").arg(i).arg(dev.description());
    }
}

QString CameraInfo::formatResolution(const QSize& resolution)
{
    if (resolution.isValid()) {
        return QString("%1x%2").arg(resolution.width()).arg(resolution.height());
    }
    return QStringLiteral("N/A");
}
