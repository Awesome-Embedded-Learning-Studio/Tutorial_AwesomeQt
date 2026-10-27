/// @file    camera_info.h
/// @brief   Camera device enumeration using QMediaDevices.
///
/// Demonstrates how to query available camera, audio input, and audio output
/// devices at runtime without requiring actual camera hardware to be present.
/// Corresponding tutorial: advanced layer 05-Other-Modules/05-QtMultimedia.

#pragma once

#include <QCameraDevice>
#include <QList>
#include <QObject>
#include <QString>

class QMediaDevices;

/// @brief Enumerates and reports all available media devices on the system.
///
/// Wraps QMediaDevices to provide a structured overview of cameras and audio
/// devices. Designed as a console-friendly utility — all output goes to
/// qDebug(), making it suitable for headless demos and CI environments.
class CameraInfo : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the enumerator and connects to QMediaDevices signals.
    /// @param[in] parent  Parent QObject for ownership management.
    /// @note The constructor connects to QMediaDevices::videoInputsChanged so
    ///       that hot-plugged cameras are reported automatically.
    explicit CameraInfo(QObject* parent = nullptr);

    /// @brief Destructor.
    ~CameraInfo() override;

    /// @brief Prints detailed information about every camera found on the system.
    ///
    /// For each camera the following fields are logged: description, device ID,
    /// default photo resolution, and default video resolution.
    void enumerateCameras() const;

    /// @brief Returns the system default camera device, or an invalid
    ///        QCameraDevice if none is available.
    /// @return The default QCameraDevice.
    /// @note Callers must check isValid() on the returned object before use.
    QCameraDevice getDefaultCamera() const;

    /// @brief Lists all available audio input devices (microphones).
    void enumerateAudioInputs() const;

    /// @brief Lists all available audio output devices (speakers/headphones).
    void enumerateAudioOutputs() const;

private:
    /// @brief Formats a resolution as "WxH" or "N/A" if invalid.
    /// @param[in] resolution  The QSize to format.
    /// @return Human-readable string for console output.
    static QString formatResolution(const QSize& resolution);
};
