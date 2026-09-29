/// @file    frame_processor.cpp
/// @brief   Implementation of FrameProcessor — test-pattern generation and
///          pixel-level image processing.

#include "frame_processor.h"

#include <QDebug>
#include <QImage>
#include <QRgb>

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

FrameProcessor::FrameProcessor(QObject* parent) : QObject(parent) {}

FrameProcessor::~FrameProcessor() = default;

// ---------------------------------------------------------------------------
// Test-pattern generator
// ---------------------------------------------------------------------------

QImage FrameProcessor::createTestPattern(int width, int height) const
{
    // Format_RGB32 stores 0xAARRGGBB natively; pixel manipulation is fast.
    QImage image(width, height, QImage::Format_RGB32);

    if (image.isNull()) {
        qWarning() << "Failed to allocate test pattern image.";
        return image;
    }

    // Upper half: classic colour bars (red, green, blue, cyan, magenta, yellow,
    // white) in equal vertical strips.
    static constexpr QRgb kColourBars[] = {
        qRgb(255, 0, 0),     // red
        qRgb(0, 255, 0),     // green
        qRgb(0, 0, 255),     // blue
        qRgb(0, 255, 255),   // cyan
        qRgb(255, 0, 255),   // magenta
        qRgb(255, 255, 0),   // yellow
        qRgb(255, 255, 255), // white
    };
    static constexpr int kBarCount = sizeof(kColourBars) / sizeof(kColourBars[0]);

    const int halfHeight = height / 2;
    const int barWidth   = width / kBarCount;

    for (int y = 0; y < halfHeight; ++y) {
        for (int x = 0; x < width; ++x) {
            const int barIndex = qBound(0, x / barWidth, kBarCount - 1);
            image.setPixel(x, y, kColourBars[barIndex]);
        }
    }

    // Lower half: horizontal greyscale gradient (black → white).
    for (int y = halfHeight; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int grey = static_cast<int>(
                (static_cast<qreal>(x) / static_cast<qreal>(width - 1)) * 255.0);
            image.setPixel(x, y, qRgb(grey, grey, grey));
        }
    }

    return image;
}

// ---------------------------------------------------------------------------
// Pixel manipulation utilities
// ---------------------------------------------------------------------------

QImage FrameProcessor::toGrayscale(const QImage& source)
{
    if (source.isNull()) {
        qWarning() << "toGrayscale: source image is null.";
        return QImage();
    }

    // Ensure we work with a known pixel format; convertInPlace would fail on
    // some formats, so we create a copy in RGB32 unconditionally.
    const QImage src =
        (source.format() == QImage::Format_RGB32) ? source : source.convertToFormat(QImage::Format_RGB32);

    QImage dest(src.size(), QImage::Format_RGB32);

    for (int y = 0; y < src.height(); ++y) {
        const QRgb* srcLine = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        QRgb* dstLine       = reinterpret_cast<QRgb*>(dest.scanLine(y));

        for (int x = 0; x < src.width(); ++x) {
            const QRgb pixel  = srcLine[x];
            // ITU-R BT.601 luminance — matches human brightness perception.
            const int gray = static_cast<int>(
                0.299 * qRed(pixel) + 0.587 * qGreen(pixel) + 0.114 * qBlue(pixel));
            const uchar clamped = static_cast<uchar>(qBound(0, gray, 255));
            dstLine[x]          = qRgb(clamped, clamped, clamped);
        }
    }

    return dest;
}

QImage FrameProcessor::adjustBrightness(const QImage& source, int factor)
{
    if (source.isNull()) {
        qWarning() << "adjustBrightness: source image is null.";
        return QImage();
    }

    const QImage src =
        (source.format() == QImage::Format_RGB32) ? source : source.convertToFormat(QImage::Format_RGB32);

    QImage dest(src.size(), QImage::Format_RGB32);

    for (int y = 0; y < src.height(); ++y) {
        const QRgb* srcLine = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        QRgb* dstLine       = reinterpret_cast<QRgb*>(dest.scanLine(y));

        for (int x = 0; x < src.width(); ++x) {
            const QRgb pixel = srcLine[x];
            // qBound clamps each channel independently to [0, 255].
            const int r = qBound(0, qRed(pixel) + factor, 255);
            const int g = qBound(0, qGreen(pixel) + factor, 255);
            const int b = qBound(0, qBlue(pixel) + factor, 255);
            dstLine[x]   = qRgb(r, g, b);
        }
    }

    return dest;
}

// ---------------------------------------------------------------------------
// Demo pipeline
// ---------------------------------------------------------------------------

void FrameProcessor::processAndReport() const
{
    static constexpr int kPatternWidth  = 320;
    static constexpr int kPatternHeight = 240;

    qDebug() << "\n=== Frame Processing Demo (software pipeline) ===\n";

    // Stage 1 — generate test pattern.
    const QImage pattern = createTestPattern(kPatternWidth, kPatternHeight);
    qDebug() << "Test pattern created:" << pattern.width() << "x" << pattern.height()
             << "format:" << pattern.format();

    // Sample the centre pixel of the red colour bar.
    const QRgb centrePixel = pattern.pixel(kPatternWidth / 2, kPatternHeight / 4);
    qDebug() << "  Centre of top half (colour bar):"
             << "R =" << qRed(centrePixel) << "G =" << qGreen(centrePixel)
             << "B =" << qBlue(centrePixel);

    // Stage 2 — grayscale conversion.
    const QImage gray = toGrayscale(pattern);
    const QRgb grayPixel = gray.pixel(kPatternWidth / 2, kPatternHeight / 4);
    qDebug() << "  After grayscale:"
             << "R =" << qRed(grayPixel) << "G =" << qGreen(grayPixel)
             << "B =" << qBlue(grayPixel);

    // Stage 3 — brightness adjustment (+80).
    static constexpr int kBrightnessDelta = 80;
    const QImage bright = adjustBrightness(gray, kBrightnessDelta);
    const QRgb brightPixel = bright.pixel(kPatternWidth / 2, kPatternHeight / 4);
    qDebug() << "  After brightness +" << kBrightnessDelta << ":"
             << "R =" << qRed(brightPixel) << "G =" << qGreen(brightPixel)
             << "B =" << qBlue(brightPixel);

    // Stage 4 — show how QVideoFrame would fit into a real pipeline.
    qDebug() << "\n  [Note] In a real application, QVideoSink delivers QVideoFrame";
    qDebug() << "  objects via the videoFrameChanged signal. You would convert with";
    qDebug() << "  QVideoFrame::toImage() and apply the same pixel operations shown";
    qDebug() << "  above before displaying or saving the result.";

    // Sample the greyscale gradient at 25 %, 50 %, 75 % width in the lower half.
    const int gradientY = (kPatternHeight / 2) + (kPatternHeight / 4);
    qDebug() << "\n  Greyscale gradient samples (lower half):";
    for (const int pct : {25, 50, 75}) {
        const int sx = (kPatternWidth * pct) / 100;
        const int val = qRed(pattern.pixel(sx, gradientY));
        qDebug().noquote() << QString("    %1% -> grey = %2").arg(pct).arg(val);
    }
}
