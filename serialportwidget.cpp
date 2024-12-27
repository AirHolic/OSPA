#include "serialportpage.h"
#include "ui_serialportpage.h"

SerialPortPage::SerialPortPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialPortPage)
{
    ui->setupUi(this);
}

SerialPortPage::~SerialPortPage()
{
    delete ui;
}
