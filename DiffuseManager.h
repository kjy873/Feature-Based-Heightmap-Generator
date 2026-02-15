#pragma once

#include "Features.h"

class DiffuseManager
{
	Maps Map;

	std::vector<float> ResidualMap;
	std::vector<float> CorrectionMap;

	std::vector<glm::vec4> PackedMapRGBA;
	std::vector<glm::vec4> PackedMapRGRC;

public:
	DiffuseManager() {};
	~DiffuseManager() {};

	void Initialize(int ResU, int ResV) {
		Map.ElevationMap.assign(ResU * ResV, 0.0f);
		Map.NoiseMap.assign(ResU * ResV, glm::vec2(0.0f, 0.0f));
		Map.ConstraintMaskMap.assign(ResU * ResV, 0);
		Map.Gradients.assign(ResU * ResV, glm::vec3(0.0f, 0.0f, 0.0f));
		ResidualMap.assign(ResU * ResV, 0.0f);
		CorrectionMap.assign(ResU * ResV, 0.0f);
		PackedMapRGBA.assign(ResU * ResV, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
		PackedMapRGRC.assign(ResU * ResV, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	};

	void SetGradientMap(const std::vector<glm::vec3>& GradientMap) { Map.Gradients = GradientMap; }
	const std::vector<glm::vec3>& GetGradientMap() const { return Map.Gradients; }
	void NormalizeGradients();

	void SetElevationMap(const std::vector<float>& ElevationMap) { Map.ElevationMap = ElevationMap; }
	const std::vector<float>& GetElevationMap() const { return Map.ElevationMap; }

	void SetNoiseMap(const std::vector<glm::vec2>& NoiseMap) { Map.NoiseMap = NoiseMap; }
	const std::vector<glm::vec2>& GetNoiseMap() const { return Map.NoiseMap; }

	void SetResidualMap(const std::vector<float>& ResidualMap) { this->ResidualMap = ResidualMap; }
	const std::vector<float>& GetResidualMap() const { return ResidualMap; }

	void SetCorrectionMap(const std::vector<float>& CorrectionMap) { this->CorrectionMap = CorrectionMap; }
	const std::vector<float>& GetCorrectionMap() const { return CorrectionMap; }

	const std::vector<glm::vec4>& GetPackedMapRGBA() const { return PackedMapRGBA; }
	const std::vector<glm::vec4>& GetPackedMapRGRC() const { return PackedMapRGRC; }

	// 이 함수는 3개의 Set Map 함수 호출 이후 호출되어야 함
	void PackMaps();
};
