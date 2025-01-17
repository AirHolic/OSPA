#ifndef SERIALMULTISENDUNIT_H
#define SERIALMULTISENDUNIT_H

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QPushButton;

class serialMultiSendUnit : public QWidget
{
    Q_OBJECT

public:
    explicit serialMultiSendUnit(const int &unitId, QWidget *parent = nullptr);
    ~serialMultiSendUnit();

    QString getLineText();
    QCheckBox *getCheckBox();
    bool getCheckBoxStatus();
    QLineEdit *getLineEdit();
    QPushButton *getPushButton();

signals:
    void clickPushButton(QString &data);

private:
    int unitId;

    QCheckBox *multiSendUnitCheckBox;
    QLineEdit *multiSendUnitLineEdit;
    QPushButton *multiSendUnitPushButton;


};

#endif // SERIALMULTISENDUNIT_H
