#pragma once

#include <type_traits>

namespace Mertech
{

struct WeightChannel
{
    uint32_t weight { 0 };
    uint16_t tare { 0 };
    uint16_t state { 0 };
};

} // Mertech
