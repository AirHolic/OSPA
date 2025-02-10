#include <QApplication>
#include "mainwindow.h"
#include "LanguageManager.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 默认加载中文语言包
    LanguageManager langManager;
    if (!langManager.loadLanguage("zh_CN")) {
        qDebug() << "Failed to load default language: zh_CN";
    }

    MainWindow w(&langManager);
    w.show();
    return a.exec();
}
