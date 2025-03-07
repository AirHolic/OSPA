#include "ThemeManager.h"
#include <QStyleFactory>
#include <QFile>

ThemeManager::ThemeManager(QObject *parent) : QObject(parent), m_currentTheme(LightTheme)
{
}

ThemeManager::~ThemeManager()
{
}

void ThemeManager::applyTheme(Theme theme)
{
    // 先记录当前主题，确保状态正确
    m_currentTheme = theme;
    
    if (theme == LightTheme)
        applyLightTheme();
    else
        applyDarkTheme();

    // 保存主题设置
    saveThemeSettings(theme);
    
    // 发出信号通知主题变更
    emit themeChanged(theme);
}

void ThemeManager::toggleTheme()
{
    // 确保切换逻辑正确
    if (m_currentTheme == LightTheme)
        applyTheme(DarkTheme);
    else
        applyTheme(LightTheme);
}

void ThemeManager::applyLightTheme()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));
    
    QPalette palette;

    // 设置浅色主题颜色
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    palette.setColor(QPalette::WindowText, Qt::black);
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
    palette.setColor(QPalette::ToolTipText, Qt::black);
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(150, 150, 150));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(150, 150, 150));

    qApp->setPalette(palette);
    
    // 加载浅色主题样式表
    QFile file(":/styles/light.qss");
    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        QString style = QLatin1String(file.readAll());
        qApp->setStyleSheet(style);
        file.close();
    } else {
        qApp->setStyleSheet(
            "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"
            "QMenu { background-color: #f0f0f0; border: 1px solid #5d5d5d; }"
            "QMenu::item:selected { background-color: #2a82da; }");
    }
}

void ThemeManager::applyDarkTheme()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    
    // 设置深色主题颜色
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(150, 150, 150));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(150, 150, 150));

    qApp->setPalette(darkPalette);
    
    // 加载深色主题样式表
    QFile file(":/styles/dark.qss");
    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        QString style = QLatin1String(file.readAll());
        qApp->setStyleSheet(style);
        file.close();
    } else {
        // 设置一些基本的样式
        qApp->setStyleSheet(
            "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"
            "QMenu { background-color: #353535; border: 1px solid #5d5d5d; }"
            "QMenu::item:selected { background-color: #2a82da; }"
        );
    }
}

void ThemeManager::saveThemeSettings(Theme theme)
{
    QSettings settings;
    settings.setValue("theme", theme);
}

void ThemeManager::loadSavedTheme()
{
    QSettings settings;
    Theme savedTheme = static_cast<Theme>(settings.value("theme", LightTheme).toInt());
    applyTheme(savedTheme);
}