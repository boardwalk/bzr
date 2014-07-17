#include "LandblockId.h"

LandblockId::LandblockId(uint8_t x, uint8_t y) : _x(x), _y(y)
{}

int LandblockId::x() const
{
	return _x;
}

int LandblockId::y() const
{
	return _y;
}

uint32_t LandblockId::fileId() const
{
	return (uint32_t)_x << 24 | (uint32_t)_y << 16 | 0xFFFF;
}

int LandblockId::squareDistance(LandblockId other) const
{
	auto dx = other.x() - x();
	auto dy = other.y() - y();
	return dx * dx + dy * dy;
}

bool LandblockId::operator==(LandblockId other) const
{
	return x() == other.x() && y() == other.y();
}

bool LandblockId::operator!=(LandblockId other) const
{
	return x() != other.x() || y() != other.y();
}