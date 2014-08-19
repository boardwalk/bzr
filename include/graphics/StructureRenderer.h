#ifndef BZR_STRUCTURERENDERER_H
#define BZR_STRUCTURERENDERER_H

#include "Noncopyable.h"

struct Mat4;

class StructureRenderer : Noncopyable
{
public:
	StructureRenderer();

	void render(const Mat4& projectionMat, const Mat4& viewMat);
};

#endif