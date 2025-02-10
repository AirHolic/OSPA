#include "LanguageManager.h"
#include <QCoreApplication>
#include <QDebug>

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent), translator(new QTranslator(this)), currentLang("en")
{
    // 可根据需要加载默认语言
}

LanguageManager::~LanguageManager()
{
    // QTranslator 由父对象管理，无需手动删除
}

bool LanguageManager::loadLanguage(const QString &languageCode)
{
    // 假设 qm 文件命名规则为 "app_<languageCode>.qm", 且位于资源文件中 ":/i18n/"
    QString qmFile = QString(":/res/language/%1.qm").arg(languageCode);
    
    // 移除之前加载的翻译器
    qApp->removeTranslator(translator);
    if (translator->load(qmFile)) {
        qApp->installTranslator(translator);
        currentLang = languageCode;
        emit languageChanged();
        qDebug() << "Loaded language:" << languageCode;
        return true;
    } else {
        qDebug() << "Failed to load language file:" << qmFile;
        return false;
    }
}

QString LanguageManager::currentLanguage() const
{
    return currentLang;
}
