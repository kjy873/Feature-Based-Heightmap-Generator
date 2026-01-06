#pragma once

#include "Mesh.h"

#include <glm.hpp>
#include <vector>

struct Chunk {
	TerrainMesh Mesh;
	int StartRow;
	int StartCol;
	int Rows;
	int Cols;
};

class TerrainObject
{
private:

	std::vector<Chunk> Chunks;
	int ChunkSizeX;
	int ChunkSizeY;

	int Rows;
	int Cols;

public:
	TerrainObject(int ResU, int ResV, int ChunkSizeX, int ChunkSizeY);


};

