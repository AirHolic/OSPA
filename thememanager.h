#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QSettings>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum Theme {
        LightTheme,
        DarkTheme
    };

    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager();

    void applyTheme(Theme theme);
    Theme currentTheme() const { return m_currentTheme; }
    void toggleTheme();
    void loadSavedTheme();

signals:
    void themeChanged(Theme newTheme);

private:
    void applyLightTheme();
    void applyDarkTheme();
    void saveThemeSettings(Theme theme);

    Theme m_currentTheme;
};

#endif // THEMEMANAGER_H
