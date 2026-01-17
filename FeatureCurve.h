#pragma once

#include <vector>
#include "Mesh.h"
#include <glew.h>

#include <glm.hpp>

#include "BufferManager.h"

namespace FC {

	class ControlPoint {

		glm::vec3 Position;

		ControlPointVisualMesh* Mesh;

		int Index = -1;


	public:

		ControlPoint() : Position(glm::vec3(0.0f)), Index(-1), Mesh(nullptr) {}
		ControlPoint(glm::vec3 pos, int Index);
		~ControlPoint() {
			if (Mesh) {
				delete Mesh;
				Mesh = nullptr;
			}
		}

		void SetMesh();
		ControlPointVisualMesh* GetMesh() const { return Mesh; }

		int GetIndex() const { return Index; }


	};

}

class FeatureCurve
{

	std::vector<FC::ControlPoint> ControlPoints;


public:

	FeatureCurve() {
		ControlPoints.resize(4);
	};
	~FeatureCurve(){}

	void AddControlPoint(const glm::vec3& pos, int Index);

	const std::vector<FC::ControlPoint>& GetControlPoints() const { return ControlPoints; }

	void UploadBuffer(BufferManager& BufferMgr);

};

