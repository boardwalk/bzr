#ifndef BZR_STRUCTURERENDERDATA_H
#define BZR_STRUCTURERENDERDATA_H

#include "Destructable.h"
#include "Noncopyable.h"

class Structure;

class StructureRenderData : public Destructable, Noncopyable
{
public:
	StructureRenderData(const Structure& structure);
	~StructureRenderData();

	void bind();
};

#endif
