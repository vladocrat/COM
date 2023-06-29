#pragma once

#include <QObject>

class Test_Packet : public QObject
{
    Q_OBJECT
public:
    Test_Packet();
    virtual ~Test_Packet();

private slots:
    void checksum();
    void checksumWithParams();
    void serialize();
    void serializeWithParams();
    void deserializeEmptyData();
    void deserializeEmptyPacket();
    void deserialize();
};


