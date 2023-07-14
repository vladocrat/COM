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

    bool checkReady()
    {
        if (!m_ser.readySend())
        {
            qDebug() << "devuce is not ready yet";
            return false;
        }

        return true;
    }
};

SerController::SerController(const QString& name, QSerialPort::BaudRate rate)
{
    createImpl();

    impl().m_ser.setPortName(name);
    impl().m_ser.setPullingRate(rate);

    QObject::connect(&impl().m_ser, &SerialPort::weightChannelInfoRecieved, [](const WeightChannel& channel)
    {
        qDebug() << "data recieved";
        qDebug() << hex << channel.weight;
        qDebug() << channel.tare;
    });
}

SerController::~SerController() noexcept
{

}

bool SerController::getWeight() noexcept
{
    if (!impl().checkReady())
    {
        return false;
    }

    Packet pkt;
    pkt.setMessageType(Packet::MessageType::STX);
    pkt.setCommand(Protocol::Command::WEIGHT_CHANNEL_STATE);
    pkt.setParameters(Constants::PASSWORD_PARAMS);

    return impl().m_ser.write(pkt.serialize());
}

bool SerController::setZero() noexcept
{
    if (!impl().checkReady())
    {
        return false;
    }

    Packet pkt;
    pkt.setMessageType(Packet::MessageType::STX);
    pkt.setCommand(Protocol::Command::SET_ZERO);
    pkt.setParameters(Constants::PASSWORD_PARAMS);

    return impl().m_ser.write(pkt.serialize());
}

bool SerController::setTare() noexcept
{
    if (!impl().checkReady())
    {
        return false;
    }

    Packet pkt;
    pkt.setMessageType(Packet::MessageType::STX);
    pkt.setCommand(Protocol::Command::SET_TARE);
    pkt.setParameters(Constants::PASSWORD_PARAMS);

    return impl().m_ser.write(pkt.serialize());
}

const SerialPort* SerController::port() const noexcept
{
    return &impl().m_ser;
}

} // Mertech
