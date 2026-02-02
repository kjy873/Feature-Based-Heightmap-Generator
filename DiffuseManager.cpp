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