#include "serialprotocoltransmission.h"
#include "ui_serialprotocoltransmission.h"

serialProtocolTransmission::serialProtocolTransmission(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::serialProtocolTransmission)
{
    ui->setupUi(this);
}

serialProtocolTransmission::~serialProtocolTransmission()
{
    delete ui;
}
