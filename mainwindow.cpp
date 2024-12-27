#include "mainwindow.h"
#include "serialportwidget.h"
#include <QToolBar>
#include <QComboBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Serial Port Manager");

    // Create the tab widget
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);
    tabWidget->setTabsClosable(true);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeSerialPort);

    // Create toolbar
    QToolBar *toolBar = new QToolBar(this);
    addToolBar(toolBar);

    // Add refresh action to toolbar
    QAction *refreshAction = new QAction("Refresh Serial Ports", this);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshSerialPorts);
    toolBar->addAction(refreshAction);

    // Add open action to toolbar
    QAction *openAction = new QAction("Open Serial Port", this);
    connect(openAction, &QAction::triggered, this, &MainWindow::openSerialPort);
    toolBar->addAction(openAction);

    // Add combo box for serial ports
    comboBoxSerialPorts = new QComboBox(this);
    comboBoxSerialPorts->setMinimumWidth(150);
    toolBar->addWidget(comboBoxSerialPorts);

    // Refresh serial ports list
    refreshSerialPorts();
}

MainWindow::~MainWindow()
{
}

void MainWindow::refreshSerialPorts()
{
    availablePorts = QSerialPortInfo::availablePorts();
    qDebug() << "Available ports:" << availablePorts.size();

    // Update the combo box with available ports
    comboBoxSerialPorts->clear();
    for (const QSerialPortInfo &port : availablePorts) {
        comboBoxSerialPorts->addItem(port.portName());
    }
}

void MainWindow::openSerialPort()
{
    if (availablePorts.isEmpty()) {
        QMessageBox::warning(this, "No Serial Ports", "No serial ports available.");
        return;
    }

    // Get the selected port name from the combo box
    QString selectedPort = comboBoxSerialPorts->currentText();

    // Check if a tab with the same port name already exists
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i) == selectedPort) {
            // Switch to the existing tab
            tabWidget->setCurrentIndex(i);
            return; // Exit the function without creating a new tab
        }
    }

    // Create a new serial port widget
    SerialPortWidget *widget = new SerialPortWidget(selectedPort, this);
    tabWidget->addTab(widget, selectedPort);
    serialPortWidgets.append(widget);
    tabWidget->setCurrentWidget(widget);

    // Connect the close signal
    connect(widget, &SerialPortWidget::closeRequested, this, &MainWindow::closeSerialPort);
}

void MainWindow::closeSerialPort(int index)
{
    if (index >= 0 && index < tabWidget->count()) {
        SerialPortWidget *widget = serialPortWidgets.takeAt(index);
        tabWidget->removeTab(index);
        delete widget;
    }
}
