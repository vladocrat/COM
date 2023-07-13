#include "sercontroller.h"
#include "packet.h"
#include "protocol.h"

#include <QDataStream>
#include <QVector>
#include <QDebug>

namespace Mertech
{

namespace Constants
{
static const QVector<uint8_t> PASSWORD_PARAMS {0x30, 0x30, 0x33, 0x30};
} // Constants

struct SerController::impl_t
{
    SerialPort m_ser;
};

SerController::SerController(const QString& name, QSerialPort::BaudRate rate)
{
    createImpl();

    impl().m_ser.setPortName(name);
    impl().m_ser.setPullingRate(rate);

    QObject::connect(&impl().m_ser, &SerialPort::weightChannelInfoRecieved, [](WeightChannel& channel)
    {
        qDebug() << "data recieved";
        qDebug() << channel.weight;
        qDebug() << channel.tare;
    });
}

SerController::~SerController() noexcept
{

}

bool SerController::open()
{
    return impl().m_ser.open(QIODevice::ReadWrite);
}

bool SerController::getWeight() noexcept
{
    if (!impl().m_ser.readySend())
    {
        qDebug() << "device is not ready yet";
        return false;
    }

    Packet pkt;
    pkt.setMessageType(Packet::MessageType::STX);
    pkt.setCommand(Protocol::Command::WEIGHT_CHANNEL_STATE);
    pkt.setParameters(Constants::PASSWORD_PARAMS);

    return impl().m_ser.write(pkt.serialize());
}

const SerialPort* SerController::port() const noexcept
{
    return &impl().m_ser;
}

} // Mertech
