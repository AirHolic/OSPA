#ifndef SERIALWIDGET_H
#define SERIALWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QSerialPort>
#include "serialmanager.h"
#include "serialmultisendunit.h"
#include "serialprotocoltransmission.h"
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

class SerialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialWidget(const QString &portName, QWidget *parent = nullptr);
    ~SerialWidget();

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
    void enableUi(bool enable);
    void loadSettings();
    void saveSettings();
    void updateStatusLabel();
    void logMessage(const QString &message);

    QString portName;
    QSettings *serialSettings;
    QSettings *multiSendSettings;
    SerialManager *serialPortManager;
    qint64 sentBytes;
    qint64 receivedBytes;

    QTimer *multiCycleSendTimer;
    int unitId;

    SerialProtocolTransmission *ymodemWidget;

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

    serialMultiSendUnit *multiSendUnit0;
    serialMultiSendUnit *multiSendUnit1;
    serialMultiSendUnit *multiSendUnit2;
    serialMultiSendUnit *multiSendUnit3;
    serialMultiSendUnit *multiSendUnit4;
    serialMultiSendUnit *multiSendUnit5;
    serialMultiSendUnit *multiSendUnit6;
    serialMultiSendUnit *multiSendUnit7;
    serialMultiSendUnit *multiSendUnit8;
    serialMultiSendUnit *multiSendUnit9;
};

#endif // SERIALWIDGET_H
