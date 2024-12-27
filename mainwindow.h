#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QToolBar>
#include <QComboBox>

class SerialPortWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshSerialPorts();
    void openSerialPort();
    void closeSerialPort(int index);

private:
    QTabWidget *tabWidget; // Tab widget to manage multiple serial port widgets
    QList<QSerialPortInfo> availablePorts; // List of available serial ports
    QList<SerialPortWidget*> serialPortWidgets; // List of active serial port widgets

    QToolBar *toolBar; // Toolbar to hold actions and widgets
    QComboBox *comboBoxSerialPorts; // Combo box to display available serial ports
};

#endif // MAINWINDOW_H
