#pragma once

#include "FeatureCurve.h"

struct CurveData {
	std::vector<glm::vec3> ControlPoints;
	std::vector<Constraints> ConstraintPoints;

	CurveData(const std::vector<glm::vec3>& controlPoints, const std::vector<Constraints>& constraintPoints)
		: ControlPoints(controlPoints), ConstraintPoints(constraintPoints) {};
};

class Rasterizer
{
	std::vector<CurveData> Curves;

public:
	Rasterizer();
	~Rasterizer();

	void Initialize(int width, int height);
	void UploadCurveData();

};

