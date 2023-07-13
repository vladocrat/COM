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
    //QTest::qExec(new Test_Packet, argc, argv);
    uint8_t bytes[4] = { 0xf5, 0x0, 0x0, 0x0 };
    QByteArray arr;
    QDataStream stream(&arr, QIODevice::ReadWrite);

    for (int i = 0; i < 4; i++) {
        stream << bytes[i];
    }

    float weight = 0.0f;
    qDebug() << arr;

    const uint8_t* data = reinterpret_cast<const uint8_t*>(arr.data());

    uint8_t reversedBytes[4] = { data[3], data[2], data[1], data[0] };

    memcpy(&weight, reversedBytes, sizeof(float));
    qDebug() << "Weight: " << weight; // Output: 3.19000009

#endif
#ifndef TEST
    // qDebug() << Mertech::Packet::getChecksum(0x02, 0xFF, {0x02, 0x01});

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

    QTimer timer;
    timer.callOnTimeout([&controller] ()
    {
        controller.getWeight();
    });

    timer.setInterval(3000);
    //timer.start();
#endif

    return a.exec();
}
