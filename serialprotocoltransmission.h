#ifndef SERIALPROTOCOLTRANSMISSION_H
#define SERIALPROTOCOLTRANSMISSION_H

#include <QWidget>

namespace Ui {
class serialProtocolTransmission;
}

class serialProtocolTransmission : public QWidget
{
    Q_OBJECT

public:
    explicit serialProtocolTransmission(QWidget *parent = nullptr);
    ~serialProtocolTransmission();

private:
    Ui::serialProtocolTransmission *ui;
};

#endif // SERIALPROTOCOLTRANSMISSION_H
