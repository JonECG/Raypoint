#ifndef NEUMONT_SHAPE_DATA_H
#define NEUMONT_SHAPE_DATA_H
#include "Vertex.h"

namespace Neumont
{
	struct ShapeData
	{
		Vertex* verts;
		uint numVerts;
		ushort* indices;
		uint numIndices;
		ShapeData() : verts(0), numVerts(0), indices(0), numIndices(0) {}
		uint vertexBufferSize() const { return numVerts * sizeof(Vertex); }
		uint indexBufferSize() const { return numIndices * sizeof(ushort); }
		void cleanUp()
		{
			delete[] verts;
			verts = 0;
			delete[] indices;
			indices = 0;
			numVerts = numIndices = 0;
		}
	};
}

#endif