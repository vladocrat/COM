#include "test_packet.h"

#include "packet.h"
#include "protocol.h"

#include <QTest>
#include <QDataStream>

Test_Packet::Test_Packet()
{

}

Test_Packet::~Test_Packet()
{

}

void Test_Packet::checksum()
{
    uint8_t length = 3;
    uint8_t command = 7;

    QCOMPARE(Mertech::Packet::getChecksum(length, command), static_cast<uint8_t>(length ^ command));
}

void Test_Packet::checksumWithParams()
{
    uint8_t length = 4;
    uint8_t command = 2;
    QVector<uint8_t> params = {1, 3, 5};

    uint8_t expectedChecksum = length ^ command ^ params[0] ^ params[1] ^ params[2];
    QCOMPARE(Mertech::Packet::getChecksum(length, command, params), expectedChecksum);
}

void Test_Packet::serialize()
{
    Mertech::Packet packet;
    QByteArray expectedData;
    QDataStream expectedStream(&expectedData, QIODevice::WriteOnly);
    auto checksum = packet.getChecksum(packet.length(), packet.command(), packet.parameters());
    expectedStream << packet.messageType() << packet.length() << packet.command() << checksum;

    QVERIFY(packet.serialize() != expectedData);
}

void Test_Packet::serializeWithParams()
{
    Mertech::Packet packet;
    packet.setMessageType(Mertech::Packet::MessageType::STX);
    packet.setCommand(Mertech::Protocol::WEIGHT_CHANNEL_STATE);
    packet.setParameters({2, 3, 4});

    QByteArray expectedData;
    QDataStream expectedStream(&expectedData, QIODevice::WriteOnly);
    auto length =  static_cast<uint8_t>(sizeof(uint8_t) + packet.parameters().length());
    expectedStream << packet.messageType() << length << packet.command();

    for (const auto& param : packet.parameters())
    {
        expectedStream << param;
    }

    expectedStream << packet.getChecksum(length, packet.command(), packet.parameters());

    QCOMPARE(packet.serialize(), expectedData);
}

void Test_Packet::deserializeEmptyData()
{
    QByteArray data;
    Mertech::Packet expectedPacket;
    auto deserialized = Mertech::Packet::deserialize(data);
    QCOMPARE(deserialized.data(), expectedPacket.data());
}

void Test_Packet::deserializeEmptyPacket()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    Mertech::Packet::MessageType messageType = Mertech::Packet::MessageType::NONE;
    uint8_t length = 0;
    uint16_t command = 0;
    uint32_t checksum = 0;

    stream << messageType << length << command << checksum;

    Mertech::Packet expectedPacket;
    QCOMPARE(Mertech::Packet::deserialize(data).data(), expectedPacket.data());
}

void Test_Packet::deserialize()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    Mertech::Packet::MessageType messageType = Mertech::Packet::MessageType::STX;
    uint8_t length = 4;
    uint16_t command = 1;
    uint32_t checksum = 0;

    QVector<uint8_t> params = {2, 3, 4};

    stream << messageType << length << command;

    for (const auto& param : params)
    {
        stream << param;
    }

    stream << checksum;

    Mertech::Packet expectedPacket;
    expectedPacket.setMessageType(messageType);
    expectedPacket.setLength(length);
    expectedPacket.setCommand(command);
    expectedPacket.setParameters(params);
    expectedPacket.setChecksum(checksum);

    QCOMPARE(Mertech::Packet::deserialize(data).data(), expectedPacket.data());
}
