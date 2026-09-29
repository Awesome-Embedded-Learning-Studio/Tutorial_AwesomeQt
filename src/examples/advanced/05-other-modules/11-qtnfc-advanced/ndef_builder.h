/// @file    ndef_builder.h
/// @brief   NDEF message builder and parser using QtNfc API.
///
/// Demonstrates constructing NDEF text/URI records, assembling composite
/// messages, serializing to raw bytes, and parsing them back — all without
/// requiring physical NFC hardware.

#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QNdefMessage>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include <QString>
#include <QUrl>

/// @brief Builder and parser for NFC NDEF messages.
///
/// Wraps the QtNfc convenience classes to show how text and URI records are
/// created, combined into a single NDEF message, serialized, and round-tripped
/// back from raw bytes.
class NdefBuilder : public QObject
{
    Q_OBJECT

public:
    /// @brief Construct the builder.
    /// @param[in] parent  Parent QObject for ownership via Qt object tree.
    explicit NdefBuilder(QObject* parent = nullptr);

    /// @brief Create an NDEF text record with the given payload and locale.
    /// @param[in] text    The text payload to encode.
    /// @param[in] locale  RFC 3066 locale tag (e.g. "en", "zh-Hans").
    /// @return A fully configured QNdefNfcTextRecord.
    /// @note QtNfc automatically sets the TNF (Type Name Format) to
    ///       QNdefRecord::NfcRtd when using the convenience subclass.
    QNdefNfcTextRecord buildTextRecord(
        const QString& text, const QString& locale) const;

    /// @brief Create an NDEF URI record from a QUrl.
    /// @param[in] uri  The URI to encode into the record.
    /// @return A fully configured QNdefNfcUriRecord.
    /// @note QNdefNfcUriRecord applies URI identifier abbreviation
    ///       (e.g. "https://www." → 0x01 prefix byte) automatically.
    QNdefNfcUriRecord buildUriRecord(const QUrl& uri) const;

    /// @brief Build a composite NDEF message containing text and URI records.
    /// @return A QNdefMessage with two records: text + URI.
    /// @note A single NDEF message may carry multiple records of different
    ///       types; the NDEF spec treats the message as a sequence.
    QNdefMessage buildCompositeMessage() const;

    /// @brief Parse and print every record in an NDEF message.
    /// @param[in] msg  The NDEF message to inspect.
    /// @note Uses qDebug for output so this works in headless / CI builds.
    void parseMessage(const QNdefMessage& msg) const;

    /// @brief Serialize an NDEF message to a byte array.
    /// @param[in] msg  The message to serialize.
    /// @return Raw NDEF byte representation.
    static QByteArray toByteArray(const QNdefMessage& msg);

    /// @brief Deserialize a byte array back into an NDEF message.
    /// @param[in] data  Raw NDEF bytes (as produced by toByteArray).
    /// @return The reconstructed QNdefMessage.
    /// @note This is the standard round-trip path: build → serialize →
    ///       transmit (or store) → deserialize → parse.
    static QNdefMessage fromByteArray(const QByteArray& data);
};
