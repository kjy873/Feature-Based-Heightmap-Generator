#include "TerrainObject.h"


TerrainObject::TerrainObject(int ResU, int ResV, int ChunkSizeX, int ChunkSizeY) {

	Rows = ResU;
	Cols = ResV;

	ChunkSizeX = ChunkSizeX;
	ChunkSizeY = ChunkSizeY;

	Chunks.clear();
	Chunks.reserve(((Rows + ChunkSizeX - 1) / ChunkSizeX) * ((Cols + ChunkSizeY - 1) / ChunkSizeY));



}