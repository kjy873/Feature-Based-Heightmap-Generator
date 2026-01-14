#pragma once

#include "Noise.h"
#include "vector"

class NoiseGenerator
{
	
	std::vector<float> Map;

	int Rows;
	int Cols;

	
public:

	NoiseGenerator(int Rows, int Cols) : Rows(Rows), Cols(Cols), Map(Rows * Cols) {};

	const float GetHeight(int Row, int Col) { return Map[Row * Cols + Col]; };

	void SetRes(int ResU, int ResV) { Rows = ResU; Cols = ResV; Map.clear(); Map.resize(Rows * Cols); }

	void GeneratePerlinNoise(int Seed, float Frequency, int Octaves, float Persistence, float Lacunarity);
	void GenerateSimplexNoise(int Seed, float Frequency, int Octaves, float Persistence, float Lacunarity);

	const std::vector<float> GetHeightMap() { return Map; }

};

