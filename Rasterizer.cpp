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

		if (fabs(point.u - 0.0f) < 1e-6f){
			point.Constraint = Curve.ConstraintPoints.front();
			point.Constraint.u = point.u;
			continue;
		}
		if (fabs(point.u - 1.0f) < 1e-6f) {
			point.Constraint = Curve.ConstraintPoints.back();
			point.Constraint.u = point.u;
			continue;
		}

		int Segment = -1;
		for (int i = 0; i < Curve.ConstraintPoints.size() - 1; i++) {
			if (point.u >= Curve.ConstraintPoints[i].u && point.u <= Curve.ConstraintPoints[i + 1].u) {
				
				//std::cout << "segment u : " << Curve.ConstraintPoints[i].u << ", " << Curve.ConstraintPoints[i + 1].u << std::endl;
				Segment = i;
				break;
			}
		}

		if (Segment == -1) continue;
		if (fabs(point.u - Curve.ConstraintPoints[Segment].u) < 1e-6f) {
			point.Constraint = Curve.ConstraintPoints[Segment];
			point.Constraint.u = point.u;
			
			continue;
		}
		if (fabs(point.u - Curve.ConstraintPoints[Segment + 1].u) < 1e-6f) {
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

		std::cout << "Interpolated Constraint u: " << point.Constraint.u << std::endl;
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
	bool HasNoise = HasMask(p0.Constraint.Mask, ConstraintMask::Noise) && HasMask(p1.Constraint.Mask, ConstraintMask::Noise);

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

	OutQuad.HasElevation = HasElevation;
	OutQuad.HasGradient = HasGradient;
	OutQuad.HasNoise = HasNoise;

	return true;

}

void Rasterizer::BuildQuads() {
	Quad quad;

	for (const auto& curve : Curves) {
		for (int i = 0; i < curve.PolylineSamples.size() - 1; i++) {
			if (BuildQuad(curve.PolylineSamples[i], curve.PolylineSamples[i + 1], quad)) {
				quad.CurveID = curve.CurveID;
				Quads.emplace_back(quad);
			}
			//else std::cout << "false" << std::endl;
		}
	}

}

void Rasterizer::PrintQuads() const{

	//std::cout << "Quad count: " << Quads.size() << std::endl;
	//for (const auto& quad : Quads) {
	//	std::cout << "Quad Vertices :" << "[V0 : (" << quad.V0.Position.x << ", " << quad.V0.Position.y << ", " << quad.V0.Position.z << "), "
	//		<< "V1 : (" << quad.V1.Position.x << ", " << quad.V1.Position.y << ", " << quad.V1.Position.z << "), "
	//		<< "V2 : (" << quad.V2.Position.x << ", " << quad.V2.Position.y << ", " << quad.V2.Position.z << "), "
	//		<< "V3 : (" << quad.V3.Position.x << ", " << quad.V3.Position.y << ", " << quad.V3.Position.z << ")]" << std::endl;
	//}

	/*for (const auto& quad : Quads) {
		quad.HasElevation ? std::cout << "Has Elevation" << std::endl : std::cout << "No Elevation" << std::endl;
		quad.HasGradient ? std::cout << "Has Gradient" << std::endl : std::cout << "No Gradient" << std::endl;
	}*/

	for (const auto& quad : Quads) {
		//std::cout << quad.CurveID << std::endl;
	}

}

void Rasterizer::BuildConstraintMaps() {

	for(const auto& quad : Quads) {

		int MinC, MinR, MaxC, MaxR;

		AABB aabb = ComputeAABB(quad.V0.Position, quad.V1.Position, quad.V2.Position, quad.V3.Position);

		MinC = floor(aabb.Min.x * (Width - 1));
		MaxC = ceil(aabb.Max.x * (Width - 1));
		MinR = floor(aabb.Min.y * (Height - 1)); // vec2라 y지만 실제로는 z좌표임
		MaxR = ceil(aabb.Max.y * (Height - 1));

		MinC = glm::clamp(MinC, 0, Width - 1);
		MaxC = glm::clamp(MaxC, 0, Width - 1);
		MinR = glm::clamp(MinR, 0, Height - 1);
		MaxR = glm::clamp(MaxR, 0, Height - 1);

		for (int r = MinR; r <= MaxR; r++) {
			for (int c = MinC; c <= MaxC; c++) {
				glm::vec2 Pos = glm::vec2((float)c / (float)(Width - 1), (float)r / (float)(Height - 1));
				int Index = r * Width + c;
				InterpolateQuad(Pos, quad, Index);

			}
		}


	}

}

AABB Rasterizer::ComputeAABB(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3 v2, const glm::vec3 v3) const {
	float MinX = std::min(std::min(v0.x, v1.x), std::min(v2.x, v3.x));
	float MaxX = std::max(std::max(v0.x, v1.x), std::max(v2.x, v3.x));
	float MinZ = std::min(std::min(v0.z, v1.z), std::min(v2.z, v3.z));
	float MaxZ = std::max(std::max(v0.z, v1.z), std::max(v2.z, v3.z));

	return AABB{ glm::vec2(MinX, MinZ), glm::vec2(MaxX, MaxZ) };
}

bool Rasterizer::Barycentric(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, float& OutU, float& OutV, float& OutW) const {

	glm::vec2 v0 = b - a;
	glm::vec2 v1 = c - a;
	glm::vec2 v2 = p - a;

	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);

	float denom = d00 * d11 - d01 * d01;
	if (abs(denom) < 1e-8f) return false;

	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	OutU = u;
	OutV = v;
	OutW = w;

	return (u >= 0) && (v >= 0) && (w >= 0);



}

void Rasterizer::InterpolateQuad(const glm::vec2& p, const Quad& quad, const int index) {

	QuadVertex V0, V1, V2;

	float u, v, w;

	if (Barycentric(p, glm::vec2(quad.V0.Position.x, quad.V0.Position.z),
		glm::vec2(quad.V1.Position.x, quad.V1.Position.z),
		glm::vec2(quad.V2.Position.x, quad.V2.Position.z),
		u, v, w)) {
		V0 = quad.V0;
		V1 = quad.V1;
		V2 = quad.V2;
	}
	else if (Barycentric(p, glm::vec2(quad.V0.Position.x, quad.V0.Position.z),
		glm::vec2(quad.V2.Position.x, quad.V2.Position.z),
		glm::vec2(quad.V3.Position.x, quad.V3.Position.z),
		u, v, w)) {
		V0 = quad.V0;
		V1 = quad.V2;
		V2 = quad.V3;
	}
	else return;

	glm::vec3 InterpolatedCurvePos = u * V0.CurvePos + v * V1.CurvePos + w * V2.CurvePos;
	glm::vec3 InterpolatedCurveNormal = glm::normalize(u * V0.CurveNormal + v * V1.CurveNormal + w * V2.CurveNormal);

	float r = u * V0.Constraint.r + v * V1.Constraint.r + w * V2.Constraint.r;
	float a = u * V0.Constraint.a + v * V1.Constraint.a + w * V2.Constraint.a;
	float b = u * V0.Constraint.b + v * V1.Constraint.b + w * V2.Constraint.b;
	float h = u * V0.Constraint.h + v * V1.Constraint.h + w * V2.Constraint.h;
	float theta = u * V0.Constraint.theta + v * V1.Constraint.theta + w * V2.Constraint.theta;
	float phi = u * V0.Constraint.phi + v * V1.Constraint.phi + w * V2.Constraint.phi;
	float Amplitude = u * V0.Constraint.Amplitude + v * V1.Constraint.Amplitude + w * V2.Constraint.Amplitude;
	float Roughness = u * V0.Constraint.Roughness + v * V1.Constraint.Roughness + w * V2.Constraint.Roughness;

	float d = glm::dot(glm::vec2(p.x - InterpolatedCurvePos.x, p.y - InterpolatedCurvePos.z),
		glm::vec2(InterpolatedCurveNormal.x, InterpolatedCurveNormal.z));
	float ad = abs(d);

	float GradientAttenuation;  //  쿼드 끝에서 경사 제약 크기가 0이 되도록 선형 감쇠
	

	bool ApplyElevation = false;
	bool ApplyGradientA = false;
	bool ApplyGradientB = false;

	//Map.ConstraintMaskMap[index] |= 128;

	// 배타적, 우선순위 : Elevation > Gradient, r 내부에서는 gradient가 적용되지 않음
	if (quad.HasElevation && (ad <= r * 0.5)) ApplyElevation = true;
	else if (quad.HasGradient && (d > 0) && (ad <= a)) ApplyGradientA = true;
	else if (quad.HasGradient && (d < 0) && (ad <= b)) ApplyGradientB = true;


	if (ApplyElevation) {
		if (Map.ConstraintMaskMap[index] & (int)ConstraintMask::Elevation) {
			if (Map.CurveIDMap[index].ElevationOwner != quad.CurveID) {
				Map.CurveIDMap[index].SumElevation += h;
				Map.CurveIDMap[index].CountElevation++;
				Map.ElevationMap[index] = Map.CurveIDMap[index].SumElevation / (float)Map.CurveIDMap[index].CountElevation;
			}
		}
		else {
			Map.ElevationMap[index] = h;
			Map.CurveIDMap[index].SumElevation += h;
			Map.CurveIDMap[index].CountElevation++;
			Map.ConstraintMaskMap[index] |= (int)ConstraintMask::Elevation;
			Map.CurveIDMap[index].ElevationOwner = quad.CurveID;

		}
	}
	if ((ApplyGradientA || ApplyGradientB) && !Map.CurveIDMap[index].GradientConflict) {  // 동일 곡선 간의 gradient 재연산 = 덮어쓰기 or max, 다른 곡선 간의 gradient 충돌 = 제거
		if (Map.ConstraintMaskMap[index] & (int)ConstraintMask::Gradient) {
			//Map.ConstraintMaskMap[index] &= ~(int)ConstraintMask::Gradient;
			//Map.GradientMap[index] = glm::vec2(0.0f, 0.0f);
			//Map.Gradients[index] = glm::vec3(0.0f, 0.0f, 0.0f);
			if (Map.CurveIDMap[index].GradientOwner != quad.CurveID) {
				Map.ConstraintMaskMap[index] &= ~(int)ConstraintMask::Gradient;
				Map.Gradients[index] = glm::vec3(0.0f, 0.0f, 0.0f);
				Map.CurveIDMap[index].GradientConflict = true;
			}
		}
		else {
			glm::vec2 n = glm::normalize(glm::vec2(InterpolatedCurveNormal.x, InterpolatedCurveNormal.z));
			if (ApplyGradientA) {
				GradientAttenuation = glm::clamp(1.0f - ad / a, 0.0f, 1.0f) * TexelSize;
				//GradientAttenuation = glm::clamp(1.0f - abs(2*(ad / a) - 1), 0.0f, 1.0f);
				Map.ConstraintMaskMap[index] |= (int)ConstraintMask::Gradient;
				Map.Gradients[index].x = n.x;
				Map.Gradients[index].y = n.y;
				Map.Gradients[index].z = GradientAttenuation * glm::tan(glm::radians(theta)); // == norm
				Map.CurveIDMap[index].GradientOwner = quad.CurveID;
			}
			else if (ApplyGradientB) {
				GradientAttenuation = glm::clamp(1.0f - ad / b, 0.0f, 1.0f) * TexelSize;
				//GradientAttenuation = glm::clamp(1.0f - abs(2*(ad / b) - 1), 0.0f, 1.0f);
				Map.ConstraintMaskMap[index] |= (int)ConstraintMask::Gradient;
				Map.Gradients[index].x = -n.x;
				Map.Gradients[index].y = -n.y;
				Map.Gradients[index].z = GradientAttenuation * glm::tan(glm::radians(phi));
				Map.CurveIDMap[index].GradientOwner = quad.CurveID;
			}

		}
	}

	bool ApplyNoise = false;
	if (quad.HasNoise) {
		if (Map.ConstraintMaskMap[index] & (int)ConstraintMask::Noise) {
			if (Map.CurveIDMap[index].NoiseOwner != quad.CurveID) {
				Map.CurveIDMap[index].CountNoise++;
				Map.CurveIDMap[index].SumNoise += glm::vec2(Amplitude, Roughness);
				Map.NoiseMap[index].x = Map.CurveIDMap[index].SumNoise.x / (float)Map.CurveIDMap[index].CountNoise;
				Map.NoiseMap[index].y = Map.CurveIDMap[index].SumNoise.y / (float)Map.CurveIDMap[index].CountNoise;
			}
		}
		else {
			Map.NoiseMap[index].x = std::max(Map.NoiseMap[index].x, Amplitude);
			Map.NoiseMap[index].y = std::max(Map.NoiseMap[index].y, Roughness);
			Map.ConstraintMaskMap[index] |= (int)ConstraintMask::Noise;
			Map.CurveIDMap[index].NoiseOwner = quad.CurveID;
			Map.CurveIDMap[index].CountNoise++;
			Map.CurveIDMap[index].SumNoise += glm::vec2(Amplitude, Roughness);
			ApplyNoise = true;

		}
	}
}
void Rasterizer::InterpolateQuadIntersectDiffCurve(const glm::vec2& p, const Quad& quad, const int index) {
	QuadVertex V0, V1, V2;

	float u, v, w;

	if (Barycentric(p, glm::vec2(quad.V0.Position.x, quad.V0.Position.z),
		glm::vec2(quad.V1.Position.x, quad.V1.Position.z),
		glm::vec2(quad.V2.Position.x, quad.V2.Position.z),
		u, v, w)) {
		V0 = quad.V0;
		V1 = quad.V1;
		V2 = quad.V2;
	}
	else if (Barycentric(p, glm::vec2(quad.V0.Position.x, quad.V0.Position.z),
		glm::vec2(quad.V2.Position.x, quad.V2.Position.z),
		glm::vec2(quad.V3.Position.x, quad.V3.Position.z),
		u, v, w)) {
		V0 = quad.V0;
		V1 = quad.V2;
		V2 = quad.V3;
	}
	else return;

	glm::vec3 InterpolatedCurvePos = u * V0.CurvePos + v * V1.CurvePos + w * V2.CurvePos;
	glm::vec3 InterpolatedCurveNormal = glm::normalize(u * V0.CurveNormal + v * V1.CurveNormal + w * V2.CurveNormal);

	float r = u * V0.Constraint.r + v * V1.Constraint.r + w * V2.Constraint.r;
	float a = u * V0.Constraint.a + v * V1.Constraint.a + w * V2.Constraint.a;
	float b = u * V0.Constraint.b + v * V1.Constraint.b + w * V2.Constraint.b;
	float h = u * V0.Constraint.h + v * V1.Constraint.h + w * V2.Constraint.h;
	float theta = u * V0.Constraint.theta + v * V1.Constraint.theta + w * V2.Constraint.theta;
	float phi = u * V0.Constraint.phi + v * V1.Constraint.phi + w * V2.Constraint.phi;
	float Amplitude = u * V0.Constraint.Amplitude + v * V1.Constraint.Amplitude + w * V2.Constraint.Amplitude;
	float Roughness = u * V0.Constraint.Roughness + v * V1.Constraint.Roughness + w * V2.Constraint.Roughness;

	float d = glm::dot(glm::vec2(p.x - InterpolatedCurvePos.x, p.y - InterpolatedCurvePos.z),
		glm::vec2(InterpolatedCurveNormal.x, InterpolatedCurveNormal.z));
	float ad = abs(d);

	float GradientAttenuation;  //  쿼드 끝에서 경사 제약 크기가 0이 되도록 선형 감쇠

	bool ApplyElevation = false;
	bool ApplyGradientA = false;
	bool ApplyGradientB = false;

	if (quad.HasElevation && (ad <= r * 0.5)) ApplyElevation = true;
	else if (quad.HasGradient && (d > 0) && (ad <= a)) ApplyGradientA = true;
	else if (quad.HasGradient && (d < 0) && (ad <= b)) ApplyGradientB = true;


	if (ApplyElevation) {
		if (Map.ConstraintMaskMap[index] & (int)ConstraintMask::Elevation) {
			Map.ElevationMap[index] = 0.5f * (Map.ElevationMap[index] + h);
		}
		else {
			Map.ElevationMap[index] = h;
			Map.ConstraintMaskMap[index] |= (int)ConstraintMask::Elevation;
		}
	}
	if (ApplyGradientA || ApplyGradientB) {  // 동일 곡선 간의 gradient 재연산 = 덮어쓰기 or max, 다른 곡선 간의 gradient 충돌 = 제거
		if (Map.ConstraintMaskMap[index] & (int)ConstraintMask::Gradient) {
			Map.ConstraintMaskMap[index] &= ~(int)ConstraintMask::Gradient;
			Map.Gradients[index] = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		else {
			glm::vec2 n = glm::normalize(glm::vec2(InterpolatedCurveNormal.x, InterpolatedCurveNormal.z));
			if (ApplyGradientA) {
				GradientAttenuation = glm::clamp(1.0f - ad / a, 0.0f, 1.0f) * TexelSize;
				Map.ConstraintMaskMap[index] |= (int)ConstraintMask::Gradient;
				Map.Gradients[index].x = n.x;
				Map.Gradients[index].y = n.y;
				Map.Gradients[index].z = GradientAttenuation * glm::tan(glm::radians(theta)); // == norm
			}
			else if (ApplyGradientB) {
				GradientAttenuation = glm::clamp(1.0f - ad / b, 0.0f, 1.0f) * TexelSize;
				Map.ConstraintMaskMap[index] |= (int)ConstraintMask::Gradient;
				Map.Gradients[index].x = -n.x;
				Map.Gradients[index].y = -n.y;
				Map.Gradients[index].z = GradientAttenuation * glm::tan(glm::radians(phi));
			}
		}
	}

	if (quad.HasNoise) {
		if (Map.ConstraintMaskMap[index] & (int)ConstraintMask::Noise) {
			Map.NoiseMap[index].x = 0.5f * (Map.NoiseMap[index].x + Amplitude);
			Map.NoiseMap[index].y = 0.5f * (Map.NoiseMap[index].y + Roughness);
		}
		else {
			Map.NoiseMap[index].x = Amplitude;
			Map.NoiseMap[index].y = Roughness;
			Map.ConstraintMaskMap[index] |= (int)ConstraintMask::Noise;
		}
	}

}

glm::ivec4 Rasterizer::GetBorderPixels2() {

	glm::ivec2 Pixel0(0, 0);
	glm::ivec2 Pixel1(0, 0);



	for (int row = 0; row < Height; ++row) {
		bool found = false;
		for (int col = 0; col < Width; ++col) {
			int Index = row * Width + col;
			using MaskT = uint8_t;
			MaskT Mask = (Map.ConstraintMaskMap[Index]);

			auto hasElevationAt = [&](int r, int c) {
				if (r < 0 || r >= Height || c < 0 || c >= Width)
					return false;
				return (Map.ConstraintMaskMap[r * Width + c] &
					static_cast<MaskT>(ConstraintMask::Elevation)) != 0;
				};

			bool hasElevation = (Mask & static_cast<MaskT>(ConstraintMask::Elevation)) != 0;
			bool hasGradient = (Mask & static_cast<MaskT>(ConstraintMask::Gradient)) != 0;
			bool nearREdge =
				hasElevationAt(row, col - 1) ||
				hasElevationAt(row, col + 1) ||
				hasElevationAt(row - 1, col) ||
				hasElevationAt(row + 1, col);
			if (!hasElevation && hasGradient && nearREdge) {
				Pixel0 = glm::ivec2(col, row);
				found = true;
				break;
			}
		}

		if (found) break;

	}

	int idx0 = Pixel0.y * Width + Pixel0.x;
	glm::vec2 n = glm::normalize(glm::vec2(
		Map.Gradients[idx0].x,
		Map.Gradients[idx0].y
	));

	glm::ivec2 dir;
	if (std::abs(n.x) > std::abs(n.y))
		dir = glm::ivec2(n.x > 0 ? 1 : -1, 0);
	else
		dir = glm::ivec2(0, n.y > 0 ? 1 : -1);

	Pixel1 = Pixel0 + dir;

	// 안전 클램프
	Pixel1.x = glm::clamp(Pixel1.x, 0, Width - 1);
	Pixel1.y = glm::clamp(Pixel1.y, 0, Height - 1);

	glm::vec2 Pos0 = glm::vec2((float)Pixel0.x / (float)(Width - 1), (float)Pixel0.y / (float)(Height - 1));
	glm::vec2 Pos1 = glm::vec2((float)Pixel1.x / (float)(Width - 1), (float)Pixel1.y / (float)(Height - 1));

	std::cout << "Border Pixel 0: (" << Pixel0.x << ", " << Pixel0.y << "), Pos: (" << Pos0.x << ", " << Pos0.y << ")" << std::endl;
	std::cout << "Border Pixel 1: (" << Pixel1.x << ", " << Pixel1.y << "), Pos: (" << Pos1.x << ", " << Pos1.y << ")" << std::endl;

	int idx1 = Pixel1.y * Width + Pixel1.x;
	printf("P0 mask = %u\n", Map.ConstraintMaskMap[idx0]);
	printf("P1 mask = %u\n", Map.ConstraintMaskMap[idx1]);

	return glm::ivec4(Pixel0.x, Pixel0.y, Pixel1.x, Pixel1.y);

}

float Rasterizer::GradientAttenuation(glm::vec3& DstGradient, const glm::vec2& Pos, const glm::vec2& CurvePos, const glm::vec2& CurveNormal, 
									  const float SignedDistance, const float GradientRadius, float GradientMagnitude) {



}