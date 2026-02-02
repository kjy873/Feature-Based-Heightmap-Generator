#pragma once

#include "Features.h"

class DiffuseManager
{
	Maps Map;

public:
	DiffuseManager() {};
	~DiffuseManager() {};

	void Initialize(int ResU, int ResV) {
		Map.ElevationMap.assign(ResU * ResV, 0.0f);
		Map.GradientMap.assign(ResU * ResV, glm::vec2(0.0f, 0.0f));
		Map.NoiseMap.assign(ResU * ResV, glm::vec2(0.0f, 0.0f));
		Map.ConstraintMaskMap.assign(ResU * ResV, 0);
		Map.Gradients.assign(ResU * ResV, glm::vec3(0.0f, 0.0f, 0.0f));
	};

	void SetGradientMap(const std::vector<glm::vec3>& GradientMap) { Map.Gradients = GradientMap; }
	const std::vector<glm::vec3>& GetGradientMap() const { return Map.Gradients; }
	void NormalizeGradients();

	void SetElevationMap(const std::vector<float>& ElevationMap) { Map.ElevationMap = ElevationMap; }
	const std::vector<float>& GetElevationMap() const { return Map.ElevationMap; }
};

