#include "mainwindow.h"
#include "serialwidget.h"
#include "LanguageManager.h"

#include <QToolBar>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QSerialPortInfo>
#include <QEvent>

MainWindow::MainWindow(LanguageManager *lm, QWidget *parent)
    : QMainWindow(parent), langManager(lm)
{
    // 设置窗口标题
    setWindowTitle(tr("Ordinary Serial Port Assistant"));
    setMinimumSize(800, 600);

    // 创建标签页控件
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);

    // 连接标签页关闭、双击事件
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeSerialPort);
    connect(tabWidget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::onTabDockRequested);

    // 创建工具栏
    QToolBar *toolBar = new QToolBar(this);
    addToolBar(toolBar);
    toolBar->setMovable(false);

    // 添加刷新串口列表按钮
    refreshAction = new QAction(tr("Refresh Serial Ports"), this);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshSerialPorts);
    toolBar->addAction(refreshAction);

    // 添加打开串口按钮
    openAction = new QAction(tr("Open Serial Port"), this);
    connect(openAction, &QAction::triggered, this, &MainWindow::openSerialPort);
    toolBar->addAction(openAction);

    // 添加串口选择下拉框
    comboBoxSerialPorts = new QComboBox(this);
    comboBoxSerialPorts->setMinimumWidth(150);
    toolBar->addWidget(comboBoxSerialPorts);

    // 添加语言切换下拉框
    languageComboBox = new QComboBox(this);
    languageComboBox->setMinimumWidth(120);
    languageComboBox->addItem("简体中文", "zh_CN");
    languageComboBox->addItem("English", "en_US");
    languageComboBox->setCurrentIndex(0); // 默认简体中文
    toolBar->addWidget(languageComboBox);

    // 连接语言下拉框信号
    connect(languageComboBox, &QComboBox::currentTextChanged,
            this, [this](const QString &){
                QString code = languageComboBox->currentData().toString();
                switchLanguage(code);
            });

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
        comboBoxSerialPorts->addItem(port.portName() + " " + port.description());
    }
}

void MainWindow::openSerialPort()
{
    if (availablePorts.isEmpty()) {
        QMessageBox::warning(this, tr("No Serial Ports"), tr("No serial ports available."));
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
    SerialWidget *widget = new SerialWidget(selectedPort, this);
    tabWidget->addTab(widget, selectedPort);
    serialPortWidgets.append(widget);
    tabWidget->setCurrentWidget(widget);

    // 连接串口控件的关闭请求信号
    connect(widget, &SerialWidget::closeRequested, this, &MainWindow::closeSerialPort);
}

void MainWindow::closeSerialPort(int index)
{
    if (index >= 0 && index < tabWidget->count()) {
        SerialWidget *widget = serialPortWidgets.takeAt(index);
        tabWidget->removeTab(index);
        delete widget;
    }
}

void MainWindow::onTabDockRequested(int index)
{
    QWidget *widget = tabWidget->widget(index);
    if (widget) {
        // 创建 QDockWidget，并将标签页中的控件移动到新窗口
        QDockWidget *dockWidget = new QDockWidget(tabWidget->tabText(index), this);
        dockWidget->setAttribute(Qt::WA_DeleteOnClose);
        dockWidget->setWidget(widget);
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);

        // 当窗口由独立状态拖回后重新放回标签页
        connect(dockWidget, &QDockWidget::topLevelChanged, this,
                [this, dockWidget](bool topLevel) {
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
        removeDockWidget(dockWidget);
        tabWidget->addTab(widget, dockWidget->windowTitle());
        dockWidget->deleteLater();
    }
}

void MainWindow::switchLanguage(const QString &languageCode)
{
    if (langManager) {
        langManager->loadLanguage(languageCode);
    }
}

// 重写 changeEvent，在语言切换时更新UI
void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if(event->type() == QEvent::LanguageChange) {
        setWindowTitle(tr("Ordinary Serial Port Assistant"));
        refreshAction->setText(tr("Refresh Serial Ports"));
        openAction->setText(tr("Open Serial Port"));
        // 如果其他控件需要更新文本，也在此添加更新代码
    }
}