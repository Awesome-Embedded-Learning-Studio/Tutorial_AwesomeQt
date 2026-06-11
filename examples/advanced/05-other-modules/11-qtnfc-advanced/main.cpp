/// @file    main.cpp
/// @brief   Console demo of NDEF message building, serialization, and parsing.
///
/// Runs five self-contained demos without any NFC hardware and then exits.

#include "ndef_builder.h"

#include <QCoreApplication>
#include <QDebug>
#include <QNdefMessage>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include <QNdefRecord>
#include <QUrl>

/// @brief Demo 1 — Build a single text NDEF record and print its contents.
/// @param[in] builder  The NdefBuilder instance to use.
static void demoTextRecord(const NdefBuilder& builder)
{
    qDebug() << "==== Demo 1: Text Record ====";

    auto record = builder.buildTextRecord(
        QStringLiteral("Hello NFC World!"), QStringLiteral("en"));

    qDebug() << "Text   :" << record.text();
    qDebug() << "Locale :" << record.locale();
    qDebug() << "TNF    :" << record.typeNameFormat();
    qDebug() << "Type   :" << record.type();
    qDebug() << "";
}

/// @brief Demo 2 — Build a single URI NDEF record and print its contents.
/// @param[in] builder  The NdefBuilder instance to use.
static void demoUriRecord(const NdefBuilder& builder)
{
    qDebug() << "==== Demo 2: URI Record ====";

    auto record = builder.buildUriRecord(
        QUrl(QStringLiteral("https://www.qt.io")));

    qDebug() << "URI    :" << record.uri().toString();
    qDebug() << "TNF    :" << record.typeNameFormat();
    qDebug() << "Type   :" << record.type();
    qDebug() << "";
}

/// @brief Demo 3 — Build a composite message containing text + URI records.
/// @param[in] builder  The NdefBuilder instance to use.
static void demoCompositeMessage(const NdefBuilder& builder)
{
    qDebug() << "==== Demo 3: Composite Message ====";

    auto message = builder.buildCompositeMessage();
    builder.parseMessage(message);
}

/// @brief Demo 4 — Serialize a message to bytes and parse it back.
/// @param[in] builder  The NdefBuilder instance to use.
/// @note This is the key round-trip path that proves serialization fidelity.
static void demoSerializeRoundTrip(const NdefBuilder& builder)
{
    qDebug() << "==== Demo 4: Serialize & Round-Trip ====";

    auto original = builder.buildCompositeMessage();

    // Serialize to raw bytes — same bytes an NFC tag would store.
    QByteArray raw = NdefBuilder::toByteArray(original);
    qDebug() << "Serialized size:" << raw.size() << "bytes";
    qDebug() << "Raw hex:" << raw.toHex(' ');
    qDebug() << "";

    // Deserialize back into a QNdefMessage and inspect.
    auto restored = NdefBuilder::fromByteArray(raw);
    builder.parseMessage(restored);

    // Verify fidelity: record count must match.
    if (original.size() == restored.size()) {
        qDebug() << "Round-trip OK — record counts match.";
    } else {
        qDebug() << "Round-trip MISMATCH — original:" << original.size()
                 << "vs restored:" << restored.size();
    }
    qDebug() << "";
}

/// @brief Demo 5 — Enumerate NDEF TNF values and show record type matching.
/// @note TNF (Type Name Format) is the first byte of every NDEF record
///       header; it tells the parser how to interpret the type field.
static void demoNdefRecordTypes()
{
    qDebug() << "==== Demo 5: NDEF Record Types & TNF ====";

    // Show all TNF enum values defined by QtNfc.
    qDebug() << "QNdefRecord::TypeNameFormat enum values:";
    qDebug() << "  Empty     =" << QNdefRecord::Empty;
    qDebug() << "  NfcRtd    =" << QNdefRecord::NfcRtd;
    qDebug() << "  Mime      =" << QNdefRecord::Mime;
    qDebug() << "  Uri       =" << QNdefRecord::Uri;
    qDebug() << "  ExternalRtd=" << QNdefRecord::ExternalRtd;
    qDebug() << "  Unknown   =" << QNdefRecord::Unknown;
    qDebug() << "";

    // Demonstrate isRecordType<T>() dispatch.
    QNdefNfcTextRecord textRec;
    textRec.setText(QStringLiteral("type check"));
    textRec.setLocale(QStringLiteral("en"));

    // Copy the base record to test type detection.
    QNdefRecord base = textRec;
    qDebug() << "isRecordType<QNdefNfcTextRecord>:"
             << base.isRecordType<QNdefNfcTextRecord>();
    qDebug() << "isRecordType<QNdefNfcUriRecord> :"
             << base.isRecordType<QNdefNfcUriRecord>();
    qDebug() << "";
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    NdefBuilder builder;

    demoTextRecord(builder);
    demoUriRecord(builder);
    demoCompositeMessage(builder);
    demoSerializeRoundTrip(builder);
    demoNdefRecordTypes();

    qDebug() << "All demos complete. Exiting.";

    // Auto-quit — no event loop needed, all operations are synchronous.
    return 0;
}
