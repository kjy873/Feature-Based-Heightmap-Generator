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

public:
	Rasterizer() {};
	Rasterizer(const std::vector<CurveData>& curves) : Curves(curves) {};
	~Rasterizer() {};

	void SetCurves(const std::vector<CurveData>& curves) { Curves = curves; }
	void Initialize(int ResU, int ResV);
	void UploadCurveData();

	const std::vector<CurveData>& GetCurves() { return Curves; }

};

