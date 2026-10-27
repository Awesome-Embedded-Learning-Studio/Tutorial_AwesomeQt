/// @file    report_generator.cpp
/// @brief   Implementation of the multi-page PDF report generator.
///
/// Handles QPrinter setup, QPainter drawing, page-break detection,
/// header/footer rendering, and table layout with alternating row colors.

#include "report_generator.h"

#include <QLocale>

// Qt headers
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QMarginsF>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPagedPaintDevice>
#include <QPen>
#include <QPdfWriter>
#include <QRectF>

// ----- Constants -----

/// Proportional weights for the four table columns (must sum to 1.0).
static constexpr double kColWeightName   = 0.35;
static constexpr double kColWeightDate   = 0.20;
static constexpr double kColWeightAmount = 0.20;
static constexpr double kColWeightStatus = 0.25;

/// Row height in points for table body rows.
static constexpr qreal kRowHeight = 24.0;

/// Header height in points (reserved at the top of every page).
static constexpr qreal kHeaderHeight = 60.0;

/// Footer height in points (reserved at the bottom of every page).
static constexpr qreal kFooterHeight = 40.0;

/// Small padding inside table cells (left/right text margin).
static constexpr qreal kCellPadding = 6.0;

/// The four alternating status values used in sample data.
static const QString kStatuses[] = {
    QStringLiteral("Paid"),
    QStringLiteral("Pending"),
    QStringLiteral("Overdue"),
    QStringLiteral("Refunded"),
};

// ----- Constructor / Setters -----

ReportGenerator::ReportGenerator(QObject* parent) : QObject(parent) {}

void ReportGenerator::setData(const QVector<ReportRow>& rows) {
    m_rows = rows;
}

void ReportGenerator::setTitle(const QString& title) {
    m_title = title;
}

int ReportGenerator::totalPages() const {
    return m_totalPages;
}

// ----- Main Generation -----

bool ReportGenerator::generate(const QString& filePath) {
    if (m_rows.isEmpty()) {
        m_rows = createSampleData();
    }
    if (m_title.isEmpty()) {
        m_title = QStringLiteral("Financial Report");
    }

    // Configure PDF writer for output
    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);  // High-resolution like QPrinter::HighResolution

    // Set margins so we have a well-defined printable area
    QMarginsF margins(20.0, 20.0, 20.0, 20.0);  // in mm
    writer.setPageMargins(margins, QPageLayout::Millimeter);

    QPainter painter;
    if (!painter.begin(&writer)) {
        return false;
    }

    // Compute layout metrics from the painter device
    QRectF pageRect = writer.pageLayout().paintRect(QPageLayout::Point);
    qreal bodyTop = kHeaderHeight;
    qreal bodyBottom = pageRect.height() - kFooterHeight;

    // Proportional column widths
    qreal tableWidth = pageRect.width();
    QVector<qreal> colWidths = computeColumnWidths(tableWidth);
    qreal x0 = 0.0;  // table starts at the left edge of pageRect

    // Two-pass approach: first pass to count pages, second pass to draw.
    // This is necessary because the footer needs "Page X of Y" where Y
    // is not known until all rows have been laid out.

    // --- Pass 1: Count pages ---
    int pageCount = 1;
    qreal y = bodyTop;
    for (int i = 0; i < m_rows.size(); ++i) {
        if (y + kRowHeight > bodyBottom) {
            ++pageCount;
            y = bodyTop;
        }
        y += kRowHeight;
    }
    m_totalPages = pageCount;

    // --- Pass 2: Draw pages ---
    // Reset printer to page 1
    painter.resetTransform();

    int currentPage = 1;
    y = bodyTop;

    // Draw the header and table header on the first page
    qreal headerEnd = drawHeader(painter, pageRect, currentPage);
    // bodyTop is fixed; headerEnd is returned for information
    Q_UNUSED(headerEnd)

    drawTableHeader(painter, x0, y, colWidths, kRowHeight);

    for (int i = 0; i < m_rows.size(); ++i) {
        // Check if this row would overflow the current page
        if (y + kRowHeight > bodyBottom) {
            // Finish current page: draw footer
            drawFooter(painter, pageRect, currentPage);

            // Start a new page
            writer.newPage();
            ++currentPage;
            y = bodyTop;

            // Draw header and table header on new page
            drawHeader(painter, pageRect, currentPage);
            drawTableHeader(painter, x0, y, colWidths, kRowHeight);
        }

        drawTableRow(painter, m_rows[i], i, x0, y, colWidths, kRowHeight);
    }

    // Draw footer on the last page
    drawFooter(painter, pageRect, currentPage);

    painter.end();
    return true;
}

// ----- Header / Footer -----

qreal ReportGenerator::drawHeader(QPainter& painter, const QRectF& pageRect,
                                  int pageNumber) {
    Q_UNUSED(pageNumber)

    // Title: bold, 14pt
    QFont titleFont(QStringLiteral("Helvetica"), 14, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRectF(0, 0, pageRect.width(), 30), Qt::AlignLeft,
                     m_title);

    // Date on the right side of the header
    QFont dateFont(QStringLiteral("Helvetica"), 10);
    painter.setFont(dateFont);
    QString dateStr = QDate::currentDate().toString(Qt::ISODate);
    painter.drawText(QRectF(0, 0, pageRect.width(), 30), Qt::AlignRight,
                     dateStr);

    // Separator line below the header
    QPen separatorPen(Qt::gray, 1.0);
    painter.setPen(separatorPen);
    qreal lineY = kHeaderHeight - 8.0;
    painter.drawLine(QPointF(0, lineY), QPointF(pageRect.width(), lineY));

    // Reset pen for subsequent drawing
    painter.setPen(Qt::black);

    return kHeaderHeight;
}

void ReportGenerator::drawFooter(QPainter& painter, const QRectF& pageRect,
                                 int pageNumber) {
    QFont footerFont(QStringLiteral("Helvetica"), 9);
    painter.setFont(footerFont);

    QString footerText = QStringLiteral("Page %1 of %2")
                             .arg(pageNumber)
                             .arg(m_totalPages);

    qreal footerY = pageRect.height() - kFooterHeight + 10.0;
    QRectF footerRect(0, footerY, pageRect.width(), 20.0);
    painter.drawText(footerRect, Qt::AlignCenter, footerText);
}

// ----- Table Drawing -----

void ReportGenerator::drawTableHeader(QPainter& painter, qreal x, qreal& y,
                                      const QVector<qreal>& colWidths,
                                      qreal rowHeight) {
    static const QStringList kHeaders = {
        QStringLiteral("Name"),
        QStringLiteral("Date"),
        QStringLiteral("Amount"),
        QStringLiteral("Status"),
    };

    // Background for the header row
    QRectF headerRect(x, y, 0, rowHeight);
    for (auto w : colWidths) {
        headerRect.setWidth(headerRect.width() + w);
    }
    painter.fillRect(headerRect, QColor(60, 120, 180));

    // Header text in white, bold
    QFont headerFont(QStringLiteral("Helvetica"), 10, QFont::Bold);
    painter.setFont(headerFont);
    painter.setPen(Qt::white);

    qreal colX = x;
    for (int col = 0; col < kHeaders.size(); ++col) {
        QRectF cellRect(colX + kCellPadding, y, colWidths[col] - 2 * kCellPadding,
                        rowHeight);
        painter.drawText(cellRect, Qt::AlignVCenter | Qt::AlignLeft,
                         kHeaders[col]);
        colX += colWidths[col];
    }

    // Border around header row
    painter.setPen(QPen(Qt::black, 1.0));
    painter.drawRect(headerRect);

    // Reset pen
    painter.setPen(Qt::black);

    y += rowHeight;
}

void ReportGenerator::drawTableRow(QPainter& painter, const ReportRow& row,
                                   int rowIndex, qreal x, qreal& y,
                                   const QVector<qreal>& colWidths,
                                   qreal rowHeight) {
    // Alternating row background
    bool isEven = (rowIndex % 2 == 0);
    QColor bgColor = isEven ? QColor(245, 245, 245) : QColor(255, 255, 255);

    QRectF rowRect(x, y, 0, rowHeight);
    for (auto w : colWidths) {
        rowRect.setWidth(rowRect.width() + w);
    }
    painter.fillRect(rowRect, bgColor);

    // Body font
    QFont bodyFont(QStringLiteral("Helvetica"), 9);
    painter.setFont(bodyFont);
    painter.setPen(Qt::black);

    // Prepare cell text values
    QStringList cellTexts;
    cellTexts << row.name
              << row.date
              << QLocale().toCurrencyString(row.amount)
              << row.status;

    qreal colX = x;
    for (int col = 0; col < cellTexts.size(); ++col) {
        QRectF cellRect(colX + kCellPadding, y,
                        colWidths[col] - 2 * kCellPadding, rowHeight);

        // Right-align the amount column for financial convention
        Qt::Alignment align =
            (col == 2) ? Qt::AlignVCenter | Qt::AlignRight
                       : Qt::AlignVCenter | Qt::AlignLeft;

        // Use elided text if the cell is too narrow
        QFontMetrics fm(bodyFont);
        QString text = fm.elidedText(cellTexts[col], Qt::ElideRight,
                                     static_cast<int>(cellRect.width()));

        painter.drawText(cellRect, align, text);
        colX += colWidths[col];
    }

    // Cell border lines
    painter.setPen(QPen(Qt::lightGray, 0.5));
    painter.drawLine(QPointF(x, y + rowHeight),
                     QPointF(x + rowRect.width(), y + rowHeight));

    // Vertical separators between columns
    colX = x;
    for (int col = 0; col < colWidths.size() - 1; ++col) {
        colX += colWidths[col];
        painter.drawLine(QPointF(colX, y), QPointF(colX, y + rowHeight));
    }

    // Outer border
    painter.setPen(QPen(Qt::black, 1.0));
    painter.drawRect(rowRect);

    // Reset pen
    painter.setPen(Qt::black);

    y += rowHeight;
}

// ----- Helpers -----

QVector<qreal> ReportGenerator::computeColumnWidths(qreal tableWidth) {
    QVector<qreal> widths;
    widths.reserve(4);
    widths << tableWidth * kColWeightName   // Name: 35%
           << tableWidth * kColWeightDate   // Date: 20%
           << tableWidth * kColWeightAmount // Amount: 20%
           << tableWidth * kColWeightStatus; // Status: 25%
    return widths;
}

QVector<ReportRow> ReportGenerator::createSampleData() {
    QVector<ReportRow> rows;
    rows.reserve(30);

    for (int i = 1; i <= 30; ++i) {
        ReportRow row;
        row.name = QStringLiteral("Employee %1").arg(i, 2, 10, QChar('0'));
        row.date = QDate(2026, 1, 1).addDays(i * 3).toString(Qt::ISODate);
        row.amount = 1000.0 + i * 150.75;
        row.status = kStatuses[i % 4];
        rows.append(row);
    }

    return rows;
}
