#pragma once

#include <vector>
#include "Mesh.h"
#include <glew.h>
#include <optional>

#include <glm.hpp>

#include "BufferManager.h"

enum class InputButton
{
	Left,
	Right
};;

enum class InputMode
{
	Default,
	Ctrl,
	Shift
};

struct AABBXZ {
	glm::vec2 Min;
	glm::vec2 Max;
	bool Valid = false;
};

namespace FC {

	class ControlPoint {

		glm::vec3 Position;


		//ControlPointVisualMesh* Mesh;

		std::unique_ptr<ControlPointVisualMesh> Mesh;

		float half = 0.005f;


	public:

		ControlPoint() : Position(glm::vec3(0.0f)), Mesh(nullptr) {}
		ControlPoint(glm::vec3 pos) : Position(pos), Mesh(std::make_unique<ControlPointVisualMesh>(8)) {};
		~ControlPoint() = default;

		ControlPoint(const ControlPoint&) = delete;
		ControlPoint& operator=(const ControlPoint&) = delete;

		ControlPoint(ControlPoint&&) noexcept = default;
		ControlPoint& operator=(ControlPoint&&) noexcept = default;

		void SetMesh();
		ControlPointVisualMesh* GetMesh() const { return Mesh.get(); }

		glm::vec3 GetPosition() const { return Position; }
		float GetHalf() const { return half; }


	};

}

class FeatureCurve
{

	std::vector<FC::ControlPoint> ControlPoints;

	int CurveID = -1;

	std::vector<glm::vec3> Vertices;

	std::unique_ptr<LineMesh> Line;

	bool LineDirty = false;

	AABBXZ BoundingBox;



	int SamplePerSegment = 64;

public:

	FeatureCurve() {};
	FeatureCurve(int id) : CurveID(id) {};
	~FeatureCurve() = default;

	FeatureCurve(const FeatureCurve&) = delete;
	FeatureCurve& operator=(const FeatureCurve&) = delete;

	FeatureCurve(FeatureCurve&&) noexcept = default;
	FeatureCurve& operator=(FeatureCurve&&) noexcept = default;

	int GetCurveID() const { return CurveID; }

	void AddControlPoint(const glm::vec3& pos);
	void AddControlPoint(FC::ControlPoint&& cp) { ControlPoints.push_back(std::move(cp)); }

	const std::vector<FC::ControlPoint>& GetControlPoints() const { return ControlPoints; }

	void UploadBuffer(BufferManager& BufferMgr);
	void UploadBufferLine(BufferManager& BufferMgr);

	void PopBack() { ControlPoints.pop_back(); }

	glm::vec3 BezierCubic(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3, float t) const {
		float u = 1.0f - t;
		return u * u * u * P0 + 3.0f * u * u * t * P1 + 3.0f * u * t * t * P2 + t * t * t * P3;
	}

	void BuildLines();

	LineMesh* GetLineMesh() const { return Line.get(); }

	void UpdateBoundingBox();
	void UpdateBoundingBox(const glm::vec3 Point);

	AABBXZ GetBoundingBox() const { return BoundingBox; }

	bool PointInBoundingBox(const glm::vec3 Pos) const {
		if (!BoundingBox.Valid) return false;
		return (Pos.x >= BoundingBox.Min.x && Pos.x <= BoundingBox.Max.x &&
			Pos.z >= BoundingBox.Min.y && Pos.z <= BoundingBox.Max.y);
	}

	float DistancePointToLineSq(const glm::vec3 Point, const glm::vec3 LineStart, const glm::vec3 LineEnd) const;

	float NearestDistanceSq(const glm::vec3 Point);


};

enum class EditCurveState
{
	P0,
	P1,
	P3,
	P2,
	CurveSelected,
	None

};

enum class PickType { Curve, ControlPoint, None };

enum class Decision { 
	None,
	SelectCurve,
	SelectControlPoint,
	Deselect,
	AddCurve,
	ExtendCurve,
	AddControlPoint,
	CommitSegment,
	Cancel,
	DeleteSelectedControlPoint,
	DeleteSelectedCurve


};

struct PickResult
{
	PickType Type = PickType::None;
	int CurveID = -1;
	int ControlPointIndex = -1;
};

class FeatureCurveManager
{
	std::vector<FeatureCurve> FeatureCurves;

	int SelectedID = -1;

	int NextCurveID = 0;

	EditCurveState State = EditCurveState::None;

	std::optional<FC::ControlPoint> Pended;

	float r = 0.05f;

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

	void Click(const glm::vec3& Pos, InputButton Button, InputMode Mode);
	void RightClick();

	void UploadBuffers(BufferManager& BufferMgr);

	std::vector<FeatureCurve>& GetCurves() { return FeatureCurves; }

	const glm::vec3 AppliedTangentPos(const glm::vec3 P0, const glm::vec3& Pos, int Tangent) const;

	void PendControlPoint(const glm::vec3& Pos);
	void UploadPendedBuffer(BufferManager& BufferMgr);
	std::optional<FC::ControlPoint>& GetPendedControlPoint() { return Pended; }

	bool SegmentComplete(int Count) const{ return (Count >= 4) && ((Count - 1) % 3 == 0); }

	void UpdateAllBoundingBoxes() { for (auto& Curve : FeatureCurves) Curve.UpdateBoundingBox(); }

	PickResult Pick(const glm::vec3& Pos);
	PickResult PickControlPoint(const glm::vec3& Pos);
	PickResult PickCurve(const glm::vec3& Pos);

	Decision Decide(InputButton Button, InputMode Mode, EditCurveState State, PickResult Picked);
	void Execute(Decision DecidedResult);

};