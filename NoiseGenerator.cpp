#include "NoiseGenerator.h"

#include <chrono>
#include <iostream>

void NoiseGenerator::GeneratePerlinNoise(int Seed, float Frequency, int Octaves, float Persistence, float Lacunarity) {
	auto start = std::chrono::high_resolution_clock::now();
	Map.clear();
	Map.resize(Rows * Cols);

	const auto g = NoiseSelector(Rows, Cols, Seed, Frequency, Octaves, Persistence, Lacunarity, "Perlin");

	for (int i = 0; i < Rows; i++) {
		for (int j = 0; j < Cols; j++) {
			Map[i * Cols + j] = g(glm::vec2(j, i)) * .1f;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;

	std::cout << "노이즈 생성 시간: " << duration.count() << " ms\n";

}

void NoiseGenerator::GenerateSimplexNoise(int Seed, float Frequency, int Octaves, float Persistence, float Lacunarity) {

	Map.clear();
	Map.resize(Rows * Cols);

	const auto g = NoiseSelector(Rows, Cols, Seed, Frequency, Octaves, Persistence, Lacunarity, "Simplex");

	for (int i = 0; i < Rows; i++) {
		for (int j = 0; j < Cols; j++) {
			Map[i * Cols + j] = g(glm::vec2(j, i)) * .1f;
		}
	}

}