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
#include <QMenuBar>
#include <QActionGroup>
#include <QTimer>
#include <QToolButton>

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

    // 创建统一的工具栏（整合串口操作和语言切换）
    QToolBar *toolBar = new QToolBar(this);
    addToolBar(toolBar);
    toolBar->setMovable(false);

    // 添加打开串口按钮
    openAction = new QAction(tr("Open Serial Port"), this);
    connect(openAction, &QAction::triggered, this, &MainWindow::openSerialPort);
    toolBar->addAction(openAction);

    // 添加串口选择下拉框
    comboBoxSerialPorts = new QComboBox(this);
    comboBoxSerialPorts->setMinimumWidth(150);
    toolBar->addWidget(comboBoxSerialPorts);

    // 在添加串口下拉框后，添加一个伸缩 spacer 推动后面的控件到最右侧
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);

    // 添加语言按钮（仅显示文本，无下拉箭头）
    QToolButton *languageButton = new QToolButton(this);
    languageButton->setText(tr("Language"));
    languageButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    languageButton->setArrowType(Qt::NoArrow);
    languageButton->setPopupMode(QToolButton::InstantPopup);   // 菜单会在点击按钮后直接弹出

    // 创建语言切换菜单
    QMenu *languageMenu = new QMenu(this);
    QActionGroup *languageActionGroup = new QActionGroup(this);

    QAction *actionZh = new QAction("简体中文", this);
    actionZh->setData("zh_CN");
    actionZh->setCheckable(true);
    actionZh->setChecked(true);
    languageActionGroup->addAction(actionZh);
    languageMenu->addAction(actionZh);

    QAction *actionEn = new QAction("English", this);
    actionEn->setData("en_US");
    actionEn->setCheckable(true);
    languageActionGroup->addAction(actionEn);
    languageMenu->addAction(actionEn);

    languageButton->setMenu(languageMenu);
    toolBar->addWidget(languageButton);

    // 点击动作切换语言
    connect(languageActionGroup, &QActionGroup::triggered, this, [this](QAction *action){
        QString code = action->data().toString();
        switchLanguage(code);
    });

    // 自动定时刷新串口列表
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshSerialPorts);
    refreshTimer->start(3000); // 每3000ms刷新一次

    // 初次刷新串口列表
    refreshSerialPorts();
}

MainWindow::~MainWindow()
{
}

void MainWindow::refreshSerialPorts()
{
    availablePorts = QSerialPortInfo::availablePorts();
    qDebug() << "Available ports:" << availablePorts.size();

    // 保存原来的选项文本
    QString currentSelection = comboBoxSerialPorts->currentText();

    // 更新串口下拉框内容
    comboBoxSerialPorts->clear();
    int indexToSelect = -1;
    int index = 0;
    for (const QSerialPortInfo &port : availablePorts) {
        QString item = port.portName() + " " + port.description();
        comboBoxSerialPorts->addItem(item);
        if (item == currentSelection)
        {
            indexToSelect = index;
        }
        ++index;
    }
    if (indexToSelect != -1)
        comboBoxSerialPorts->setCurrentIndex(indexToSelect);
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
    //connect(widget, &SerialWidget::closeRequested, this, &MainWindow::closeSerialPort);
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
    // 获取标签页对应的 SerialWidget
    SerialWidget *widget = qobject_cast<SerialWidget *>(tabWidget->widget(index));
    if (!widget) {
        return;
    }
    // 保存原标签页标题
    QString tabTitle = tabWidget->tabText(index);
    // 从标签页中移除并更新 serialPortWidgets
    tabWidget->removeTab(index);
    int listIndex = serialPortWidgets.indexOf(widget);
    if(listIndex != -1){
        serialPortWidgets.removeAt(listIndex);
    }

    // 创建 QDockWidget，将该 widget 移入独立窗口
    QDockWidget *dockWidget = new QDockWidget(tabTitle, this);
    dockWidget->setAttribute(Qt::WA_DeleteOnClose);
    dockWidget->setWidget(widget);
    addDockWidget(Qt::RightDockWidgetArea, dockWidget);

    // 当独立窗口拖拽回标签区域时重新放回标签页
    connect(dockWidget, &QDockWidget::topLevelChanged, this,
            [this, dockWidget](bool topLevel) {
                if (!topLevel) {
                    onDockTabRequested(dockWidget);
                }
            });
}

void MainWindow::onDockTabRequested(QDockWidget *dockWidget)
{
    QWidget *widget = dockWidget->widget();
    if (widget) {
        removeDockWidget(dockWidget);
        tabWidget->addTab(widget, dockWidget->windowTitle());
        serialPortWidgets.append(qobject_cast<SerialWidget *>(widget));
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
    if (event->type() == QEvent::LanguageChange) {
        setWindowTitle(tr("Ordinary Serial Port Assistant"));
        // 仅更新存在的控件（由于刷新按钮已移除，此处不再更新其文本）
        openAction->setText(tr("Open Serial Port"));
        
        // 如果其他控件需要更新文本，也在此添加更新代码
    }
}
