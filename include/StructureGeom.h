#ifndef BZR_STRUCTUREGEOM_H
#define BZR_STRUCTUREGEOM_H

#include "Resource.h"

class StructureGeom : public ResourceImpl<ResourceType::StructureGeom>
{
public:
	StructureGeom(uint32_t id, const void* data, size_t size);	
};

#endif