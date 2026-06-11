/// @file    frame_processor.h
/// @brief   Software-only frame processing pipeline (no camera required).
///
/// Provides static image processing utilities that demonstrate the same pixel
/// manipulation patterns used in a real QVideoFrame / QVideoSink pipeline.
/// All input comes from generated test-pattern images, so the demo runs on any
/// machine without camera hardware.
/// Corresponding tutorial: advanced layer 05-Other-Modules/05-QtMultimedia.

#pragma once

#include <QImage>
#include <QObject>

/// @brief Simulates the image-processing stage of a video capture pipeline.
///
/// In a real application, QVideoSink::videoFrameChanged delivers QVideoFrame
/// objects that can be converted to QImage for analysis. This class replaces
/// that camera source with a programmatically generated test pattern so the
/// processing logic can be demonstrated and tested without hardware.
class FrameProcessor : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the processor.
    /// @param[in] parent  Parent QObject for ownership management.
    explicit FrameProcessor(QObject* parent = nullptr);

    /// @brief Destructor.
    ~FrameProcessor() override;

    /// @brief Generates a colour test-pattern image (colour bars + gradients).
    /// @param[in] width   Desired image width in pixels.
    /// @param[in] height  Desired image height in pixels.
    /// @return A QImage in Format_RGB32.
    /// @note The test pattern is deliberately rich in colour variety so that
    ///       grayscale conversion and brightness adjustment produce visible,
    ///       easy-to-verify results.
    QImage createTestPattern(int width, int height) const;

    /// @brief Converts an image to grayscale using the luminance formula.
    /// @param[in] source  The input image (any format; converted to RGB32 first).
    /// @return A new QImage in Format_RGB32 where each pixel is its luminance.
    /// @note Uses the ITU-R BT.601 luma coefficients (0.299R + 0.587G + 0.114B)
    ///       which matches how Qt's own grayscale conversion works internally.
    static QImage toGrayscale(const QImage& source);

    /// @brief Adjusts the brightness of every pixel by a constant offset.
    /// @param[in] source  The input image.
    /// @param[in] factor  Brightness delta in [-255, +255]. Positive is brighter.
    /// @return A new QImage with adjusted brightness, clamped to [0, 255].
    /// @note Pixel values are clamped with qBound to avoid overflow artifacts.
    static QImage adjustBrightness(const QImage& source, int factor);

    /// @brief Runs the full demo pipeline: create pattern → grayscale → brighten.
    ///
    /// Prints pixel sample statistics at each stage so the console output
    /// is self-explanatory even without saving images to disk.
    void processAndReport() const;
};
