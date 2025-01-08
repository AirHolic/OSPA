#ifndef SERIALPORTMULTISENDUNIT_H
#define SERIALPORTMULTISENDUNIT_H

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QPushButton;

class serialPortMultiSendUnit : public QWidget
{
    Q_OBJECT

public:
    explicit serialPortMultiSendUnit(const int &unitId, QWidget *parent = nullptr);
    ~serialPortMultiSendUnit();

    QString getLineText();
    QCheckBox *getCheckBox();
    bool getCheckBoxStatus();
    QLineEdit *getLineEdit();
    QPushButton *getPushButton();

signals:
    void clickPushButton(QString &data);

private slots:
    void returnPushButton();

private:
    int unitId;

    QCheckBox *multiSendUnitCheckBox;
    QLineEdit *multiSendUnitLineEdit;
    QPushButton *multiSendUnitPushButton;


};

#endif // SERIALPORTMULTISENDUNIT_H
