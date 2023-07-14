#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QTest>
#include <QSerialPortInfo>

#include <bitset>

#include "packet.h"
#include "sercontroller.h"

#ifdef TEST
#include "test_packet.h"
#include <QDataStream>
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef TEST
    QTest::qExec(new Test_Packet, argc, argv);
#endif
#ifndef TEST
    // qDebug() << Mertech::Packet::getChecksum(0x02, 0xFF, {0x02, 0x01});

    Mertech::SerController controller("COM1");

    QList<QSerialPortInfo> L = QSerialPortInfo::availablePorts();

    for (const auto& rate : L)
    {
        qDebug() << rate.standardBaudRates();
    }

    Mertech::SerController controller("COM3", QSerialPort::Baud9600);

    if (!controller.open())
    {
        qDebug() << "failed to open device";
    }

    qDebug() << "Opened device successfully";

    if (!controller.getWeight())
    {
        qDebug() << "failed to get weight";
    }
#endif


    return a.exec();
}
