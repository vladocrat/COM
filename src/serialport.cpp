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
static const uint32_t TIMEOUT = 5000; // in ms
} // Constants

struct SerialPort::impl_t
{
    QSerialPort m_ser;
    int32_t m_packageSize { -1 };
    bool m_readySend { true };
    QTimer m_timeoutTimer;
    bool m_firstMessage { true };
    QByteArray m_lastMessge;
};

SerialPort::SerialPort()
{
    createImpl();

    impl().m_ser.setDataBits(QSerialPort::Data8);
    impl().m_ser.setParity(QSerialPort::NoParity);
    impl().m_ser.setStopBits(QSerialPort::OneStop);
    impl().m_ser.setFlowControl(QSerialPort::NoFlowControl);

    QObject::connect(&impl().m_ser, &QSerialPort::errorOccurred, [](QSerialPort::SerialPortError error)
    {
        qDebug() << error;
    });

    QObject::connect(&impl().m_ser, &QSerialPort::bytesWritten, [this](uint64_t bytes)
    {
        qDebug() << "bytes written: " << bytes;

        if (!impl().m_ser.waitForReadyRead(Constants::TIMEOUT))
        {
            qDebug() << "failed to wait";
        }

    });

    QObject::connect(&impl().m_ser, &QSerialPort::readyRead, [this]()
    {
        qDebug() << "ReadyRead";
        qDebug() << impl().m_ser.bytesAvailable();
        //auto buffer = impl().m_ser.readAll();
        //qDebug() << buffer;
        handleData();
    });
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
    qDebug() << Q_FUNC_INFO;

    impl().m_lastMessge = data;

    if (impl().m_firstMessage)
    {
        qDebug() << "first msg";
        signal(Packet::MessageType::ENQ);
    }

    return writeInternal(data);
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
    qDebug() << Q_FUNC_INFO;

    if (!write(impl().m_lastMessge))
    {
        qDebug() << "Failed to write after message timedout";
        return;
    }
}

void SerialPort::signal(Packet::MessageType type)
{
    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);
    stream << static_cast<uint8_t>(type);
    writeInternal(arr);
}

bool SerialPort::writeInternal(const QByteArray& arr)
{
    qDebug() << Q_FUNC_INFO;

    auto ret =  impl().m_ser.write(arr) != -1;

    if (!impl().m_ser.flush())
    {
        qDebug() << "failed to flush";
    }

    return ret;
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

void SerialPort::handleData()
{
    QDataStream stream(&impl().m_ser);
    uint8_t cmd;

    //! readsize
    auto command = (char*)&cmd;
    int8_t dataLeft = sizeof(uint8_t);
    int32_t read { -1 };

    do {
       read = impl().m_ser.read(command, dataLeft);

       if (read > 0)
       {
           command += read;
           dataLeft -= read;
       }
    } while (dataLeft > 0);

    qDebug() << hex << cmd;

    //! readdata
    switch (cmd)
    {
    case Packet::ACK:
    {
        qDebug() << "recieved ACK";
        setReadySend(false);

        if (impl().m_ser.bytesAvailable() > 0)
        {
           handleData();
        }

        return;
    }
    case Packet::NAK:
    {
        qDebug() << "recieved NAK";
        setReadySend(true);

        if (impl().m_ser.bytesAvailable() > 0)
        {
            handleData();
        }

        break;
    }
    case Packet::STX:
    {
        qDebug() << "recieved STX";
        read = 0;
        uint8_t length;
        stream >> length;
        qDebug() << length;

        length++; //+ 1 is checksum
        auto data = new char[length];

        do {
          int32_t needed = length - read;
          char* target = data + read;
          read += impl().m_ser.read(target, needed);
        } while ((read - length) != 0);

        QByteArray msg(data, length + 1);
        qDebug() << msg;
        QDataStream stream(msg);

        uint8_t action;
        stream >> action;
        qDebug() << action;

        switch (action)
        {
        case Protocol::Command::WEIGHT_CHANNEL_STATE:
        {
            uint8_t error;
            stream >> error;
            qDebug() << "error " << error;

            uint16_t state;
            stream >> state;
            qDebug() << "state " << state;

            uint8_t weightBytes[4];

            for (int i = 0; i < 4; i++)
            {
                uint8_t elem;
                stream >> elem;
                weightBytes[i] = elem;
            }

            uint32_t value = 0;
            std::memcpy(&value, weightBytes, sizeof(uint32_t));
            float weight = (value / 1000.0f);

            uint16_t tare;
            stream >> tare;
            qDebug() << "tare " << tare;

            uint8_t reserved;
            stream >> reserved;
            qDebug() << "reserved " << reserved;

            uint8_t checksum;
            stream >> checksum;

            qDebug() << "expected: " << checksum;

            WeightChannel channel;
            channel.state = state;
            channel.tare = tare;
            channel.weight = weight;

            emit weightChannelInfoRecieved(channel);
            break;
        }
        case Protocol::Command::SET_TARE:
        {
            uint8_t errorCode { 0 };

            stream >> errorCode;

            auto result = errorCode == 0;

            emit tareSet(result);
            break;
        }
        case Protocol::Command::SET_ZERO:
        {
            uint8_t errorCode { 0 };

            stream >> errorCode;

            auto result = errorCode == 0;

            emit zeroSet(result);
            break;
        }
        default:
            Q_UNREACHABLE();
        }

        break;
    }
    }
} // SerialPort


} // Mertech
