#pragma once

#include <glm.hpp>
#include <functional>
#include <string>

static const glm::vec2 GRADS[8] = {
	{1,0},{-1,0},{0,1},{0,-1},
	{0.7071067f,0.7071067f},{-0.7071067f,0.7071067f},
	{0.7071067f,-0.7071067f},{-0.7071067f,-0.7071067f}
};  // for fast gradient calculation

static float PI = 3.14159265358979323846f;

// linear interpolation
inline float Lerp(float a, float b, float t) {
	return (1 - t) * a + t * b;
}
// hold interpolation
inline float hold(float p, float t) {
	return (1 - t) * p + t * p;
}

inline glm::vec2 CalGradient(const glm::vec2& p, int seed) {
	uint32_t h = (uint32_t)p.x * 374761393u
		^ (uint32_t)p.y * 668265263u
		^ (uint32_t)seed * 982451653u;

	h ^= h >> 13;
	h *= 1274126177u;
	h ^= h >> 16;

	float angle = (h & 0xFFFFu) * (2.0f * PI / 65536.0f);

	return glm::vec2(cos(angle), sin(angle));
}

inline glm::vec2 CalGradientFast(const glm::vec2& p, int seed) {

	uint32_t h = (uint32_t)p.x * 374761393u
		^ (uint32_t)p.y * 668265263u
		^ (uint32_t)seed * 982451653u;

	h ^= h >> 13;
	h *= 1274126177u;
	h ^= h >> 16;

	return GRADS[h & 7u];

}

inline float perlinSmooth(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}
inline float Perlin(const glm::vec2& input, int seed) {

	int cellX = floor(input.x); // cell index
	int cellY = floor(input.y);

	float localX = input.x - cellX; // local position in the cell
	float localY = input.y - cellY;

	float d1 = glm::dot(glm::vec2(localX, localY), CalGradientFast(glm::vec2(cellX, cellY), seed));
	float d2 = glm::dot(glm::vec2(localX - 1, localY), CalGradientFast(glm::vec2(cellX + 1, cellY), seed));
	float d3 = glm::dot(glm::vec2(localX, localY - 1), CalGradientFast(glm::vec2(cellX, cellY + 1), seed));
	float d4 = glm::dot(glm::vec2(localX - 1, localY - 1), CalGradientFast(glm::vec2(cellX + 1, cellY + 1), seed));

	float interpolatedX1 = Lerp(d1, d2, perlinSmooth(localX));
	float interpolatedX2 = Lerp(d3, d4, perlinSmooth(localX));

	float value = Lerp(interpolatedX1, interpolatedX2, perlinSmooth(localY));

	return(value);
}

inline float SimplexAttenuation(const glm::vec2& point, const glm::vec2& gradient) {

	float t = 0.5 - glm::dot(point, point);

	if (t < 0) return  0.0f;
	else return t * t * t * t * dot(point, gradient);

}

inline float Simplex(const glm::vec2& input, int seed) {
	float F2 = (sqrt(3) - 1) / 2.0f;
	float G2 = (3 - sqrt(3)) / 6.0f;

	float s = (input.x + input.y) * F2;

	float i = floor(input.x + s);
	float j = floor(input.y + s);

	float t = (i + j) * G2;

	glm::vec2 cellOrigin = glm::vec2(i - t, j - t);
	glm::vec2 local = glm::vec2(input.x - cellOrigin.x, input.y - cellOrigin.y);

	glm::vec2 p1 = glm::vec2(local.x, local.y);
	glm::vec2 p2 = glm::vec2(0.0f);
	glm::vec2 p3 = glm::vec2(0.0f);

	glm::vec2 g1 = CalGradient(glm::vec2(i, j), seed);
	glm::vec2 g2 = glm::vec2(0.0f);
	glm::vec2 g3 = glm::vec2(0.0f);
	if (local.x > local.y) {
		p2 = glm::vec2(local.x - 1, local.y);
		p3 = glm::vec2(local.x - 1, local.y - 1);

		g2 = CalGradient(glm::vec2(i + 1, j), seed);
		g3 = CalGradient(glm::vec2(i + 1, j + 1), seed);
	}
	else {
		p2 = glm::vec2(local.x, local.y - 1);
		p3 = glm::vec2(local.x - 1, local.y - 1);

		g2 = CalGradient(glm::vec2(i, j + 1), seed);
		g3 = CalGradient(glm::vec2(i + 1, j + 1), seed);
	}

	float a1 = SimplexAttenuation(p1, g1);
	float a2 = SimplexAttenuation(p2, g2);
	float a3 = SimplexAttenuation(p3, g3);

	return (a1 + a2 + a3) * 70.0f;

}

inline float NoiseCombiner1(const glm::vec2& p, const float& width, const float& height, const int& seed,
	float frequency, int octaves, float persistence, float lacunarity, const std::string& noiseType) {
	float total = 0.0f;
	float amplitude = 1.0f;

	glm::vec2 point = glm::vec2((p.x + 0.5f) / width, (p.y + 0.5f) / height);

	if (noiseType == "Perlin") {
		for (int i = 0; i < octaves; i++) {
			total += Perlin(point * frequency, seed) * amplitude;
			frequency *= lacunarity;
			amplitude *= persistence;
		}
	}

	else if (noiseType == "Simplex") {
		for (int i = 0; i < octaves; i++) {
			total += Simplex(point * frequency, seed) * amplitude;
			frequency *= lacunarity;
			amplitude *= persistence;
		}
	}

	return total;

}

inline std::function<float(const glm::vec2&)> NoiseSelector(const float& width, const float& height, const int& seed,
	float frequency, int octaves, float persistence, float lacunarity, const std::string& noiseType)
{

	return [width, height, seed, frequency, octaves, persistence, lacunarity, noiseType](const glm::vec2& p) {
		return NoiseCombiner1(p, width, height, seed, frequency, octaves, persistence, lacunarity, noiseType);
		};

}