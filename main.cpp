#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QTest>

#include <bitset>

#include "packet.h"
#include "sercontroller.h"

#ifdef TEST
#include "test_packet.h"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef TEST
    QTest::qExec(new Test_Packet, argc, argv);
#endif

    qDebug() << Mertech::Packet::getChecksum(0x02, 0xFF, {0x02, 0x01});

    Mertech::SerController controller("COM1");

    QTimer timer;
    timer.callOnTimeout([&controller] ()
    {
        controller.getWeight();
    });

    timer.setInterval(3000);
    //timer.start();

    return a.exec();
}
