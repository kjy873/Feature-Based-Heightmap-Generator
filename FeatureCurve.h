#pragma once

#include <vector>
#include "Mesh.h"
#include <glew.h>


#include <glm.hpp>

#include "BufferManager.h"

namespace FC {

	class ControlPoint {

		glm::vec3 Position;


		//ControlPointVisualMesh* Mesh;

		std::unique_ptr<ControlPointVisualMesh> Mesh;

		int Index = -1;


	public:

		ControlPoint() : Position(glm::vec3(0.0f)), Index(-1), Mesh(nullptr) {}
		ControlPoint(glm::vec3 pos, int Index) : Position(pos), Index(Index), Mesh(std::make_unique<ControlPointVisualMesh>(8)) {};
		~ControlPoint() = default;

		ControlPoint(const ControlPoint&) = delete;
		ControlPoint& operator=(const ControlPoint&) = delete;

		ControlPoint(ControlPoint&&) noexcept = default;
		ControlPoint& operator=(ControlPoint&&) noexcept = default;

		void SetMesh();
		ControlPointVisualMesh* GetMesh() const { return Mesh.get(); }

		int GetIndex() const { return Index; }

		glm::vec3 GetPosition() const { return Position; }


	};

}

class FeatureCurve
{

	std::vector<FC::ControlPoint> ControlPoints;

	int CurveID = -1;




public:

	FeatureCurve() {
	};
	FeatureCurve(int id) : CurveID(id) {
	};
	~FeatureCurve() = default;

	FeatureCurve(const FeatureCurve&) = delete;
	FeatureCurve& operator=(const FeatureCurve&) = delete;

	FeatureCurve(FeatureCurve&&) noexcept = default;
	FeatureCurve& operator=(FeatureCurve&&) noexcept = default;

	int GetCurveID() const { return CurveID; }

	void AddControlPoint(const glm::vec3& pos);

	const std::vector<FC::ControlPoint>& GetControlPoints() const { return ControlPoints; }

	void UploadBuffer(BufferManager& BufferMgr);

};

enum class EditCurveState
{
	Idle,
	PlacingP1,
	PlacingP3,
	PlacingP2
};

class FeatureCurveManager
{
	std::vector<FeatureCurve> FeatureCurves;

	int SelectedID = -1;

	int NextCurveID = 0;

	EditCurveState State = EditCurveState::Idle;

public:

	FeatureCurveManager() {}
	~FeatureCurveManager() {}

	void AddFeatureCurve();

	int GetCount() const { return static_cast<int>(FeatureCurves.size()); }
	int GetRemainder(FeatureCurve* Curve) const;

	int CreateCurveID() { return NextCurveID++; }
	FeatureCurve* GetFeatureCurve(int id);

	void SelectCurve(int id) { SelectedID = id; }
	int GetSelectedCurveID() const { return SelectedID; }

	void LeftClick(const glm::vec3& Pos, int Tangent);
	void RightClick();

	void UploadBuffers(BufferManager& BufferMgr);

	std::vector<FeatureCurve>& GetCurves() { return FeatureCurves; }

	const glm::vec3 AppliedTangentPos(const glm::vec3 P0, const glm::vec3& Pos, int Tangent) const;



};