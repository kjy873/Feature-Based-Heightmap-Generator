#pragma once

#include "Features.h"

struct LinearCoord {
	glm::vec3 Pos;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	float u;
};

class Rasterizer
{
	
	std::vector<CurveData> Curves;

	std::vector<LinearCoord> PolylineSamples;

	float TexelSizeU = 0.0f;
	float TexelSizeV = 0.0f;

public:
	Rasterizer() {};
	Rasterizer(const std::vector<CurveData>& curves) : Curves(curves) {};
	~Rasterizer() {};

	void SetCurves(const std::vector<CurveData>& curves) { Curves = curves; }
	void Initialize(float ResU, float ResV) { TexelSizeU = 1.0f / ResU; TexelSizeV = 1.0f / ResV; };

	const std::vector<CurveData>& GetCurves() { return Curves; }

};

