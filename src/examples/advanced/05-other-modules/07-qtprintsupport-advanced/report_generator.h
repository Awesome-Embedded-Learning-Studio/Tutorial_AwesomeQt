/// @file    report_generator.h
/// @brief   Multi-page PDF report generator using QPdfWriter and QPainter.
///
/// Demonstrates: page headers/footers, auto-pagination, table layout with
/// alternating row colors, text wrapping, and proportional column widths.
/// Corresponds to tutorial: advanced 05-other-modules/07-qtprintsupport.

#pragma once

#include <QDate>
#include <QMarginsF>
#include <QObject>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QRectF>
#include <QString>
#include <QVector>

/// @brief Represents a single data row in the report table.
struct ReportRow {
    QString name;    ///< Person or item name
    QString date;    ///< Date string (YYYY-MM-DD)
    double amount;   ///< Monetary amount
    QString status;  ///< Status label (e.g. "Paid", "Pending")
};

/// @brief Generates a paginated PDF report with header, footer, and table.
///
/// Uses QPdfWriter and QPainter for all drawing. Automatically
/// inserts page breaks when content exceeds the printable area. Each page
/// gets a consistent header (title + date) and footer (page numbering).
class ReportGenerator : public QObject {
    Q_OBJECT

public:
    /// @brief Constructs the generator with optional parent for Qt ownership.
    /// @param[in] parent Parent QObject for lifetime management.
    explicit ReportGenerator(QObject* parent = nullptr);

    /// @brief Sets the report data rows to be rendered into the table.
    /// @param[in] rows The data rows. Copied internally.
    void setData(const QVector<ReportRow>& rows);

    /// @brief Sets the report title shown in the page header.
    /// @param[in] title Title string displayed on every page.
    void setTitle(const QString& title);

    /// @brief Generates the PDF report to the given file path.
    /// @param[in] filePath Absolute path for the output PDF file.
    /// @return true if the PDF was written successfully, false on error.
    ///
    /// Opens QPdfWriter, then iterates through data rows,
    /// drawing header/footer/table content page by page.
    bool generate(const QString& filePath);

    /// @brief Returns the total number of pages rendered in the last generate() call.
    /// @return Page count, or 0 if generate() has not been called.
    int totalPages() const;

    /// @brief Creates the sample dataset of 30 ReportRow entries.
    /// @return A vector populated with realistic-looking test data.
    ///
    /// @note 30 rows are enough to overflow a single A4 page and exercise
    ///       the auto-pagination logic with a typical font/table size.
    static QVector<ReportRow> createSampleData();

private:
    /// @brief Draws the page header: title, date, and a separator line.
    /// @param[in,out] painter The active QPainter on the current page.
    /// @param[in] pageRect The printable rectangle of the page.
    /// @param[in] pageNumber The 1-based page number.
    /// @return The Y coordinate after the header (where body content starts).
    ///
    /// @note The header height must be consistent across pages so that the
    ///       body table aligns uniformly regardless of page number.
    qreal drawHeader(QPainter& painter, const QRectF& pageRect, int pageNumber);

    /// @brief Draws the page footer: "Page X of Y" centered at the bottom.
    /// @param[in,out] painter The active QPainter on the current page.
    /// @param[in] pageRect The printable rectangle of the page.
    /// @param[in] pageNumber The 1-based current page number.
    ///
    /// @note Footer is drawn after body content so Y-total is already known,
    ///       but we place it at a fixed bottom position independent of content.
    void drawFooter(QPainter& painter, const QRectF& pageRect, int pageNumber);

    /// @brief Draws the table column headers (Name, Date, Amount, Status).
    /// @param[in,out] painter The active QPainter.
    /// @param[in] x Left edge of the table.
    /// @param[in,out] y Current Y position; updated to below the header row.
    /// @param[in] colWidths Width of each column.
    /// @param[in] rowHeight Height of a single table row.
    void drawTableHeader(QPainter& painter, qreal x, qreal& y,
                         const QVector<qreal>& colWidths, qreal rowHeight);

    /// @brief Draws a single data row into the table.
    /// @param[in,out] painter The active QPainter.
    /// @param[in] row The data to render.
    /// @param[in] rowIndex 0-based index of the row (used for alternating colors).
    /// @param[in] x Left edge of the table.
    /// @param[in,out] y Current Y position; updated to below this row.
    /// @param[in] colWidths Width of each column.
    /// @param[in] rowHeight Height of a single table row.
    void drawTableRow(QPainter& painter, const ReportRow& row, int rowIndex,
                      qreal x, qreal& y, const QVector<qreal>& colWidths,
                      qreal rowHeight);

    /// @brief Computes proportional column widths based on page width.
    /// @param[in] tableWidth Total available width for the table.
    /// @return A vector of 4 column widths (name, date, amount, status).
    static QVector<qreal> computeColumnWidths(qreal tableWidth);

    QVector<ReportRow> m_rows;       ///< Data rows to render
    QString m_title;                  ///< Report title for header
    int m_totalPages = 0;            ///< Pages rendered in last generate()
};
