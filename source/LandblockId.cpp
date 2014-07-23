#include "LandblockId.h"

LandblockId::LandblockId() : _x(0), _y(0)
{}

LandblockId::LandblockId(uint8_t lx, uint8_t ly) : _x(lx), _y(ly)
{}

uint8_t LandblockId::x() const
{
    return _x;
}

uint8_t LandblockId::y() const
{
    return _y;
}

uint32_t LandblockId::fileId() const
{
    return (uint32_t)_x << 24 | (uint32_t)_y << 16 | 0xFFFF;
}

int LandblockId::calcSquareDistance(LandblockId other) const
{
    auto dx = other.x() - x();
    auto dy = other.y() - y();
    return dx * dx + dy * dy;
}

bool LandblockId::operator==(LandblockId other) const
{
    return _x == other._x && _y == other._y;
}

bool LandblockId::operator!=(LandblockId other) const
{
    return _x != other._x || _y != other._y;
}
