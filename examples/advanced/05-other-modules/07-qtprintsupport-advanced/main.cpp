/// @file    main.cpp
/// @brief   Entry point for the PDF report generation GUI example.
///
/// Demonstrates ReportGenerator usage through a simple GUI window.
/// Click "Generate Report" to create a multi-page PDF with tables,
/// headers, footers, and alternating row colors.

#include "report_generator.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

/// @brief Application entry point.
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Qt Print Support Demo");

    // -- Main window --
    QWidget window;
    window.setWindowTitle("07-qtprintsupport-advanced");
    window.setMinimumSize(500, 350);

    auto* layout = new QVBoxLayout(&window);

    // Title
    auto* titleLabel = new QLabel("PDF Report Generator");
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Info text area
    auto* infoText = new QTextEdit;
    infoText->setReadOnly(true);
    infoText->setPlaceholderText("Click the button below to generate a PDF report...");
    layout->addWidget(infoText);

    // Generate button
    auto* generateBtn = new QPushButton("Generate PDF Report");
    generateBtn->setMinimumHeight(40);
    layout->addWidget(generateBtn);

    // Build output path
    QString outputPath = QDir::tempPath() + QDir::separator() + "qt_report_demo.pdf";

    // Connect button to generation logic
    QObject::connect(generateBtn, &QPushButton::clicked, [&]() {
        infoText->clear();
        infoText->append("Generating report to: " + outputPath + "\n");

        ReportGenerator generator;
        generator.setTitle("Quarterly Financial Summary");
        generator.setData(ReportGenerator::createSampleData());

        bool success = generator.generate(outputPath);

        if (success) {
            QFileInfo fi(outputPath);
            infoText->append("Report generated successfully!");
            infoText->append(QString("  Pages: %1").arg(generator.totalPages()));
            infoText->append(QString("  Size:  %1 bytes").arg(fi.size()));
            infoText->append(QString("  Path:  %1").arg(fi.absoluteFilePath()));
        } else {
            infoText->append("ERROR: Failed to generate report.");
        }
    });

    window.show();
    return app.exec();
}
