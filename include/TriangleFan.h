#ifndef BZR_TRIANGLEFAN_H
#define BZR_TRIANGLEFAN_H

#include <vector>

class BlobReader;

struct TriangleFan
{
	struct Index
	{
	    Index() : vertexIndex(0), texCoordIndex(0)
	    {}

	    int vertexIndex;
	    int texCoordIndex;
	};

	TriangleFan() : texIndex(0)
    {}

    int texIndex;
    vector<Index> indices;

    void read(BlobReader& reader);
};

#endif