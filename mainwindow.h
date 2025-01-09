#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QTabWidget>
#include <QDockWidget>
#include <QList>
#include "serialwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshSerialPorts(); // 刷新串口列表
    void openSerialPort();     // 打开串口
    void closeSerialPort(int index); // 关闭串口
    void onTabDockRequested(int index); // 标签页拖出为窗口
    void onDockTabRequested(QWidget *widget); // 窗口拖入为标签页

private:
    QTabWidget *tabWidget; // 标签页控件
    QComboBox *comboBoxSerialPorts; // 串口选择下拉框
    QList<SerialWidget *> serialPortWidgets; // 串口控件列表
    QList<QSerialPortInfo> availablePorts; // 可用串口列表
};

#endif // MAINWINDOW_H