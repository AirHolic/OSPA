#include "serialmultisendunit.h"
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

serialMultiSendUnit::serialMultiSendUnit(const int &unitId, QWidget *parent)
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
        QString dataStr = getLineText();
        emit clickPushButton(dataStr);
    });
}
serialMultiSendUnit::~serialMultiSendUnit()
{

}

QString serialMultiSendUnit::getLineText()
{
    return multiSendUnitLineEdit->text();
}

bool serialMultiSendUnit::getCheckBoxStatus()
{
    return multiSendUnitCheckBox->checkState();
}

QLineEdit *serialMultiSendUnit::getLineEdit()
{
    return multiSendUnitLineEdit;
}

QCheckBox *serialMultiSendUnit::getCheckBox()
{
    return multiSendUnitCheckBox;
}

QPushButton *serialMultiSendUnit::getPushButton()
{
    return multiSendUnitPushButton;
}
