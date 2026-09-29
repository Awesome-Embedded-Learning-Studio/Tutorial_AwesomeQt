/// @file    ndef_builder.cpp
/// @brief   Implementation of the NDEF message builder and parser.

#include "ndef_builder.h"

#include <QNdefMessage>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include <QDebug>
#include <QUrl>

NdefBuilder::NdefBuilder(QObject* parent) : QObject(parent)
{
}

QNdefNfcTextRecord NdefBuilder::buildTextRecord(
    const QString& text, const QString& locale) const
{
    QNdefNfcTextRecord record;

    // setText / setLocale configure the payload; encoding defaults to UTF-8.
    record.setText(text);
    record.setLocale(locale);

    return record;
}

QNdefNfcUriRecord NdefBuilder::buildUriRecord(const QUrl& uri) const
{
    QNdefNfcUriRecord record;

    // QNdefNfcUriRecord internally applies the NDEF URI identifier code
    // that abbreviates common prefixes like "https://www.".
    record.setUri(uri);

    return record;
}

QNdefMessage NdefBuilder::buildCompositeMessage() const
{
    QNdefMessage message;

    // Append a text record describing the tag content.
    auto textRecord = buildTextRecord(
        QStringLiteral("AwesomeQt NFC Demo"), QStringLiteral("en"));

    // Append a URI record pointing to the project site.
    auto uriRecord = buildUriRecord(QUrl(QStringLiteral("https://www.example.com")));

    // QNdefMessage inherits QList<QNdefRecord>, so we use QList API.
    message.append(textRecord);
    message.append(uriRecord);

    return message;
}

void NdefBuilder::parseMessage(const QNdefMessage& msg) const
{
    qDebug() << "=== NDEF Message (" << msg.size() << "record(s)) ===";

    int index = 0;
    for (const auto& record : msg) {
        qDebug() << "\n--- Record" << ++index << "---";
        qDebug() << "  TNF    :" << record.typeNameFormat();
        qDebug() << "  Type   :" << record.type();
        qDebug() << "  Id     :" << record.id();
        qDebug() << "  Payload:" << record.payload();

        // Try to interpret as a known convenience type.
        if (record.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord textRecord(record);
            qDebug() << "  [Text] content:" << textRecord.text();
            qDebug() << "  [Text] locale :" << textRecord.locale();
        } else if (record.isRecordType<QNdefNfcUriRecord>()) {
            QNdefNfcUriRecord uriRecord(record);
            qDebug() << "  [URI] target  :" << uriRecord.uri().toString();
        } else {
            qDebug() << "  [Unknown record type]";
        }
    }

    qDebug() << "\n=== End of message ===\n";
}

QByteArray NdefBuilder::toByteArray(const QNdefMessage& msg)
{
    return msg.toByteArray();
}

QNdefMessage NdefBuilder::fromByteArray(const QByteArray& data)
{
    return QNdefMessage::fromByteArray(data);
}
