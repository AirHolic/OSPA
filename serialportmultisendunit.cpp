#include "serialportmultisendunit.h"
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

serialPortMultiSendUnit::serialPortMultiSendUnit(const int &unitId, QWidget *parent)
    : QWidget(parent), unitId(unitId)
{
    QHBoxLayout *multiSendUnit = new QHBoxLayout(this);
    multiSendUnitCheckBox = new QCheckBox("", this);
    multiSendUnitLineEdit = new QLineEdit(this);
    multiSendUnitPushButton = new QPushButton(QString::number(unitId), this);
    multiSendUnit->addWidget(multiSendUnitCheckBox);
    multiSendUnit->addWidget(multiSendUnitLineEdit);
    multiSendUnit->addWidget(multiSendUnitPushButton);

    connect(multiSendUnitPushButton, &QPushButton::clicked, this, [this](){
        emit returnPushButton();
    });
}
serialPortMultiSendUnit::~serialPortMultiSendUnit()
{

}

QString serialPortMultiSendUnit::getLineText()
{
    return multiSendUnitLineEdit->text();
}

bool serialPortMultiSendUnit::getCheckBoxStatus()
{
    return multiSendUnitCheckBox->checkState();
}

QLineEdit *serialPortMultiSendUnit::getLineEdit()
{
    return multiSendUnitLineEdit;
}

QCheckBox *serialPortMultiSendUnit::getCheckBox()
{
    return multiSendUnitCheckBox;
}

QPushButton *serialPortMultiSendUnit::getPushButton()
{
    return multiSendUnitPushButton;
}

void serialPortMultiSendUnit::returnPushButton()
{
    QString dataStr = getLineText();
    emit clickPushButton(dataStr);
}
