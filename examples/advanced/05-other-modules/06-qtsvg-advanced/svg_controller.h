/// @file    svg_controller.h
/// @brief   SVG DOM manipulation controller using QDomDocument.
///
/// Demonstrates dynamic SVG element manipulation at runtime by modifying
/// attributes, visibility, and text content through QDomDocument.
/// Corresponds to: advanced layer 05-Other-Modules/06-QtSvg.

#pragma once

#include <QByteArray>
#include <QDomDocument>
#include <QObject>
#include <QString>

/// @brief Controller class for manipulating SVG documents via DOM.
///
/// Wraps QDomDocument to provide high-level operations on SVG elements:
/// color changes, visibility toggling, text updates, and generic attribute
/// manipulation. All elements are located by their XML id attribute.
class SvgController : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs an SVG controller with an empty document.
    /// @param[in] parent  Parent QObject for ownership management.
    explicit SvgController(QObject* parent = nullptr);

    /// @brief Builds a sample SVG containing a circle, rectangle, and text.
    /// @note  Replaces any existing document content. Element IDs are
    ///        "demo-circle", "demo-rect", and "demo-text" respectively.
    void createSampleSvg();

    /// @brief Changes the fill color of the element identified by elementId.
    /// @param[in] elementId  The SVG element's id attribute value.
    /// @param[in] color      CSS color string (e.g. "#ff0000", "red").
    /// @return true if the element was found and updated, false otherwise.
    bool setElementColor(const QString& elementId, const QString& color);

    /// @brief Toggles element visibility via the display CSS property.
    /// @param[in] elementId  The SVG element's id attribute value.
    /// @param[in] visible    True to show, false to hide.
    /// @return true if the element was found and updated, false otherwise.
    /// @note  Hidden elements receive style="display:none". Visible elements
    ///        have the display property removed from the style string.
    bool setElementVisibility(const QString& elementId, bool visible);

    /// @brief Updates the text content of a <text> element.
    /// @param[in] elementId  The SVG element's id attribute value.
    /// @param[in] text       New text content to set.
    /// @return true if the element was found and updated, false otherwise.
    /// @note  This replaces the first child text node of the element.
    bool setElementText(const QString& elementId, const QString& text);

    /// @brief Sets an arbitrary attribute on the specified element.
    /// @param[in] elementId  The SVG element's id attribute value.
    /// @param[in] attr       Attribute name (e.g. "r", "width", "transform").
    /// @param[in] value      Attribute value as a string.
    /// @return true if the element was found and updated, false otherwise.
    bool setElementAttribute(const QString& elementId, const QString& attr,
                             const QString& value);

    /// @brief Serializes the SVG document to a UTF-8 byte array.
    /// @return QByteArray containing the full SVG XML content.
    [[nodiscard]] QByteArray toByteArray() const;

    /// @brief Writes the current SVG document to a file on disk.
    /// @param[in] path  Destination file path (absolute or relative).
    /// @return true if the file was written successfully, false on error.
    bool saveToFile(const QString& path) const;

private:
    /// @brief Finds a QDomElement by its id attribute.
    /// @param[in] elementId  The id value to search for.
    /// @return The matching element, or a null element if not found.
    /// @note  Performs a recursive depth-first search from the document root.
    [[nodiscard]] QDomElement findElementById(const QString& elementId) const;

    /// @brief Recursive helper for findElementById.
    /// @param[in] parent     The parent element to search within.
    /// @param[in] elementId  The id value to search for.
    /// @return The matching element, or a null element if not found.
    [[nodiscard]] QDomElement findElementByIdRecursive(const QDomElement& parent,
                                                       const QString& elementId) const;

    QDomDocument m_doc;  ///< The in-memory SVG DOM document.
};
