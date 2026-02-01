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
	// +normal 방향 = left
	// constraint의 a 방향
	glm::vec3 normal = glm::normalize(glm::vec3(tangent.z, 0.0f, -tangent.x));

	return LinearCoord{ pos, normal, tangent, glm::clamp(t, 0.0f, 1.0f) };

}

void Rasterizer::PrintPolylines() const {
	int count = 0;
	for (const auto& curve : Curves) {
		count += curve.PolylineSamples.size();
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
	std::cout << "Total Polyline Samples: " << count << std::endl;
}

void Rasterizer::PrintPolylineMasks() const {
	for (const auto& curve : Curves) {
		for (const auto& sample : curve.PolylineSamples) {
			if (HasMask(sample.Constraint.Mask, ConstraintMask::Elevation) || HasMask(sample.Constraint.Mask, ConstraintMask::Gradient)) std::cout << "HasMask" << std::endl;
		}
	}
}

float Rasterizer::InterpolateCubic(float p0, float p1, float p2, float p3, float t) {

	float t2 = t * t;
	float t3 = t2 * t;

	return 0.5f * (
		(2.0f * p1) +
		(-p0 + p2) * t +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
		);

}

// constraint point -> sample point(poly line)
// 규칙
// 커브 양 끝 샘플 u== 0.0/1.0에서는 커브 양 끝 constraint point의 constraint를 그대로 사용(보간 대상X)
// 보간은 양쪽 constraint point가 모두 해당 constraint를 가질 때에만 수행
// 
// h - cubic interpolation
// 2개의 점만 찾은 경우 - endpoint clamping -> cubic interpolation
// 3개의 점만 찾은 경우 - 2개 + 2개(1개->endpoint clamping) -> cubic interpolation
// 4개의 점을 찾은 경우 - 4개의 점으로 cubic interpolation

void Rasterizer::InterpolateConstraints(CurveData& Curve) {

	for (auto& point : Curve.PolylineSamples) {

		point.Constraint.Mask = ConstraintMask::None;

		if ((point.u - 0.0f) < 1e-6f){
			point.Constraint = Curve.ConstraintPoints.front();
			point.Constraint.u = point.u;
			continue;
		}
		if ((point.u - 1.0f) < 1e-6f) {
			point.Constraint = Curve.ConstraintPoints.back();
			point.Constraint.u = point.u;
			continue;
		}

		int Segment = -1;
		for (int i = 0; i < Curve.ConstraintPoints.size() - 1; i++) {
			if (point.u >= Curve.ConstraintPoints[i].u && point.u <= Curve.ConstraintPoints[i + 1].u) {
				Segment = i;
				break;
			}
		}

		if (Segment == -1) continue;
		if (point.u - Curve.ConstraintPoints[Segment].u < 1e-6f) {
			point.Constraint = Curve.ConstraintPoints[Segment];
			point.Constraint.u = point.u;
			continue;
		}
		if (point.u - Curve.ConstraintPoints[Segment + 1].u > -1e-6f) {
			point.Constraint = Curve.ConstraintPoints[Segment + 1];
			point.Constraint.u = point.u;
			continue;
		}

		float t = (point.u - Curve.ConstraintPoints[Segment].u) / (Curve.ConstraintPoints[Segment + 1].u - Curve.ConstraintPoints[Segment].u);
		t = glm::clamp(t, 0.0f, 1.0f);

		if (HasMask(Curve.ConstraintPoints[Segment].Mask, ConstraintMask::Elevation) && HasMask(Curve.ConstraintPoints[Segment + 1].Mask, ConstraintMask::Elevation)) {
			float p0h, p1h, p2h, p3h;
			p1h = Curve.ConstraintPoints[Segment].h;
			p2h = Curve.ConstraintPoints[Segment + 1].h;
			p0h = (Segment > 0) ? Curve.ConstraintPoints[Segment - 1].h : Curve.ConstraintPoints[Segment].h;
			p3h = (Segment + 2 < Curve.ConstraintPoints.size()) ? Curve.ConstraintPoints[Segment + 2].h : Curve.ConstraintPoints[Segment + 1].h;

			point.Constraint.h = InterpolateCubic(p0h, p1h, p2h, p3h, t);
			point.Constraint.r = Lerp(Curve.ConstraintPoints[Segment].r, Curve.ConstraintPoints[Segment + 1].r, t);

			point.Constraint.Mask |= ConstraintMask::Elevation;
		}

		if (HasMask(Curve.ConstraintPoints[Segment].Mask, ConstraintMask::Gradient) && HasMask(Curve.ConstraintPoints[Segment + 1].Mask, ConstraintMask::Gradient)) {
			point.Constraint.a = Lerp(Curve.ConstraintPoints[Segment].a, Curve.ConstraintPoints[Segment + 1].a, t);
			point.Constraint.b = Lerp(Curve.ConstraintPoints[Segment].b, Curve.ConstraintPoints[Segment + 1].b, t);
			point.Constraint.theta = Lerp(Curve.ConstraintPoints[Segment].theta, Curve.ConstraintPoints[Segment + 1].theta, t);
			point.Constraint.phi = Lerp(Curve.ConstraintPoints[Segment].phi, Curve.ConstraintPoints[Segment + 1].phi, t);

			point.Constraint.Mask |= ConstraintMask::Gradient;
		}

		if (HasMask(Curve.ConstraintPoints[Segment].Mask, ConstraintMask::Noise) && HasMask(Curve.ConstraintPoints[Segment + 1].Mask, ConstraintMask::Noise)) {
			point.Constraint.Amplitude = Lerp(Curve.ConstraintPoints[Segment].Amplitude, Curve.ConstraintPoints[Segment + 1].Amplitude, t);
			point.Constraint.Roughness = Lerp(Curve.ConstraintPoints[Segment].Roughness, Curve.ConstraintPoints[Segment + 1].Roughness, t);

			point.Constraint.Mask |= ConstraintMask::Noise;
		}
		
		point.Constraint.u = point.u;
	}

}

void Rasterizer::InterpolateCurves() {
	for (auto& curve : Curves) {
		InterpolateConstraints(curve);
	}
}

bool Rasterizer::BuildQuad(const LinearCoord& p0, const LinearCoord& p1, Quad& OutQuad) {
	QuadVertex v0, v1, v2, v3;

	float LeftWidthP0 = 0.0f;
	float LeftWidthP1 = 0.0f;
	float RightWidthP0 = 0.0f;
	float RightWidthP1 = 0.0f;

	bool HasElevation = HasMask(p0.Constraint.Mask, ConstraintMask::Elevation) && HasMask(p1.Constraint.Mask, ConstraintMask::Elevation);
	bool HasGradient = HasMask(p0.Constraint.Mask, ConstraintMask::Gradient) && HasMask(p1.Constraint.Mask, ConstraintMask::Gradient);

	if (!HasElevation && !HasGradient) return false;

	if (HasElevation && HasGradient) {
		LeftWidthP0 = std::max(p0.Constraint.a, p0.Constraint.r);
		LeftWidthP1 = std::max(p1.Constraint.a, p1.Constraint.r);
		RightWidthP0 = std::max(p0.Constraint.b, p0.Constraint.r);
		RightWidthP1 = std::max(p1.Constraint.b, p1.Constraint.r);
	}
	else if (HasElevation) {
		LeftWidthP0 = RightWidthP0 = p0.Constraint.r;
		LeftWidthP1 = RightWidthP1 = p1.Constraint.r;
	}
	else {
		LeftWidthP0 = p0.Constraint.a;
		LeftWidthP1 = p1.Constraint.a;
		RightWidthP0 = p0.Constraint.b;
		RightWidthP1 = p1.Constraint.b;
	}

	v0.Position = p0.Pos + p0.Normal * LeftWidthP0;
	v0.Constraint = p0.Constraint;
	v0.CurvePos = p0.Pos;
	v0.CurveNormal = p0.Normal;
	v1.Position = p1.Pos + p1.Normal * LeftWidthP1;
	v1.Constraint = p1.Constraint;
	v1.CurvePos = p1.Pos;
	v1.CurveNormal = p1.Normal;
	
	v2.Position = p0.Pos - p0.Normal * RightWidthP0;
	v2.Constraint = p0.Constraint;
	v2.CurvePos = p0.Pos;
	v2.CurveNormal = p0.Normal;
	v3.Position = p1.Pos - p1.Normal * RightWidthP1;
	v3.Constraint = p1.Constraint;
	v3.CurvePos = p1.Pos;
	v3.CurveNormal = p1.Normal;


	OutQuad.V0 = v0;
	OutQuad.V1 = v1;
	OutQuad.V2 = v3;
	OutQuad.V3 = v2;

	return true;

}

void Rasterizer::BuildQuads() {
	Quad quad;

	for (const auto& curve : Curves) {
		for (int i = 0; i < curve.PolylineSamples.size() - 1; i++) {
			if (BuildQuad(curve.PolylineSamples[i], curve.PolylineSamples[i + 1], quad)) Quads.emplace_back(quad);
			//else std::cout << "false" << std::endl;
		}
	}

}

void Rasterizer::PrintQuads() const{

	std::cout << "Quad count: " << Quads.size() << std::endl;
	for (const auto& quad : Quads) {
		std::cout << "Quad Vertices :" << "[V0 : (" << quad.V0.Position.x << ", " << quad.V0.Position.y << ", " << quad.V0.Position.z << "), "
			<< "V1 : (" << quad.V1.Position.x << ", " << quad.V1.Position.y << ", " << quad.V1.Position.z << "), "
			<< "V2 : (" << quad.V2.Position.x << ", " << quad.V2.Position.y << ", " << quad.V2.Position.z << "), "
			<< "V3 : (" << quad.V3.Position.x << ", " << quad.V3.Position.y << ", " << quad.V3.Position.z << ")]" << std::endl;
	}

}