#pragma once

#include "glm.hpp"
#include <vector>


	enum class ConstraintMask
	{
		None = 0,
		Elevation = 1 << 0,
		Gradient = 1 << 1,
		Noise = 1 << 2,
	};

	inline ConstraintMask operator|(ConstraintMask a, ConstraintMask b)
	{
		return static_cast<ConstraintMask>(
			static_cast<int>(a) | static_cast<int>(b)
			);
	}

	inline ConstraintMask operator&(ConstraintMask a, ConstraintMask b)
	{
		return static_cast<ConstraintMask>(
			static_cast<int>(a) & static_cast<int>(b)
			);
	}

	inline bool HasMask(ConstraintMask value, ConstraintMask flag)
	{
		return (value & flag) != ConstraintMask::None;
	}

	struct Constraints {
		float h = 0.0f;
		float r = 0.0f;

		float a = 0.0f;
		float b = 0.0f;
		float theta = 0.0f;
		float phi = 0.0f;

		float Amplitude = 0.0f;
		float Roughness = 0.0f;

		float u = 0.0f;

		ConstraintMask Mask;
	};

	struct LinearCoord {
		glm::vec3 Pos;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		float u;
	};

	struct CurveData {
		std::vector<glm::vec3> ControlPoints;
		std::vector<Constraints> ConstraintPoints;
		std::vector<LinearCoord> PolylineSamples;


		CurveData() {};
		CurveData(std::vector<glm::vec3> Control, std::vector<Constraints> Constraint) : ControlPoints(Control), ConstraintPoints(Constraint) {};

		CurveData& operator=(const CurveData& other) {
			if (this != &other) {
				ControlPoints = other.ControlPoints;
				ConstraintPoints = other.ConstraintPoints;
			}
			return *this;
		}
	};

	inline glm::vec3 BezierCubic(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3, float t) {
		float u = 1.0f - t;
		return (u * u * u * P0 + 3.0f * u * u * t * P1 + 3.0f * u * t * t * P2 + t * t * t * P3);
	}
