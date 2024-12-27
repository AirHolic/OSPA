#ifndef SERIALPORTPAGE_H
#define SERIALPORTPAGE_H

#include <QWidget>

namespace Ui {
class SerialPortPage;
}

class SerialPortPage : public QWidget
{
    Q_OBJECT

public:
    explicit SerialPortPage(QWidget *parent = nullptr);
    ~SerialPortPage();

private:
    Ui::SerialPortPage *ui;
};

#endif // SERIALPORTPAGE_H
