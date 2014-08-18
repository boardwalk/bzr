#ifndef BZR_TRIANGLESTRIP_H
#define BZR_TRIANGLESTRIP_H

#include <vector>

class BlobReader;

struct TriangleStrip
{
	struct Index
	{
	    Index() : vertexIndex(0), texCoordIndex(0)
	    {}

	    int vertexIndex;
	    int texCoordIndex;
	};

	TriangleStrip() : texIndex(0)
    {}

    int texIndex;
    vector<Index> indices;

    void read(BlobReader& reader);
};

#endif