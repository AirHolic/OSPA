#include <QApplication>
#include "mainwindow.h"
#include "languagemanager.h"
#include <QDebug>
#include <exception>
#include <QMessageBox>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        break;
    }
    
    QFile outFile("crash_log.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << '\n';
}

int main(int argc, char *argv[])
{
    try {
        qInstallMessageHandler(customMessageHandler);
        
        QApplication a(argc, argv);
        
        // 设置应用程序信息，用于 QSettings
        QCoreApplication::setOrganizationName("YourOrganization");
        QCoreApplication::setOrganizationDomain("yourdomain.com");
        QCoreApplication::setApplicationName("OSPA");

        // 设置全局异常处理
        std::set_terminate([]() {
            QString error = "程序发生致命错误，即将退出。\n";
            if (auto e = std::current_exception()) {
                try {
                    std::rethrow_exception(e);
                } catch (const std::exception& e) {
                    error += QString("错误信息: %1").arg(e.what());
                } catch (...) {
                    error += "未知错误";
                }
            }
            
            qFatal("%s", error.toLocal8Bit().constData());
            QMessageBox::critical(nullptr, "错误", error);
        });

        // 默认加载中文语言包
        LanguageManager langManager;
        if (!langManager.loadLanguage("zh_CN")) {
            qDebug() << "Failed to load default language: zh_CN";
        }

        MainWindow w(&langManager);
        w.show();
        return a.exec();
    }
    catch (const std::exception& e) {
        QString error = QString("程序启动时发生错误：%1").arg(e.what());
        qFatal("%s", error.toLocal8Bit().constData());
        QMessageBox::critical(nullptr, "错误", error);
        return 1;
    }
    catch (...) {
        QString error = "程序启动时发生未知错误";
        qFatal("%s", error.toLocal8Bit().constData());
        QMessageBox::critical(nullptr, "错误", error);
        return 1;
    }
}
