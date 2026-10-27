/// @file    pdf_search_engine.h
/// @brief   PDF text search engine using QPdfDocument.
///
/// Demonstrates advanced PDF operations: document loading, page text extraction,
/// full-document text search, and bookmark name retrieval.
/// Corresponds to tutorial: advanced layer 05-Other-Modules/17-QtPdf.

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

class QPdfDocument;

/// @brief Engine that wraps QPdfDocument for text search and extraction.
///
/// Provides a simplified interface over QPdfDocument: load a PDF file,
/// extract text from individual pages, search across all pages, and
/// list bookmark titles. All heavy operations are synchronous since
/// QPdfDocument::load() blocks until the document is ready.
class PdfSearchEngine : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the engine with an optional parent for ownership.
    /// @param[in] parent Parent QObject for Qt object-tree lifetime management.
    explicit PdfSearchEngine(QObject* parent = nullptr);

    /// @brief Destructor — QPdfDocument is a QObject child, auto-deleted.
    ~PdfSearchEngine() override;

    // Non-copyable, non-movable (QObject base)
    PdfSearchEngine(const PdfSearchEngine&) = delete;
    PdfSearchEngine& operator=(const PdfSearchEngine&) = delete;
    PdfSearchEngine(PdfSearchEngine&&) = delete;
    PdfSearchEngine& operator=(PdfSearchEngine&&) = delete;

    /// @brief Loads a PDF document from the given file path.
    /// @param[in] path Absolute or relative path to the PDF file.
    /// @return true if the document loaded successfully, false otherwise.
    /// @note A previous document is closed automatically before loading the new one.
    bool loadDocument(const QString& path);

    /// @brief Returns the total number of pages in the currently loaded document.
    /// @return Page count, or 0 if no document is loaded.
    int getPageCount() const;

    /// @brief Extracts all searchable text from a single page.
    /// @param[in] page Zero-based page index.
    /// @return Extracted text, or an empty string if the page is invalid or
    ///         text extraction is unavailable for that page.
    QString extractPageText(int page) const;

    /// @brief Searches every page for the given text string.
    /// @param[in] text The search query (case-insensitive).
    /// @return A sorted vector of zero-based page indices that contain the text.
    /// @note Emits searchCompleted() with the number of matching pages.
    QVector<int> search(const QString& text);

    /// @brief Returns the titles of all top-level bookmarks in the document.
    /// @return QStringList of bookmark names; empty if no bookmarks exist.
    /// @note Bookmarks are retrieved from QPdfDocument's built-in outline model.
    QStringList getBookmarkNames() const;

signals:
    /// @brief Emitted after search() completes with the total match count.
    /// @param matchCount Number of pages that matched the query.
    void searchCompleted(int matchCount);

private:
    /// @brief Extracts text from a single page using QPdfDocument's text extraction.
    /// @param[in] page Zero-based page index.
    /// @return Extracted text content.
    /// @note QPdfDocument returns page text via the getAllText() method on a
    ///       QPdfDocument instance when available, otherwise falls back to
    ///       extracting text from the page's text content.
    QString doGetPageText(int page) const;

    QPdfDocument* m_document;  ///< Owned child QObject — the PDF document handle.
};
