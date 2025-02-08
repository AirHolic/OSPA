#ifndef SERIALWIDGET_H
#define SERIALWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QSerialPort>
#include "serialmanager.h"
#include "serialmultisendunit.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTimer>
#include "crc.h"

class QTextEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QLabel;
class QShortcut;
class SearchDialog;
class SerialProtocolTransmission;


class SerialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialWidget(const QString &portName, QWidget *parent = nullptr);
    ~SerialWidget();
    void enableUi(bool enable);
    void logMessage(const QString &message);
    void updateStatusLabel();

    QString portName;
    SerialManager *serialPortManager;
    qint64 sentBytes;
    qint64 receivedBytes;

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
    void onSendTextChanged();

private:
    void initUI();
    void initConnections();
    void initSearchDialog();
    void loadSettings();
    void saveSettings();
    uint calculateCRC(const QByteArray &data, int formatIndex);


    QSettings *serialSettings;
    QSettings *multiSendSettings;

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

    Crc *crc;
    QVBoxLayout *crcLayout;
    QHBoxLayout *crcInsertPositionLayout;
    QCheckBox *crcEnableCheckBox;
    QComboBox *crcFormatComboBox;
    QComboBox *crcInsertPositionComboBox;
    QLabel *crcInsertPositionLabel1;
    QLineEdit *crcCalculateFristSpecificByteLineEdit;
    QLabel *crcInsertPositionLabel2;
    QLineEdit *crcCalculateLastSpecificByteLineEdit;
    QLabel *crcInsertPositionLabel3;
    QLineEdit *crcInsertSpecificByteLineEdit;
    QLabel *crcInsertPositionLabel4;
    QLineEdit *crcResultLineEdit;
    QLabel *crcResultLabel;

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
