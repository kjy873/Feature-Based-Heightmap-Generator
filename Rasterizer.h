#pragma once

#include "Features.h"



class Rasterizer
{
	
	std::vector<CurveData> Curves;

	float TexelSize = 0.0f;

	float Width;
	float Falloff;

public:
	Rasterizer() {};
	Rasterizer(const std::vector<CurveData>& curves) : Curves(curves) {};
	~Rasterizer() {};

	void SetCurves(const std::vector<CurveData>& curves) { Curves = curves; }
	void Initialize(float ResU, float ResV) { TexelSize = std::min(1.0f / ResU, 1.0f / ResV); }; // *0.5f °¡´É

	void BuildPolyline();
	const LinearCoord CreateLinearCoord(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t);

	const std::vector<CurveData>& GetCurves() { return Curves; }

	void PrintPolylines() const {
		for (const auto& curve : Curves) {
			printf("Polyline Samples:\n");
			for (const auto& sample : curve.PolylineSamples) {
				printf("Pos: (%.3f, %.3f, %.3f), Normal: (%.3f, %.3f, %.3f), Tangent: (%.3f, %.3f, %.3f), u: %.3f\n",
					sample.Pos.x, sample.Pos.y, sample.Pos.z,
					sample.Normal.x, sample.Normal.y, sample.Normal.z,
					sample.Tangent.x, sample.Tangent.y, sample.Tangent.z,
					sample.u);
			}
			printf("\n");
		}
	}

};

