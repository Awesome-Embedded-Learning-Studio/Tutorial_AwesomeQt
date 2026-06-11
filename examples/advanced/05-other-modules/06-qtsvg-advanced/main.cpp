/// @file    main.cpp
/// @brief   Console demo for SVG DOM manipulation via SvgController.
///
/// Runs four sequential demos: creating a sample SVG, changing element
/// colors, toggling visibility, and updating text content. The final
/// SVG is saved to a temporary file.
/// Corresponds to: advanced layer 05-Other-Modules/06-QtSvg.

#include "svg_controller.h"

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryFile>
#include <QTextStream>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// @brief Prints a section separator with a demo label to stdout.
/// @param[in] title  The demo title to display.
static void printHeader(const QString& title)
{
    QTextStream out(stdout);
    out << "\n========================================\n"
        << title
        << "\n========================================\n";
}

/// @brief Prints the full SVG document content to stdout.
/// @param[in] controller  The controller whose SVG content is printed.
static void printSvg(const SvgController& controller)
{
    QTextStream out(stdout);
    out << controller.toByteArray() << "\n";
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

/// @brief Application entry point — runs all SVG manipulation demos.
/// @param[in] argc  Argument count.
/// @param[in] argv  Argument values.
/// @return Exit code (0 on success).
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    SvgController controller;

    // ---- Demo 1: Create and print the sample SVG ----
    printHeader(QStringLiteral("Demo 1: Create sample SVG"));
    controller.createSampleSvg();
    printSvg(controller);

    // ---- Demo 2: Change the circle color from blue to red ----
    printHeader(QStringLiteral("Demo 2: Change circle color (blue -> red)"));
    bool ok = controller.setElementColor(
        QStringLiteral("demo-circle"), QStringLiteral("red"));
    QTextStream(stdout) << "setElementColor result: "
                        << (ok ? "success" : "element not found") << "\n";
    printSvg(controller);

    // ---- Demo 3: Toggle rectangle visibility (hide it) ----
    printHeader(QStringLiteral("Demo 3: Hide rectangle"));
    ok = controller.setElementVisibility(
        QStringLiteral("demo-rect"), false);
    QTextStream(stdout) << "setElementVisibility(hide) result: "
                        << (ok ? "success" : "element not found") << "\n";
    printSvg(controller);

    // ---- Demo 3b: Toggle rectangle visibility (show it again) ----
    printHeader(QStringLiteral("Demo 3b: Show rectangle again"));
    ok = controller.setElementVisibility(
        QStringLiteral("demo-rect"), true);
    QTextStream(stdout) << "setElementVisibility(show) result: "
                        << (ok ? "success" : "element not found") << "\n";
    printSvg(controller);

    // ---- Demo 4: Update text content ----
    printHeader(QStringLiteral("Demo 4: Update text content"));
    ok = controller.setElementText(
        QStringLiteral("demo-text"), QStringLiteral("Modified SVG!"));
    QTextStream(stdout) << "setElementText result: "
                        << (ok ? "success" : "element not found") << "\n";
    printSvg(controller);

    // ---- Demo 5: Set a generic attribute (resize circle radius) ----
    printHeader(QStringLiteral("Demo 5: Set circle radius to 25"));
    ok = controller.setElementAttribute(
        QStringLiteral("demo-circle"), QStringLiteral("r"),
        QStringLiteral("25"));
    QTextStream(stdout) << "setElementAttribute result: "
                        << (ok ? "success" : "element not found") << "\n";
    printSvg(controller);

    // ---- Save final SVG to a temporary file ----
    printHeader(QStringLiteral("Save final SVG to temp file"));
    QTemporaryFile tmpFile(
        QDir::tempPath() + QStringLiteral("/qtsvg-advanced-XXXXXX.svg"));
    if (tmpFile.open()) {
        // QTemporaryFile::fileName() is only valid while the file is open,
        // so capture the name before closing
        QString tmpPath = tmpFile.fileName();
        tmpFile.close();
        if (controller.saveToFile(tmpPath)) {
            QTextStream(stdout) << "Saved to: " << tmpPath << "\n";
        } else {
            QTextStream(stdout) << "Failed to write file: " << tmpPath << "\n";
        }
    }

    QTextStream(stdout) << "\nAll demos complete.\n";
    return 0;
}
