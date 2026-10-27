#include "ttsutils.h"

#include <QDebug>
#include <QLocale>
#include <QTextToSpeech>

void printAvailableEngineInfo(QTextToSpeech &tts)
{
    qDebug() << "=== 可用语言列表 ===";
    QList<QLocale> locales = tts.availableLocales();
    for (const QLocale &locale : locales) {
        qDebug() << " " << locale.name()
                 << locale.nativeLanguageName()
                 << locale.nativeTerritoryName();
    }

    qDebug() << "";
    qDebug() << "=== 当前语言下的可用声音 ===";
    QList<QVoice> voices = tts.availableVoices();
    for (const QVoice &voice : voices) {
        qDebug() << " " << voice.name()
                 << "gender:" << voice.gender()
                 << "age:" << voice.age();
    }
    qDebug() << "";
}
