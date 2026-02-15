#include "DiffuseManager.h"


void DiffuseManager::NormalizeGradients() {
	for (auto& g : Map.Gradients) {
		glm::vec2 Direction = glm::vec2(g.x, g.y);
		float Length = glm::length(Direction);

		if (Length > 1e-6f) Direction /= Length;
		else Direction = glm::vec2(0.0f, 0.0f);

		g = glm::vec3(Direction.x, Direction.y, g.z);
	}

}

void DiffuseManager::PackMaps() {
	for (int i = 0; i < PackedMapRGBA.size(); i++) {
		PackedMapRGBA[i] = glm::vec4(Map.ElevationMap[i], Map.Gradients[i].x, Map.Gradients[i].y, Map.Gradients[i].z);
		PackedMapRGRC[i] = glm::vec4(Map.NoiseMap[i].x, Map.NoiseMap[i].y, ResidualMap[i], CorrectionMap[i]);

	}
}