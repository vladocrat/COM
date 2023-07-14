#pragma once

#include <QObject>
#include <QString>

#include "serialport.h"
#include "pimpl.h"

namespace Mertech
{

class SerController : public QObject
{
    Q_OBJECT
public:
    SerController(const QString& name = "", QSerialPort::BaudRate rate = QSerialPort::Baud115200);
    virtual ~SerController() noexcept;

    [[nodiscard]] bool getWeight() noexcept;
    [[nodiscard]] bool setZero() noexcept;
    [[nodiscard]] bool setTare() noexcept;

    const SerialPort* port() const noexcept;

private:
    DECLARE_PIMPL
};

} // Mertech
