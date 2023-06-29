#pragma once

#include <QByteArray>
#include <QVector>

#include "pimpl.h"

namespace Mertech
{

class Packet
{
public:
    enum MessageType
    {
        NONE = 0,
        STX = 0x02, //! normal message
        ENQ = 0x05, //! sent after not recieving NAK/ACK after timeout
        ACK = 0x06, //! message recieved successfully
        NAK = 0x15  //! failed to recieve message
    };

    Packet();
    Packet(Packet& other);
    virtual ~Packet();

    QByteArray serialize() noexcept;
    static Packet deserialize(const QByteArray& data) noexcept;
    static uint8_t getChecksum(uint8_t lenght, uint8_t command, const QVector<uint8_t>& params = {}) noexcept;
    bool isValid() const noexcept;

    const MessageType messageType() const noexcept;
    const uint8_t length() const noexcept;
    const uint8_t command() const noexcept;
    const uint8_t checksum() const noexcept;
    const QVector<uint8_t> parameters() const noexcept;
    const QByteArray data() const noexcept;

    void setMessageType(MessageType type)noexcept;
    void setLength(uint8_t length)noexcept;
    void setCommand(uint8_t command)noexcept;
    void setChecksum(uint8_t checksum)noexcept;
    void setParameters(const QVector<uint8_t>& params)noexcept;
    void setData(const QByteArray& data)noexcept;

private:
    DECLARE_PIMPL
};

} // Mertech
