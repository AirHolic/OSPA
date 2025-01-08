#ifndef SERIALPORTWIDGET_H
#define SERIALPORTWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QSerialPort>
#include "serialportmanager.h"
#include "serialportmultisendunit.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTimer>

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
    void sendAddUi(int index);
    void toggleConnection();
    void sendData();
    void multiSendData(QString &dataStr);
    void multiCycleTimer(int state);
    void multiAutoSendData();
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
    QSettings *serialSettings;
    QSettings *multiSendSettings;
    SerialPortManager *serialPortManager;
    qint64 sentBytes;
    qint64 receivedBytes;

    QTimer *multiCycleSendTimer;
    int unitId;

    // UI elements
    QVBoxLayout *sendSideLayout;
    QVBoxLayout *multiSendSideLayout;
    QVBoxLayout *multiCycleSendSideLayout;
    QTabWidget *sendTabWidget;
    QTextEdit *receiveTextEdit;
    QTextEdit *sendTextEdit;
    QLineEdit *multiCycleSendLineEdit;
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
    QCheckBox *multiCycleSendCheckBox;
    QLabel *statusLabel;
    QLabel *multiCycleSendLabel;
    QLabel *multiCycleSendUnitLabel;
    SearchDialog *searchDialog;
    QShortcut *searchShortcut;

    serialPortMultiSendUnit *multiSendUnit0;
    serialPortMultiSendUnit *multiSendUnit1;
    serialPortMultiSendUnit *multiSendUnit2;
    serialPortMultiSendUnit *multiSendUnit3;
    serialPortMultiSendUnit *multiSendUnit4;
    serialPortMultiSendUnit *multiSendUnit5;
    serialPortMultiSendUnit *multiSendUnit6;
    serialPortMultiSendUnit *multiSendUnit7;
    serialPortMultiSendUnit *multiSendUnit8;
    serialPortMultiSendUnit *multiSendUnit9;
};

#endif // SERIALPORTWIDGET_H
