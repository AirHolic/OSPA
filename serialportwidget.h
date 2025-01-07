#ifndef SERIALPORTWIDGET_H
#define SERIALPORTWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QSerialPort>
#include "serialportmanager.h"
#include <QTabWidget>

class QVBoxLayout;
class QTextEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QLabel;
class QShortcut;
class SearchDialog;

class SerialPortWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialPortWidget(const QString &portName, QWidget *parent = nullptr);
    ~SerialPortWidget();

signals:
    void closeRequested(int index); // 请求关闭信号

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void toggleConnection();
    void sendData();
    void openSearchDialog();
    void clearReceiveArea();
    void onDataReceived(const QByteArray &data);
    void onErrorOccurred(const QString &error);

private:
    void initUI();
    void initConnections();
    void initSearchDialog();
    void loadSettings();
    void saveSettings();
    void updateStatusLabel();
    void logMessage(const QString &message);

    QString portName;
    QSettings *settings;
    SerialPortManager *serialPortManager;
    qint64 sentBytes;
    qint64 receivedBytes;

    // UI elements
    QTabWidget *sendTabWidget;
    QTextEdit *receiveTextEdit;
    QTextEdit *sendTextEdit;
    QComboBox *baudRateComboBox;
    QComboBox *dataBitsComboBox;
    QComboBox *parityComboBox;
    QComboBox *stopBitsComboBox;
    QComboBox *flowControlComboBox;
    QPushButton *connectButton;
    QPushButton *sendButton;
    QPushButton *searchButton;
    QPushButton *clearReceiveButton;
    QCheckBox *hexReceiveCheckBox;
    QCheckBox *hexSendCheckBox;
    QCheckBox *sendNewRowCheckbox;
    QLabel *statusLabel;
    SearchDialog *searchDialog;
    QShortcut *searchShortcut;
};

#endif // SERIALPORTWIDGET_H
