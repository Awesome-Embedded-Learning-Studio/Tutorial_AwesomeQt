/// @file    main.cpp
/// @brief   Console demo for PdfSearchEngine — create, load, search PDFs.
///
/// Demonstrates:
///   1. Creating a simple test PDF using QPdfWriter + QPainter.
///   2. Loading the PDF and reading the page count.
///   3. Extracting text from page 0.
///   4. Searching for specific text across all pages.
///   5. Listing bookmark names (if any).
///
/// The app auto-quits after printing results. Corresponds to tutorial:
/// advanced layer 05-Other-Modules/17-QtPdf.

#include "pdf_search_engine.h"

#include <QGuiApplication>
#include <QFile>
#include <QPainter>
#include <QPdfDocument>
#include <QPdfWriter>
#include <QTextStream>
#include <QTimer>

#include <cstdio>

namespace {

/// @brief Creates a minimal multi-page PDF file for testing.
/// @param[in] path File path to write the PDF to.
/// @return true if the file was created successfully.
/// @note Uses QPdfWriter + QPainter to render text directly onto PDF pages.
///       This approach does not require a GUI event loop, making it suitable
///       for headless/console environments.
bool createTestPdf(const QString& path)
{
    QPdfWriter writer{path};

    // Set a reasonable page size for the test document
    writer.setPageSize(QPageSize{QPageSize::A4});

    QPainter painter{&writer};
    if (!painter.isActive()) {
        return false;
    }

    // Page 1 content
    QFont headingFont{};
    headingFont.setPointSize(16);
    headingFont.setBold(true);

    QFont bodyFont{};
    bodyFont.setPointSize(10);

    int yPos = 100;

    // --- Page 1 ---
    painter.setFont(headingFont);
    painter.drawText(100, yPos, QStringLiteral("QtPdf Advanced Demo"));
    yPos += 40;

    painter.setFont(bodyFont);
    painter.drawText(100, yPos,
        QStringLiteral("This is page one of the test PDF document."));
    yPos += 25;
    painter.drawText(100, yPos,
        QStringLiteral("It contains searchable keywords like "
                       "Qt framework and PDF rendering."));
    yPos += 25;
    painter.drawText(100, yPos,
        QStringLiteral("Chapter 1: Introduction"));
    yPos += 25;
    painter.drawText(100, yPos,
        QStringLiteral("The Qt framework provides powerful cross-platform tools."));

    // --- Page 2 ---
    writer.newPage();
    yPos = 100;

    painter.setFont(headingFont);
    painter.drawText(100, yPos, QStringLiteral("Page Two"));
    yPos += 40;

    painter.setFont(bodyFont);
    painter.drawText(100, yPos,
        QStringLiteral("Advanced PDF operations include text extraction, "
                       "page navigation, and bookmark handling."));
    yPos += 25;
    painter.drawText(100, yPos,
        QStringLiteral("Searching for PDF rendering should match this page too."));
    yPos += 25;
    painter.drawText(100, yPos,
        QStringLiteral("Chapter 2: PDF Search"));
    yPos += 25;
    painter.drawText(100, yPos,
        QStringLiteral("The search engine scans every page for the query string."));

    // --- Page 3 ---
    writer.newPage();
    yPos = 100;

    painter.setFont(headingFont);
    painter.drawText(100, yPos, QStringLiteral("Page Three"));
    yPos += 40;

    painter.setFont(bodyFont);
    painter.drawText(100, yPos,
        QStringLiteral("Final page with some concluding remarks about "
                       "Qt framework capabilities."));
    yPos += 25;
    painter.drawText(100, yPos,
        QStringLiteral("This page also mentions PDF rendering for search testing."));

    painter.end();
    return true;
}

/// @brief Prints a separator line to stdout for readability.
void printSeparator()
{
    std::puts("------------------------------------------------------------");
}

}  // namespace

int main(int argc, char* argv[])
{
    QGuiApplication app{argc, argv};

    // Use QTextStream for safe, Unicode-aware console output
    QTextStream out{stdout};
    out.setEncoding(QStringConverter::Utf8);

    const QString testPdfPath =
        QStringLiteral("qtpdf_test_document.pdf");

    // ------------------------------------------------------------------
    // Demo 1: Create a test PDF
    // ------------------------------------------------------------------
    out << "=== Demo 1: Creating test PDF ===\n";
    printSeparator();

    if (!createTestPdf(testPdfPath)) {
        out << "ERROR: Failed to create test PDF file.\n";
        out.flush();
        return 1;
    }

    out << "Test PDF created: " << testPdfPath << "\n";
    out.flush();

    // ------------------------------------------------------------------
    // Demo 2: Load the PDF and read page count
    // ------------------------------------------------------------------
    out << "\n=== Demo 2: Loading PDF and reading page count ===\n";
    printSeparator();

    PdfSearchEngine engine;

    if (!engine.loadDocument(testPdfPath)) {
        out << "ERROR: Failed to load the test PDF.\n";
        out.flush();
        return 1;
    }

    const int pageCount = engine.getPageCount();
    out << "Page count: " << pageCount << "\n";
    out.flush();

    if (pageCount <= 0) {
        out << "WARNING: Document loaded but reports 0 pages.\n";
        out.flush();
    }

    // ------------------------------------------------------------------
    // Demo 3: Extract text from page 0
    // ------------------------------------------------------------------
    out << "\n=== Demo 3: Extracting text from page 1 (index 0) ===\n";
    printSeparator();

    const QString page0Text = engine.extractPageText(0);

    if (page0Text.isEmpty()) {
        out << "No text extracted from page 0.\n";
    } else {
        // Print only the first 300 characters to keep output manageable
        out << "Text (first 300 chars):\n"
            << page0Text.left(300) << "\n";
    }
    out.flush();

    // ------------------------------------------------------------------
    // Demo 4: Search for text in the document
    // ------------------------------------------------------------------
    out << "\n=== Demo 4: Searching for 'PDF rendering' ===\n";
    printSeparator();

    const QVector<int> matchingPages =
        engine.search(QStringLiteral("PDF rendering"));

    out << "Found 'PDF rendering' on "
        << matchingPages.size() << " page(s).\n";

    if (!matchingPages.isEmpty()) {
        out << "Matching page indices (zero-based):";
        for (int page : matchingPages) {
            out << " " << page;
        }
        out << "\n";
    }
    out.flush();

    // ------------------------------------------------------------------
    // Demo 5: Search for another term
    // ------------------------------------------------------------------
    out << "\n=== Demo 5: Searching for 'Qt framework' ===\n";
    printSeparator();

    const QVector<int> qtPages =
        engine.search(QStringLiteral("Qt framework"));

    out << "Found 'Qt framework' on "
        << qtPages.size() << " page(s).\n";

    if (!qtPages.isEmpty()) {
        out << "Matching page indices (zero-based):";
        for (int page : qtPages) {
            out << " " << page;
        }
        out << "\n";
    }
    out.flush();

    // ------------------------------------------------------------------
    // Demo 6: Bookmark names
    // ------------------------------------------------------------------
    out << "\n=== Demo 6: Listing bookmark names ===\n";
    printSeparator();

    const QStringList bookmarks = engine.getBookmarkNames();

    if (bookmarks.isEmpty()) {
        out << "No bookmarks found (expected — QPdfWriter does not generate them).\n";
    } else {
        out << "Bookmarks (" << bookmarks.size() << "):\n";
        for (const QString& name : bookmarks) {
            out << "  - " << name << "\n";
        }
    }
    out.flush();

    // ------------------------------------------------------------------
    // Cleanup and exit
    // ------------------------------------------------------------------
    out << "\n=== All demos completed. Cleaning up. ===\n";
    out.flush();

    // Remove the temporary test PDF
    QFile::remove(testPdfPath);

    // Auto-quit after a short delay to allow pending signals to settle
    QTimer::singleShot(0, &app, &QGuiApplication::quit);

    return app.exec();
}
