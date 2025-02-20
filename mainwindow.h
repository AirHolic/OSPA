#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QTabWidget>
#include <QDockWidget>
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
    void onTabDockRequested(int index);  // 标签页拖出为窗口
    // 修改槽函数参数类型：由 QWidget* 改为 QDockWidget*
    void onDockTabRequested(QDockWidget *dockWidget); // 窗口拖入为标签页
    void switchLanguage(const QString &languageCode); // 切换语言槽函数

private:
    QTabWidget *tabWidget;              // 标签页控件
    QComboBox *comboBoxSerialPorts;     // 串口选择下拉框
    QList<SerialWidget *> serialPortWidgets; // 串口控件列表
    QList<QSerialPortInfo> availablePorts;   // 可用串口列表

    QAction *openAction;    // 打开串口按钮

    LanguageManager *langManager;      // 多语言管理指针

    QTimer *refreshTimer; // 定时刷新串口列表
};

#endif // MAINWINDOW_H