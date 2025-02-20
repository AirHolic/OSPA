#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QTabWidget>
#include <QDockWidget>
#include <QSplitter>
#include <QList>
#include <QTimer>
#include "serialwidget.h"

class LanguageManager; // 前向声明

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 构造函数增加 LanguageManager 指针参数
    explicit MainWindow(LanguageManager *lm, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 重写 changeEvent，用于在语言切换时更新UI界面
    void changeEvent(QEvent *event) override;

private slots:
    void refreshSerialPorts();           // 自动刷新串口列表
    void openSerialPort();               // 打开串口
    void closeSerialPort(int index);     // 关闭串口
    void onTabSplitRequested(int index); // 标签页分离请求
    void switchLanguage(const QString &languageCode); // 切换语言槽函数

private:
    void closeSerialPortFromTab(QTabWidget *tWidget, int index);
    void adjustSplitterSizes();          // 调整分割器大小

    QSplitter *centralSplitter;
    QTabWidget *tabWidget;
    QComboBox *comboBoxSerialPorts;
    QList<SerialWidget *> serialPortWidgets;
    QList<QSerialPortInfo> availablePorts;

    QAction *openAction;

    LanguageManager *langManager;
    QTimer *refreshTimer;
};

#endif // MAINWINDOW_H