/// @file    svg_controller.cpp
/// @brief   Implementation of the SvgController class.
///
/// Provides SVG DOM manipulation using QDomDocument: creating sample SVGs,
/// modifying element attributes, toggling visibility, and serializing output.
/// Corresponds to: advanced layer 05-Other-Modules/06-QtSvg.

#include "svg_controller.h"

#include <QFile>
#include <QTextStream>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

/// @brief Constructs the controller with an empty SVG DOM document.
/// @param[in] parent  Parent QObject for ownership via Qt object tree.
SvgController::SvgController(QObject* parent)
    : QObject(parent)
{
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/// @brief Creates a sample SVG with a circle, rectangle, and text element.
///
/// The SVG viewport is 200x150. Each element has a unique id attribute so
/// it can be located and modified later:
///   - "demo-circle" : blue circle at (60, 60) with radius 40
///   - "demo-rect"   : green rectangle at (120, 30) with size 60x80
///   - "demo-text"   : dark gray text "Hello SVG" at (30, 130)
void SvgController::createSampleSvg()
{
    m_doc = QDomDocument();

    // SVG processing instruction for well-formed XML output
    auto pi = m_doc.createProcessingInstruction(
        QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\""));
    m_doc.appendChild(pi);

    // <svg> root element with namespace and viewport
    auto svg = m_doc.createElementNS(QStringLiteral("http://www.w3.org/2000/svg"),
                                     QStringLiteral("svg"));
    svg.setAttribute(QStringLiteral("xmlns"), QStringLiteral("http://www.w3.org/2000/svg"));
    svg.setAttribute(QStringLiteral("width"), QStringLiteral("200"));
    svg.setAttribute(QStringLiteral("height"), QStringLiteral("150"));
    svg.setAttribute(QStringLiteral("viewBox"), QStringLiteral("0 0 200 150"));
    m_doc.appendChild(svg);

    // <circle id="demo-circle"> — blue filled circle
    auto circle = m_doc.createElement(QStringLiteral("circle"));
    circle.setAttribute(QStringLiteral("id"), QStringLiteral("demo-circle"));
    circle.setAttribute(QStringLiteral("cx"), QStringLiteral("60"));
    circle.setAttribute(QStringLiteral("cy"), QStringLiteral("60"));
    circle.setAttribute(QStringLiteral("r"), QStringLiteral("40"));
    circle.setAttribute(QStringLiteral("fill"), QStringLiteral("blue"));
    svg.appendChild(circle);

    // <rect id="demo-rect"> — green filled rectangle
    auto rect = m_doc.createElement(QStringLiteral("rect"));
    rect.setAttribute(QStringLiteral("id"), QStringLiteral("demo-rect"));
    rect.setAttribute(QStringLiteral("x"), QStringLiteral("120"));
    rect.setAttribute(QStringLiteral("y"), QStringLiteral("30"));
    rect.setAttribute(QStringLiteral("width"), QStringLiteral("60"));
    rect.setAttribute(QStringLiteral("height"), QStringLiteral("80"));
    rect.setAttribute(QStringLiteral("fill"), QStringLiteral("green"));
    svg.appendChild(rect);

    // <text id="demo-text"> — descriptive label at the bottom
    auto text = m_doc.createElement(QStringLiteral("text"));
    text.setAttribute(QStringLiteral("id"), QStringLiteral("demo-text"));
    text.setAttribute(QStringLiteral("x"), QStringLiteral("30"));
    text.setAttribute(QStringLiteral("y"), QStringLiteral("130"));
    text.setAttribute(QStringLiteral("font-size"), QStringLiteral("18"));
    text.setAttribute(QStringLiteral("fill"), QStringLiteral("#333333"));
    auto textNode = m_doc.createTextNode(QStringLiteral("Hello SVG"));
    text.appendChild(textNode);
    svg.appendChild(text);
}

/// @brief Sets the fill color attribute of an SVG element.
/// @param[in] elementId  The id attribute of the target element.
/// @param[in] color      New fill color (CSS color value).
/// @return true if the element was found and its fill attribute was set.
bool SvgController::setElementColor(const QString& elementId, const QString& color)
{
    auto elem = findElementById(elementId);
    if (elem.isNull()) {
        return false;
    }
    elem.setAttribute(QStringLiteral("fill"), color);
    return true;
}

/// @brief Shows or hides an SVG element via CSS display property.
/// @param[in] elementId  The id attribute of the target element.
/// @param[in] visible    True to make visible, false to hide.
/// @return true if the element was found and its style was updated.
/// @note  Visibility is controlled through the inline style attribute.
///        When hiding, style="display:none" is applied. When showing,
///        the display property is stripped so the element returns to its
///        default rendering.
bool SvgController::setElementVisibility(const QString& elementId, bool visible)
{
    auto elem = findElementById(elementId);
    if (elem.isNull()) {
        return false;
    }

    if (visible) {
        // Remove display:none from the style string to restore visibility
        QString style = elem.attribute(QStringLiteral("style"));
        if (style.contains(QStringLiteral("display:none"))) {
            style.remove(QStringLiteral("display:none"));
            // Clean up leading/trailing semicolons and whitespace
            style = style.trimmed();
            if (style.startsWith(QLatin1Char(';'))) {
                style.remove(0, 1);
            }
            if (style.endsWith(QLatin1Char(';'))) {
                style.chop(1);
            }
            style = style.trimmed();
        }
        if (style.isEmpty()) {
            elem.removeAttribute(QStringLiteral("style"));
        } else {
            elem.setAttribute(QStringLiteral("style"), style);
        }
    } else {
        // Append display:none to existing style or create new style attribute
        QString style = elem.attribute(QStringLiteral("style"));
        if (!style.isEmpty()) {
            style += QLatin1Char(';');
        }
        style += QStringLiteral("display:none");
        elem.setAttribute(QStringLiteral("style"), style);
    }

    return true;
}

/// @brief Replaces the text content of a <text> SVG element.
/// @param[in] elementId  The id attribute of the target <text> element.
/// @param[in] text       New text content.
/// @return true if the element was found and its first text node was updated.
/// @note  If the element has no existing text child, a new one is appended.
bool SvgController::setElementText(const QString& elementId, const QString& text)
{
    auto elem = findElementById(elementId);
    if (elem.isNull()) {
        return false;
    }

    // Replace the first child text node if it exists
    auto child = elem.firstChild();
    while (!child.isNull()) {
        if (child.isText()) {
            child.setNodeValue(text);
            return true;
        }
        child = child.nextSibling();
    }

    // No text node found — create and append one
    auto textNode = m_doc.createTextNode(text);
    elem.appendChild(textNode);
    return true;
}

/// @brief Sets an arbitrary XML attribute on the specified element.
/// @param[in] elementId  The id attribute of the target element.
/// @param[in] attr       Attribute name (e.g. "r", "width", "transform").
/// @param[in] value      Attribute value as a string.
/// @return true if the element was found and the attribute was set.
bool SvgController::setElementAttribute(const QString& elementId,
                                        const QString& attr, const QString& value)
{
    auto elem = findElementById(elementId);
    if (elem.isNull()) {
        return false;
    }
    elem.setAttribute(attr, value);
    return true;
}

/// @brief Serializes the entire SVG DOM to a UTF-8 byte array.
/// @return QByteArray containing the complete SVG XML document.
[[nodiscard]] QByteArray SvgController::toByteArray() const
{
    return m_doc.toByteArray(2);  // 2-space indent for readability
}

/// @brief Writes the SVG document to a file on disk.
/// @param[in] path  File system path for the output file.
/// @return true if the file was opened and written successfully.
bool SvgController::saveToFile(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(toByteArray());
    file.close();
    return true;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

/// @brief Finds an element by id attribute, starting from the document root.
/// @param[in] elementId  The id value to search for.
/// @return The matching QDomElement, or a null element.
[[nodiscard]] QDomElement SvgController::findElementById(
    const QString& elementId) const
{
    auto root = m_doc.documentElement();
    if (root.isNull()) {
        return {};
    }
    // Check the root itself in case its id matches
    if (root.attribute(QStringLiteral("id")) == elementId) {
        return root;
    }
    return findElementByIdRecursive(root, elementId);
}

/// @brief Depth-first recursive search for an element with the given id.
/// @param[in] parent     The parent element whose children are searched.
/// @param[in] elementId  The id value to look for.
/// @return The matching QDomElement, or a null element if not found.
[[nodiscard]] QDomElement SvgController::findElementByIdRecursive(
    const QDomElement& parent, const QString& elementId) const
{
    auto child = parent.firstChildElement();
    while (!child.isNull()) {
        if (child.attribute(QStringLiteral("id")) == elementId) {
            return child;
        }
        // Recurse into children of this child
        auto found = findElementByIdRecursive(child, elementId);
        if (!found.isNull()) {
            return found;
        }
        child = child.nextSiblingElement();
    }
    return {};
}
