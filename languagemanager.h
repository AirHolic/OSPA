#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QString>

class LanguageManager : public QObject
{
    Q_OBJECT
public:
    explicit LanguageManager(QObject *parent = nullptr);
    ~LanguageManager();

    // 通过加载对应语言的 qm 文件切换语言，返回加载是否成功
    bool loadLanguage(const QString &languageCode);

    QString currentLanguage() const;

signals:
    // 当语言切换后发出此信号，可用于更新界面
    void languageChanged();

private:
    QTranslator *translator;
    QString currentLang;
};

#endif // LANGUAGEMANAGER_H
