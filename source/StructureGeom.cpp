#include "StructureGeom.h"

StructureGeom::StructureGeom(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
	(void)data;
	(void)size;
}
