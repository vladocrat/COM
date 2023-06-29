#include "packet.h"

#include <QDataStream>
#include <QDebug>

#include <bitset>

namespace Mertech
{

struct Packet::impl_t
{
    MessageType messageType { MessageType::NONE };
    uint8_t length { 0 };
    uint8_t command { 0 };
    uint8_t checksum { 0 };
    QVector<uint8_t> params;
    QByteArray data;
};

Packet::Packet()
{
    createImpl();
}

Packet::Packet(Packet& other)
{
    createImpl();

    impl().messageType = other.messageType();
    impl().length = other.length();
    impl().command = other.command();
    impl().params = other.parameters();
    impl().checksum = other.checksum();
}

Packet::~Packet()
{

}

QByteArray Packet::serialize() noexcept
{
    QDataStream stream(&impl().data, QIODevice::WriteOnly);

    impl().length = sizeof(uint8_t) + impl().params.length();

    stream << impl().messageType << impl().length << impl().command;

    for (const auto& param : qAsConst(impl().params))
    {
        stream << param;
    }

    stream << getChecksum(impl().length, impl().command, impl().params);

    return impl().data;
}

Packet Packet::deserialize(const QByteArray& data) noexcept
{
    QDataStream stream(data);

    Packet pkt;

    MessageType type;
    uint8_t length;
    uint8_t command;
    uint8_t checksum;

    stream >> type;
    stream >> length;
    stream >> command;

    pkt.setMessageType(type);
    pkt.setLength(length);
    pkt.setCommand(command);

    if (length <= 2)
    {
        stream >> checksum;
        pkt.setChecksum(checksum);

        return pkt;
    }

    QVector<uint8_t> params;
    uint8_t param;

    for (uint8_t i = 0; i < length; i++)
    {
        stream >> param;
        params.push_back(param);
    }

    pkt.setParameters(params);

    return pkt;
}

uint8_t Packet::getChecksum(uint8_t length, uint8_t command, const QVector<uint8_t>& params) noexcept
{
    uint8_t checksum;

    if (params.empty())
    {
        checksum = length ^ command;

        return checksum;
    }

    checksum = length ^ command;

    for (const auto& param : params)
    {
        checksum ^= param;
    }

    return checksum;
}

bool Packet::isValid() const noexcept
{
    return impl().checksum == getChecksum(impl().length, impl().command, impl().params);
}

const Packet::MessageType Packet::messageType() const noexcept
{
    return impl().messageType;
}

const uint8_t Packet::length() const noexcept
{
    return impl().length;
}

const uint8_t Packet::command() const noexcept
{
    return impl().command;
}

const uint8_t Packet::checksum() const noexcept
{
    return impl().checksum;
}

const QVector<uint8_t> Packet::parameters() const noexcept
{
    return impl().params;
}

const QByteArray Packet::data() const noexcept
{
    return impl().data;
}

void Packet::setMessageType(MessageType type) noexcept
{
    if (impl().messageType != type)
    {
        impl().messageType = type;
    }
}

void Packet::setLength(uint8_t length) noexcept
{
    if (impl().length != length)
    {
        impl().length = length;
    }
}

void Packet::setCommand(uint8_t command) noexcept
{
    if (impl().command != command)
    {
        impl().command = command;
    }
}

void Packet::setChecksum(uint8_t checksum) noexcept
{
    if (impl().checksum != checksum)
    {
        impl().checksum = checksum;
    }
}

void Packet::setParameters(const QVector<uint8_t>& params) noexcept
{
    if (impl().params != params)
    {
        impl().params = params;
    }
}

void Packet::setData(const QByteArray& data) noexcept
{
    if (impl().data != data)
    {
        impl().data = data;
    }
}

} // Mertech
