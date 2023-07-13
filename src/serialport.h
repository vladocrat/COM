#pragma once

#include <QByteArray>
#include <QIODevice>
#include <QString>
#include <QObject>

#include <QtSerialPort/QSerialPort>

#include "pimpl.h"
#include "packet.h"
#include "weightchannel.h"

namespace Mertech
{

class SerialPort : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool readySend READ readySend NOTIFY readySendChanged)
public:
    SerialPort();
    virtual ~SerialPort();

    const bool open(QIODevice::OpenMode mode) noexcept;
    const bool write(const QByteArray& data) noexcept;

    void setPortName(const QString& name);
    void setPullingRate(QSerialPort::BaudRate rate);

    const QString name() const noexcept;
    const uint32_t pullRate() const noexcept;
    const QSerialPort::SerialPortError error() const noexcept;
    const bool readySend() const noexcept;

signals:
    void readySendChanged();
    void weightChannelInfoRecieved(Mertech::WeightChannel&);

private slots:
    void messageTimedout();

private:
    void signal(Packet::MessageType);
    bool writeInternal(const QByteArray&);
    void setReadySend(bool);
    void handleData();

    DECLARE_PIMPL
};

} // Mertech
