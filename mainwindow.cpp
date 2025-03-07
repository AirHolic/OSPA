#include "mainwindow.h"
#include "serialwidget.h"
#include "languagemanager.h"

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
#include <QSplitter>
#include <QCoreApplication>

MainWindow::MainWindow(LanguageManager *lm, QWidget *parent)
    : QMainWindow(parent), langManager(lm)
{
    // 设置窗口标题与大小
    setWindowTitle(tr("Ordinary Serial Port Assistant"));
    setMinimumSize(800, 600);

    // 创建中央分割器，并将初始标签页控件添加进去
    centralSplitter = new QSplitter(Qt::Horizontal, this);
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    centralSplitter->addWidget(tabWidget);
    setCentralWidget(centralSplitter);

    // 连接标签页的关闭与双击事件（双击时分割）
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index){
        closeSerialPortFromTab(tabWidget, index);
    });
    connect(tabWidget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::onTabSplitRequested);

    // 创建工具栏
    QToolBar *toolBar = new QToolBar(this);
    addToolBar(toolBar);
    toolBar->setMovable(false);

    // 添加打开串口按钮
    openAction = new QAction(tr("Open Serial Port"), this);
    connect(openAction, &QAction::triggered, this, &MainWindow::openSerialPort);
    toolBar->addAction(openAction);

    // 串口选择下拉框
    comboBoxSerialPorts = new QComboBox(this);
    comboBoxSerialPorts->setMinimumWidth(150);
    toolBar->addWidget(comboBoxSerialPorts);

    // 添加伸缩 spacer 使后面控件靠右
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);

    m_themeManager = new ThemeManager(this);

    // 创建视图按钮
    viewButton = new QToolButton(this);
    viewButton->setText(tr("视图"));
    viewButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    viewButton->setArrowType(Qt::NoArrow);
    viewButton->setPopupMode(QToolButton::InstantPopup);

    // 创建视图菜单和主题子菜单
    QMenu *viewMenu = new QMenu(this);
    QMenu *themeMenu = viewMenu->addMenu(tr("主题"));
    
    QAction *lightThemeAction = themeMenu->addAction(tr("浅色模式"));
    connect(lightThemeAction, &QAction::triggered, this, &MainWindow::switchToLightTheme);
    
    QAction *darkThemeAction = themeMenu->addAction(tr("深色模式"));
    connect(darkThemeAction, &QAction::triggered, this, &MainWindow::switchToDarkTheme);
    
    viewMenu->addSeparator();
    
    QAction *toggleThemeAction = viewMenu->addAction(tr("切换主题"));
    toggleThemeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    connect(toggleThemeAction, &QAction::triggered, this, &MainWindow::toggleTheme);

    viewButton->setMenu(viewMenu);
    toolBar->addWidget(viewButton);

    // 语言切换工具按钮
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

    // 切换语言信号
    connect(languageActionGroup, &QActionGroup::triggered, this, [this](QAction *action){
        QString code = action->data().toString();
        switchLanguage(code);
    });

    // 自动刷新串口列表定时器
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshSerialPorts);
    refreshTimer->start(3000); // 每3000ms刷新一次

    // 初次刷新串口列表
    refreshSerialPorts();
    
    connect(m_themeManager, &ThemeManager::themeChanged, this, &MainWindow::onThemeChanged);
    m_themeManager->loadSavedTheme(); // 加载保存的主题设置
    
    
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

    // 检查各个 QTabWidget 中是否已存在相同串口
    QList<QTabWidget*> tabWidgets = centralSplitter->findChildren<QTabWidget*>();
    for(auto *tWidget : tabWidgets) {
        for (int i = 0; i < tWidget->count(); ++i) {
            if (tWidget->tabText(i) == selectedPort) {
                tWidget->setCurrentIndex(i);
                return;
            }
        }
    }
    // 创建新的串口控件，并添加到初始标签页控件中
    SerialWidget *widget = new SerialWidget(selectedPort, this);
    tabWidget->addTab(widget, selectedPort);
    serialPortWidgets.append(widget);
    tabWidget->setCurrentWidget(widget);
    // 注意：初始的 tabWidget 的关闭信号已在构造函数中连接
}

void MainWindow::closeSerialPort(int index)
{
    // 保留原接口，暂时不再使用
    closeSerialPortFromTab(tabWidget, index);
}

void MainWindow::closeSerialPortFromTab(QTabWidget *tWidget, int index)
{
    if (index < 0 || index >= tWidget->count())
        return;
    
    SerialWidget *widget = qobject_cast<SerialWidget*>(tWidget->widget(index));
    if (!widget) return;

    // 从列表中移除
    int listIdx = serialPortWidgets.indexOf(widget);
    if (listIdx != -1)
        serialPortWidgets.removeAt(listIdx);

    // 从标签页移除
    tWidget->removeTab(index);
    
    // 删除 widget
    widget->deleteLater();

    // 特殊处理主标签区
    if (tWidget == tabWidget && tWidget->count() == 0) {
        // 当主标签区为空时，查找其他分割区域
        QList<QTabWidget*> otherTabs = centralSplitter->findChildren<QTabWidget*>();
        QTabWidget* newMainTab = nullptr;

        // 找到第一个非空的分割区域设为主标签区
        for (QTabWidget* other : otherTabs) {
            if (other != tabWidget && other->count() > 0) {
                // 检查该标签页是否包含有效的 SerialWidget
                bool hasValidSerialWidget = false;
                for(int i = 0; i < other->count(); i++) {
                    if(qobject_cast<SerialWidget*>(other->widget(i))) {
                        hasValidSerialWidget = true;
                        break;
                    }
                }
                if(hasValidSerialWidget) {
                    newMainTab = other;
                    break;
                }
            }
            // else if (other == tabWidget) {
            //     continue;
            // }
            // else if (other->count() == 0) {
            //     other->setParent(nullptr);
            //     other->deleteLater();
            // }
        }
        // 如果找到合适的分割区域
        if (newMainTab) {
            qDebug() << "New main tab found: " << newMainTab->tabText(0);
            // 将找到的分割区域设为主标签区
            int oldIndex = centralSplitter->indexOf(newMainTab);
            centralSplitter->insertWidget(0, newMainTab); // 移到第一个位置
            delete tabWidget; // 删除原主标签区
            tabWidget = newMainTab; // 更新主标签区指针

            // 调整分割区域大小
            QMetaObject::invokeMethod(this, [this]() {
                adjustSplitterSizes();
            }, Qt::QueuedConnection);
        }
        return;
    }

    // 处理其他分割区域
    if (tWidget != tabWidget) {
        if (tWidget->count() == 0) {
            tWidget->setParent(nullptr);
            tWidget->deleteLater();
        }
        else if (tWidget->count() == 1) {
            QWidget* w = tWidget->widget(0);
            QString title = tWidget->tabText(0);
            tWidget->removeTab(0);
            
            tabWidget->addTab(w, title);
            if (SerialWidget* sw = qobject_cast<SerialWidget*>(w)) {
                if (!serialPortWidgets.contains(sw)) {
                    serialPortWidgets.append(sw);
                }
            }

            tWidget->setParent(nullptr);
            tWidget->deleteLater();
        }

        // 调整分割区域大小
        QMetaObject::invokeMethod(this, [this]() {
            adjustSplitterSizes();
        }, Qt::QueuedConnection);
    }
}

void MainWindow::onTabSplitRequested(int index)
{
    QTabWidget *srcTabWidget = qobject_cast<QTabWidget*>(sender());
    if (!srcTabWidget)
        srcTabWidget = tabWidget;

    // 只有当待分割区域内至少有 2 个标签时才进行分割操作
    if (srcTabWidget->count() <= 1)
        return;

    if(index < 0)
        return;

    SerialWidget *widget = qobject_cast<SerialWidget*>(srcTabWidget->widget(index));
    if (!widget)
        return;
    QString tabTitle = srcTabWidget->tabText(index);

    // 从当前 QTabWidget 移除该标签，并更新 serialPortWidgets
    srcTabWidget->removeTab(index);
    int listIdx = serialPortWidgets.indexOf(widget);
    if (listIdx != -1)
        serialPortWidgets.removeAt(listIdx);

    // 新建一个 QTabWidget作为分割区域，并设置其属性
    QTabWidget *newTabWidget = new QTabWidget();
    newTabWidget->setTabsClosable(true);
    newTabWidget->setMovable(true);
    newTabWidget->addTab(widget, tabTitle);
    serialPortWidgets.append(widget);

    // 连接新建区域的关闭与双击信号
    connect(newTabWidget, &QTabWidget::tabCloseRequested, this, [this, newTabWidget](int idx){
        closeSerialPortFromTab(newTabWidget, idx);
    });
    connect(newTabWidget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::onTabSplitRequested);

    // 将新建的 QTabWidget 添加到中央分割器中
    // 如果是从主标签区分割，添加到末尾；否则添加到源标签区域之后
    int insertIndex = centralSplitter->indexOf(srcTabWidget) + 1;
    if (insertIndex <= 0) insertIndex = centralSplitter->count();
    centralSplitter->insertWidget(insertIndex, newTabWidget);
    
    // 调整分割区域大小
    adjustSplitterSizes();
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
        openAction->setText(tr("Open Serial Port"));
        
        // 如果其他控件需要更新文本，也在此添加更新代码
    }
}

void MainWindow::adjustSplitterSizes()
{
    int count = centralSplitter->count();
    if (count <= 0) return;

    // 移除所有大小为0的分割区域
    for (int i = count - 1; i >= 0; --i) {
        QWidget *widget = centralSplitter->widget(i);
        if (widget && widget->width() == 0) {
            widget->setParent(nullptr);
            widget->deleteLater();
        }
    }

    // 重新获取分割区域数量
    count = centralSplitter->count();
    qDebug() << "Current splitter count:" << count;
    if (count <= 0) return;

    // 计算可用宽度
    int availableWidth = centralSplitter->width();
    int widthPerWidget = availableWidth / count;

    // 设置每个分割区域的大小
    QList<int> sizes;
    for (int i = 0; i < count; ++i) {
        sizes << widthPerWidget;
    }
    
    // 应用新的大小
    centralSplitter->setSizes(sizes);
    
    // 强制更新布局
    centralSplitter->updateGeometry();
    centralSplitter->update();
}

void MainWindow::setupViewButton()
{
    // 删除原来的菜单栏中的视图菜单

    // 创建视图工具按钮
    QToolButton *viewButton = new QToolButton(this);
    viewButton->setText(tr("视图"));
    viewButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    viewButton->setArrowType(Qt::NoArrow);
    viewButton->setPopupMode(QToolButton::InstantPopup);

    // 创建视图菜单和主题子菜单
    QMenu *viewMenu = new QMenu(this);
    QMenu *themeMenu = viewMenu->addMenu(tr("主题"));
    
    QAction *lightThemeAction = themeMenu->addAction(tr("浅色模式"));
    connect(lightThemeAction, &QAction::triggered, this, &MainWindow::switchToLightTheme);
    
    QAction *darkThemeAction = themeMenu->addAction(tr("深色模式"));
    connect(darkThemeAction, &QAction::triggered, this, &MainWindow::switchToDarkTheme);
    
    viewMenu->addSeparator();
    
    QAction *toggleThemeAction = viewMenu->addAction(tr("切换主题"));
    toggleThemeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    connect(toggleThemeAction, &QAction::triggered, this, &MainWindow::toggleTheme);

    viewButton->setMenu(viewMenu);
}

void MainWindow::onThemeChanged(ThemeManager::Theme theme)
{
    Q_UNUSED(theme);
    // 可以在这里处理主题变更后的任何特定UI更新
}


void MainWindow::switchToLightTheme()
{
    if (m_themeManager) {
        qDebug() << "切换到浅色主题";
        m_themeManager->applyTheme(ThemeManager::LightTheme);
    }
}

void MainWindow::switchToDarkTheme()
{
    if (m_themeManager) {
        qDebug() << "切换到深色主题";
        m_themeManager->applyTheme(ThemeManager::DarkTheme);
    }
}

void MainWindow::toggleTheme()
{
    if (m_themeManager) {
        qDebug() << "切换主题，当前主题:" << (m_themeManager->currentTheme() == ThemeManager::LightTheme ? "浅色" : "深色");
        m_themeManager->toggleTheme();
    }
}
