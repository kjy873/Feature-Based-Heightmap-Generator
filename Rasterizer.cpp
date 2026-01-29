#include "Rasterizer.h"

void Rasterizer::BuildPolyline() {

	float du = TexelSize * 0.25f;

	for (auto& curve : Curves) {
		const std::vector<glm::vec3>& cp = curve.ControlPoints;
		const int LoopCount = (int)(cp.size() - 1) / 3;

		bool FirstPoint = true;

		float accumulator = 0.0f;
		glm::vec3 PrePos = cp[0];
		for (int i = 0; i < LoopCount; i++) {
			// 한 세그먼트마다 4 개의 점, 3n ~ 3n + 3
			const glm::vec3 p0 = cp[3 * i];
			const glm::vec3 p1 = cp[3 * i + 1];
			const glm::vec3 p2 = cp[3 * i + 2];
			const glm::vec3 p3 = cp[3 * i + 3];

			for (float j = 0.0f; j < 1.0f; j += du) {
				if (FirstPoint) {
					const LinearCoord lc = CreateLinearCoord(p0, p1, p2, p3, j);
					curve.PolylineSamples.emplace_back(lc);
					PrePos = lc.Pos;
					FirstPoint = false;
					continue;
				}
				
				glm::vec3 pos = BezierCubic(p0, p1, p2, p3, j);
				
				accumulator += glm::length(pos - PrePos);
				if (accumulator >= TexelSize) {
					curve.PolylineSamples.emplace_back(CreateLinearCoord(p0, p1, p2, p3, j));
					accumulator -= TexelSize;
				}
				PrePos = pos;
			}

			curve.PolylineSamples.emplace_back(CreateLinearCoord(p0, p1, p2, p3, 1.0f));
		}
	}

}

const LinearCoord Rasterizer::CreateLinearCoord(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
	
	float u = 1.0f - t;
	glm::vec3 pos = u * u * u * p0 + 3.0f * u * u * t * p1 + 3.0f * u * t * t * p2 + t * t * t * p3;

	glm::vec3 PreviousTangent = glm::vec3(0.0f);
	glm::vec3 tangent = glm::normalize(3.0f * u * u * (p1 - p0) + 6.0f * u * t * (p2 - p1) + 3.0f * t * t * (p3 - p2));
	if (glm::length(tangent) < 1e-8f) tangent = PreviousTangent;
	else PreviousTangent = tangent;
	// left
	glm::vec3 normal = glm::normalize(glm::vec3(tangent.z, 0.0f, -tangent.x));

	return LinearCoord{ pos, normal, tangent, glm::clamp(t, 0.0f, 1.0f) };

}