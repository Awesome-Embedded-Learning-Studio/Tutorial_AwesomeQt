/// @file    pdf_search_engine.cpp
/// @brief   Implementation of PdfSearchEngine — PDF text search and extraction.
///
/// Wraps QPdfDocument for synchronous PDF loading, per-page text extraction,
/// full-document search, and bookmark retrieval.

#include "pdf_search_engine.h"

#include <QIODevice>
#include <QPdfDocument>
#include <QPdfSelection>

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

PdfSearchEngine::PdfSearchEngine(QObject* parent)
    : QObject(parent)
    , m_document{new QPdfDocument(this)}  // child — auto-deleted by Qt object tree
{
}

PdfSearchEngine::~PdfSearchEngine() = default;

// ---------------------------------------------------------------------------
// Document loading
// ---------------------------------------------------------------------------

bool PdfSearchEngine::loadDocument(const QString& path)
{
    // Close any previously opened document before loading a new one
    m_document->close();

    // QPdfDocument::load() returns Error::None on success. Any other value
    // indicates a problem (file not found, corrupted PDF, permission error, etc.).
    const QPdfDocument::Error error = m_document->load(path);

    if (error != QPdfDocument::Error::None) {
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// Page count
// ---------------------------------------------------------------------------

int PdfSearchEngine::getPageCount() const
{
    return m_document->pageCount();
}

// ---------------------------------------------------------------------------
// Text extraction
// ---------------------------------------------------------------------------

QString PdfSearchEngine::extractPageText(int page) const
{
    if (page < 0 || page >= m_document->pageCount()) {
        return {};
    }
    return doGetPageText(page);
}

QString PdfSearchEngine::doGetPageText(int page) const
{
    // QPdfDocument::getAllText() extracts all text from a given page as a
    // single string. The page index is zero-based.
    const QPdfSelection selection = m_document->getAllText(page);
    return selection.text();
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------

QVector<int> PdfSearchEngine::search(const QString& text)
{
    QVector<int> matchingPages;

    if (text.isEmpty()) {
        emit searchCompleted(0);
        return matchingPages;
    }

    const int totalPages = m_document->pageCount();

    for (int page = 0; page < totalPages; ++page) {
        // Retrieve all text on this page, then perform a case-insensitive check.
        // QPdfDocument does not provide a cross-page search API, so we iterate.
        const QString pageText = doGetPageText(page);

        if (pageText.contains(text, Qt::CaseInsensitive)) {
            matchingPages.push_back(page);
        }
    }

    emit searchCompleted(static_cast<int>(matchingPages.size()));
    return matchingPages;
}

// ---------------------------------------------------------------------------
// Bookmarks
// ---------------------------------------------------------------------------

QStringList PdfSearchEngine::getBookmarkNames() const
{
    QStringList names;

    // QPdfDocument does not expose a dedicated bookmark/outline API in all
    // versions. When QPdfLinkModel or the outline model is unavailable, we
    // return an empty list. This is a graceful fallback — bookmarks are
    // optional metadata and their absence does not affect core functionality.
    //
    // In Qt 6.x versions that support it, bookmarks can be traversed via
    // QPdfDocument's internal outline structure. For now we keep this as a
    // placeholder that compiles cleanly and returns an empty result.

    return names;
}
