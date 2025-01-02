#include "mainwindow.h"
#include "serialportwidget.h"
#include <QToolBar>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Serial Port Manager");

    // 创建标签页控件
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);
    tabWidget->setTabsClosable(true);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeSerialPort);

    // 连接标签页双击事件
    connect(tabWidget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::onTabDockRequested);

    // 创建工具栏
    QToolBar *toolBar = new QToolBar(this);
    addToolBar(toolBar);

    // 添加刷新串口列表按钮
    QAction *refreshAction = new QAction("Refresh Serial Ports", this);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshSerialPorts);
    toolBar->addAction(refreshAction);

    // 添加打开串口按钮
    QAction *openAction = new QAction("Open Serial Port", this);
    connect(openAction, &QAction::triggered, this, &MainWindow::openSerialPort);
    toolBar->addAction(openAction);

    // 添加串口选择下拉框
    comboBoxSerialPorts = new QComboBox(this);
    comboBoxSerialPorts->setMinimumWidth(150);
    toolBar->addWidget(comboBoxSerialPorts);

    // 刷新串口列表
    refreshSerialPorts();
}

MainWindow::~MainWindow()
{
}

void MainWindow::refreshSerialPorts()
{
    availablePorts = QSerialPortInfo::availablePorts();
    qDebug() << "Available ports:" << availablePorts.size();

    // 更新下拉框中的串口列表
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

    // 获取下拉框中选择的串口名称
    QString selectedPort = comboBoxSerialPorts->currentText();

    // 检查是否已存在相同串口的标签页
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i) == selectedPort) {
            // 切换到已存在的标签页
            tabWidget->setCurrentIndex(i);
            return;
        }
    }

    // 检查是否已存在相同串口的独立窗口
    for (QDockWidget *dockWidget : findChildren<QDockWidget *>()) {
        if (dockWidget->windowTitle() == selectedPort) {
            // 激活并聚焦到该窗口
            dockWidget->raise();
            dockWidget->activateWindow();
            return;
        }
    }

    // 创建新的串口控件
    SerialPortWidget *widget = new SerialPortWidget(selectedPort, this);
    tabWidget->addTab(widget, selectedPort);
    serialPortWidgets.append(widget);
    tabWidget->setCurrentWidget(widget);

    // 连接关闭信号
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

void MainWindow::onTabDockRequested(int index)
{
    QWidget *widget = tabWidget->widget(index);
    if (widget) {
        // 创建 QDockWidget
        QDockWidget *dockWidget = new QDockWidget(tabWidget->tabText(index), this);
        dockWidget->setAttribute(Qt::WA_DeleteOnClose);// 关闭时删除
        dockWidget->setWidget(widget);
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);

        // 移除标签页
        tabWidget->removeTab(index);

        // 连接窗口拖回事件
        connect(dockWidget, &QDockWidget::topLevelChanged, this, [this, dockWidget](bool topLevel) {
            if (!topLevel) {
                onDockTabRequested(dockWidget->widget());
            }
        });
    }
}

void MainWindow::onDockTabRequested(QWidget *widget)
{
    QDockWidget *dockWidget = qobject_cast<QDockWidget *>(widget->parentWidget());
    if (dockWidget) {
        // 移除 QDockWidget
        removeDockWidget(dockWidget);

        // 将控件重新添加到标签页
        tabWidget->addTab(widget, dockWidget->windowTitle());

        // 删除 QDockWidget
        dockWidget->deleteLater();
    }
}