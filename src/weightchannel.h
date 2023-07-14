#pragma once

#include <type_traits>

namespace Mertech
{

struct WeightChannel
{
    float weight { 0 };
    uint16_t tare { 0 };
    uint16_t state { 0 };
};

} // Mertech
