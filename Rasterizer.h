#pragma once

#include "Features.h"

#include "iostream"

struct AABB {
	glm::vec2 Min;
	glm::vec2 Max;
};

class Rasterizer
{
	
	std::vector<CurveData> Curves;

	float TexelSize = 0.0f;

	std::vector<Quad> Quads;

	Maps Map;

	float Falloff;

	int Width, Height;

public:
	Rasterizer() {};
	Rasterizer(const std::vector<CurveData>& curves) : Curves(curves) {};
	~Rasterizer() {};

	void SetCurves(const std::vector<CurveData>& curves) { Curves = curves; }
	void Initialize(float ResU, float ResV) { 
		Width = ResU;
		Height = ResV;
		TexelSize = std::min(1.0f / ResU, 1.0f / ResV); 
		Map.ElevationMap.assign(ResU * ResV, 0.0f);
		Map.GradientMap.assign(ResU * ResV, glm::vec2(0.0f, 0.0f));
		Map.NoiseMap.assign(ResU * ResV, glm::vec2(0.0f, 0.0f));
		Map.ConstraintMaskMap.assign(ResU * ResV, 0);
		Map.Gradients.assign(ResU * ResV, glm::vec3(0.0f, 0.0f, 0.0f));
	}; // *0.5f °¡´É

	void BuildPolyline();
	const LinearCoord CreateLinearCoord(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t);

	const std::vector<CurveData>& GetCurves() { return Curves; }

	void PrintPolylines() const;
	void PrintPolylineMasks() const;

	float Lerp(float a, float b, float t) { return (1.0f - t) * a + t * b; };
	float InterpolateCubic(float p0, float p1, float p2, float p3, float t);

	void InterpolateConstraints(CurveData& Curve);
	void InterpolateCurves();

	bool BuildQuad(const LinearCoord& p0, const LinearCoord& p1, Quad& OutQuad);
	void BuildQuads();
	void PrintQuads() const;

	void BuildConstraintMaps();

	AABB ComputeAABB(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3 v2, const glm::vec3 v3) const;

	bool Barycentric(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, float& OutU, float& OutV, float& OutW) const;
	void InterpolateQuad(const glm::vec2& p, const Quad& quad, int r, int c);

	const Maps& GetMaps() const { return Map; }

	glm::ivec4 GetBorderPixels2();

	float GradientAttenuation(glm::vec3& DstGradient, const glm::vec2& Pos, const glm::vec2& CurvePos, const glm::vec2& CurveNormal,
		const float SignedDistance, const float GradientRadius, float GradientMagnitude);

};

