#include "serialport.h"

#include "packet.h"
#include "protocol.h"

#include <QDebug>
#include <QDataStream>
#include <QTimer>

namespace Mertech
{

namespace Constants
{
static const uint32_t TIMEOUT = 1000; // in ms
} // Constants

struct SerialPort::impl_t
{
    QSerialPort m_ser;
    int32_t m_packageSize { -1 };
    bool m_readySend { false };
    QTimer m_timeoutTimer;
    bool m_firstMessage { true };
};

SerialPort::SerialPort()
{
    createImpl();

    QObject::connect(&impl().m_ser, &QSerialPort::errorOccurred, [](QSerialPort::SerialPortError error)
    {
        qDebug() << error;
    });

    QObject::connect(&impl().m_ser, &QSerialPort::readyRead, [this]()
    {
        qDebug() << "ReadyRead";

        QDataStream stream(&impl().m_ser);

        if ((impl().m_ser.bytesAvailable() >= sizeof(uint8_t)) && impl().m_packageSize == -1)
        {
            stream >> impl().m_packageSize;
        }
        else
        {
            return;
        }

        if (impl().m_ser.bytesAvailable() < impl().m_packageSize) {
            return;
        }

        QByteArray data;
        stream >> data;
        impl().m_packageSize = -1;
        handleData(data);
    });

    if (impl().m_firstMessage)
    {
        QByteArray arr;
        QDataStream stream(&arr, QIODevice::WriteOnly);
        stream << Packet::MessageType::ENQ;

        QObject::connect(&impl().m_timeoutTimer, &QTimer::timeout, this, &SerialPort::messageTimedout);
        impl().m_timeoutTimer.setSingleShot(true);
        impl().m_timeoutTimer.setInterval(Constants::TIMEOUT);
        impl().m_timeoutTimer.start();

        if (impl().m_ser.write(arr) != -1)
        {
            qDebug() << "failed to create connection to the port";
        }
    }
}

SerialPort::~SerialPort()
{
    if (impl().m_ser.isOpen())
    {
        impl().m_ser.close();
    }

    if (impl().m_timeoutTimer.isActive())
    {
        impl().m_timeoutTimer.stop();
    }
}

const bool SerialPort::open(QIODevice::OpenMode mode) noexcept
{
    return impl().m_ser.open(mode);
}

const bool SerialPort::write(const QByteArray& data) noexcept
{   
    if (impl().m_firstMessage)
    {
        qDebug() << "something went wrong";
        return false;
    }

    auto ret = impl().m_ser.write(data) != -1;

    if (ret) {
        QObject::connect(&impl().m_timeoutTimer, &QTimer::timeout, this, &SerialPort::messageTimedout);
        impl().m_timeoutTimer.setSingleShot(true);
        impl().m_timeoutTimer.setInterval(Constants::TIMEOUT);
        impl().m_timeoutTimer.start();
    }

    return ret;
}

void SerialPort::setPortName(const QString& name)
{
    if (name != impl().m_ser.portName())
    {
        impl().m_ser.setPortName(name);
    }
}

void SerialPort::setPullingRate(QSerialPort::BaudRate rate)
{
    if (rate != impl().m_ser.baudRate())
    {
        impl().m_ser.setBaudRate(rate);
    }
}

const QString SerialPort::name() const noexcept
{
    return impl().m_ser.portName();
}

const uint32_t SerialPort::pullRate() const noexcept
{
    return impl().m_ser.baudRate();
}

const QSerialPort::SerialPortError SerialPort::error() const noexcept
{
    return impl().m_ser.error();
}

const bool SerialPort::readySend() const noexcept
{
    return impl().m_readySend;
}

void SerialPort::messageTimedout()
{
    //!TODO
    Q_UNIMPLEMENTED();
}

void SerialPort::setReadySend(bool val)
{
    if (impl().m_readySend == val)
    {
        return;
    }

    impl().m_readySend = val;
    emit readySendChanged();
}

void SerialPort::handleData(const QByteArray& msg)
{
    if (impl().m_timeoutTimer.isActive())
    {
        impl().m_timeoutTimer.stop();
    }

    //auto pkt = Packet::deserialize(msg);
    QDataStream stream(msg);
    uint8_t messageType;
    uint8_t length { 0 };
    uint8_t command { 0 };
    uint8_t checksum { 0 };
    QVector<uint8_t> params;

    stream >> messageType;

    switch (messageType)
    {
    case Packet::MessageType::ACK:
    {
        if (impl().m_firstMessage)
        {
            qDebug() << "connection has successfully been established";
            setReadySend(true);
            return;
        }

        qDebug() << "message was recieved";
        setReadySend(false);
        impl().m_timeoutTimer.start();
        break;
    }
    case Packet::MessageType::NAK:
    {
        qDebug() << "error while recieving message";
        setReadySend(true);
        break;
    }
    case Packet::MessageType::STX:
    {
        qDebug() << "regular meesage";

        setReadySend(true);

        stream >> length;
        stream >> command;

        switch (command)
        {
        case Protocol::Command::WEIGHT_CHANNEL_STATE:
        {
            uint8_t errorCode { 0 };
            uint16_t state { 0 };
            uint32_t weight { 0 };
            uint16_t tare { 0 };
            uint8_t reserved { 0 };

            stream >> errorCode;
            stream >> state;
            stream >> weight;
            stream >> tare;
            stream >> reserved;

            WeightChannel channel;
            channel.weight = weight;
            channel.state = state;
            channel.tare = tare;

            emit weightChannelInfoRecieved(channel);
            break;
        }
        case Protocol::Command::SET_TARE:
        {
            uint8_t errorCode { 0 };

            stream >> errorCode;

            auto result = errorCode != 0;

            emit tareSet(result);
            break;
        }
        case Protocol::Command::SET_ZERO:
        {
            uint8_t errorCode { 0 };

            stream >> errorCode;

            auto result = errorCode != 0;

            emit zeroSet(result);
            break;
        }
        default:
            Q_UNREACHABLE();
        }

        break;
    }
    default:
        Q_UNREACHABLE();
    };
}

} // Mertech
