#pragma once

#include <vector>
#include "Mesh.h"
#include <glew.h>
#include <optional>
#include <algorithm>

#include <glm.hpp>

#include "BufferManager.h"
#include "Features.h"

struct SplitRequest {
	int JunctionIndex;
	float u;
};

struct JunctionLinked {
	int CurveID;
	float u;
	int Segment;
	float t;
};

struct JunctionData {
	glm::vec3 Pos;
	std::vector<JunctionLinked> LinkedCurves;
};

struct SaveData {
	int CurveCount = 0;
	std::vector<std::vector<glm::vec3>> ControlPoints;
	std::vector<std::vector<Constraints>> Constraints;
	std::vector<std::vector<glm::vec3>> ConstraintPos;
};

struct CurvePoint
{
	glm::vec3 Position;
	float u = 0.0f;

	int Segment;
	float t;

	CurvePoint() : Position(glm::vec3(0.0f)), u(0.0f), Segment(-1), t(0.0f) {};
	CurvePoint(const glm::vec3& pos, float uu, int segment, float t) : Position(pos), u(uu), Segment(segment), t(t) {};
};

struct ConstraintPoint
{

	int ID = -1;

	bool StartEndPoint = false; // 시작점, 끝점 여부

	bool Cached = false;
	glm::vec3 CachedPos = glm::vec3(0.0f, 0.0f, 0.0f); // FeatureCurve에서 u->position 변환 함수를 만들 것. 필요 시 여기서 캐싱 

	Constraints Data;
	//float u = 0.0f;
	bool KeepU = false;

	std::unique_ptr<ControlPointVisualMesh> Mesh;

	float HighlightWeight = 0.0f;

	bool Uploaded = false;

	float half = 0.005f;

	int LinkedJunctionNode = -1;

	ConstraintPoint() : Mesh(std::make_unique<ControlPointVisualMesh>(8)) {};
	ConstraintPoint(int id) : ID(id), Mesh(std::make_unique<ControlPointVisualMesh>(8)) {}
	ConstraintPoint(const glm::vec3& Pos) : CachedPos(Pos), Cached(true), Mesh(std::make_unique<ControlPointVisualMesh>(8)) {}
	ConstraintPoint(int id, const glm::vec3& Pos) : ID(id), CachedPos(Pos), Cached(true), Mesh(std::make_unique<ControlPointVisualMesh>(8)) {}
	ConstraintPoint(int id, const glm::vec3& Pos, float uu) : ID(id), CachedPos(Pos), Cached(true), Mesh(std::make_unique<ControlPointVisualMesh>(8)) 
	{
		Data.u = uu;
	}



	void CreateMesh() { Mesh = std::make_unique<ControlPointVisualMesh>(8); }
	void SetMesh() { Mesh->SetHexahedron(CachedPos, half, glm::vec3(0.0f, 1.0f, 1.0f)); }

	ControlPointVisualMesh* GetMesh() const { return Mesh.get(); }

	const glm::vec3& GetPosition() const { return CachedPos; }
	float GetHalf() const { return half; }

	float GetHighlightWeight() const { return HighlightWeight; }
	void SetHighlightWeight(float weight) { HighlightWeight = weight; }

	int GetID() const { return ID; }

	const Constraints& GetConstraints() const { return Data; }
	const ConstraintMask& GetConstraintMask() const { return Data.Mask; }

	void SetConstraints(const Constraints& InputConstraints) { Data = InputConstraints; }
	void SetConstraintMask(const ConstraintMask& InputConstraintMask) { Data.Mask = InputConstraintMask; }

	void LinkToJunctionNode(int JunctionID) { LinkedJunctionNode = JunctionID; }
	int GetLinkedJunctionNodeID() const { return LinkedJunctionNode; }

};

class JunctionNode : public ConstraintPoint
{

	int ID = -1;

public:

	JunctionNode() = default;
	JunctionNode(glm::vec3 Pos, int ID) : ConstraintPoint(ID, Pos) {};

	void SetPosition(const glm::vec3& Pos) { CachedPos = Pos; Cached = true; SetMesh(); }

};

enum class InputButton
{
	Left,
	Right
};

enum class InputMode
{
	Default,
	Ctrl,
	Shift,
	Alt
};

struct AABBXZ {
	glm::vec2 Min;
	glm::vec2 Max;
	bool Valid = false;
};

namespace FC {

	class ControlPoint {

		glm::vec3 Position;

		std::unique_ptr<ControlPointVisualMesh> Mesh;

		float HighlightWeight = 0.0f;

		float half = 0.005f;

		int ID = -1;

		bool Dirty = true;

		int LinkedJunctionNode = -1;

	public:

		ControlPoint() : Position(glm::vec3(0.0f)), Mesh(std::make_unique<ControlPointVisualMesh>(8)) {};
		ControlPoint(int id) : ID(id), Position(glm::vec3(0.0f)), Mesh(std::make_unique<ControlPointVisualMesh>(8)) {};
		ControlPoint(glm::vec3 pos) : Position(pos), Mesh(std::make_unique<ControlPointVisualMesh>(8)) {};
		ControlPoint(int id, glm::vec3 pos) : ID(id), Position(pos), Mesh(std::make_unique<ControlPointVisualMesh>(8)) {};
		~ControlPoint() = default;

		ControlPoint(const ControlPoint&) = delete;
		ControlPoint& operator=(const ControlPoint&) = delete;

		ControlPoint(ControlPoint&&) noexcept = default;
		ControlPoint& operator=(ControlPoint&&) noexcept = default;

		void SetPosition(const glm::vec3& pos) { Position = pos; }
		void SetMesh();
		void CreateMesh() { Mesh = std::make_unique<ControlPointVisualMesh>(8); }
		ControlPointVisualMesh* GetMesh() const { return Mesh.get(); }

		glm::vec3 GetPosition() const { return Position; }
		float GetHalf() const { return half; }

		float GetHighlightWeight() const { return HighlightWeight; }
		void SetHighlightWeight(float weight) { HighlightWeight = weight; }

		int GetID() const { return ID; }

		bool GetDirty() const { return Dirty; }

		void LinkToJunctionNode(int JunctionID) { LinkedJunctionNode = JunctionID; }
		int GetLinkedJunctionNodeID() const { return LinkedJunctionNode; }
	};

}

class FeatureCurve
{

	std::vector<FC::ControlPoint> ControlPoints;

	int CurveID = -1;

	std::vector<CurvePoint> SamplePoints;

	std::unique_ptr<LineMesh> Line;
	float HighlightWeight = 0.0f;

	bool LineDirty = false;

	AABBXZ BoundingBox;

	std::vector<ConstraintPoint> ConstraintPoints;

	int SamplePerSegment = 128;

	int NextControlPointID = 0;
	int NextConstraintPointID = 0;

	int CommittedSegments = 0;


	static inline glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t) { return a * (1.0f - t) + b * t; }

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
	void UploadBufferConstraintPoint(BufferManager& BufferMgr);

	void PopBack() { ControlPoints.pop_back(); }

	void BuildLines();
	void BuildLinesLength();

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

	void AddConstraintPoint(const glm::vec3 Pos, const float u);

	int FindNearestCurvePoint(const glm::vec3 Pos);
	int FIndNearestCurvePointByU(const float u);

	const std::vector<ConstraintPoint>& GetConstraintPoints() const { return ConstraintPoints; }
	std::vector<ConstraintPoint>& GetConstraintPoints() { return ConstraintPoints; }

	const std::vector<CurvePoint>& GetSamplePoints() const { return SamplePoints; }

	float GetHighlightWeight() const { return HighlightWeight; }
	void SetHighlightWeight(float weight) { HighlightWeight = weight; }

	FC::ControlPoint* GetControlPointPtr(int ID);
	ConstraintPoint* GetConstraintPointPtr(int ID);

	FC::ControlPoint& GetControlPoint(int id);
	ConstraintPoint& GetConstraintPoint(int id);

	int FindConstraintPointByU(float u) const;

	int CreateControlPointID() { return NextControlPointID++; }

	int CommitSegment() { return ++CommittedSegments; }
	int GetCommittedSegments() const { return CommittedSegments; }

	void ProcessSplitRequest(const SplitRequest& Request);
	bool GetSegTFromU(float u, int& OutSeg, float& OutT);
	


};

enum class EditCurveState
{
	P0,
	P1,
	P3,
	P2,
	CurveSelected,
	ControlPointSelected,
	ConstraintPointSelected,
	JunctionNodeSelected,
	None

};

enum class PickType { Curve, ControlPoint, ConstraintPoint, JunctionNode, None };

enum class Decision { 
	None,
	SelectCurve,
	SelectControlPoint,
	SelectConstraintPoint,
	Deselect,
	AddCurve,
	ExtendCurve,
	AddControlPoint,
	CommitSegment,
	Cancel,
	DeleteSelectedControlPoint,
	DeleteSelectedCurve,
	AddConstraintPoint,
	SelectJunctionNode


};

struct CurveManagerView {
	
	bool ConstraintPointPanelOpen = false;

	int SelectedCurveID = -1;
	int SelectedControlPointID = -1;
	int SelectedConstraintPointID = -1;
	int SelectedJunctionNodeID = -1;

};

struct PickResult
{
	PickType Type = PickType::None;
	int CurveID = -1;
	int ControlPointID = -1;
	int ConstraintPointID = -1;
	int JunctionNodeID = -1;
};

class FeatureCurveManager
{
	std::vector<FeatureCurve> FeatureCurves;

	int SelectedCurveID = -1;
	int SelectedControlPointID = -1;
	int SelectedConstraintPointID = -1;
	int SelectedJunctionNodeID = -1;

	int NextCurveID = 0;
	int NextJunctionNodeID = 0;

	EditCurveState State = EditCurveState::None;

	std::optional<FC::ControlPoint> Pended;

	float r = 0.05f;

	std::vector<JunctionNode> JunctionNodes;
	

	FC::ControlPoint HoveringControlPoint = FC::ControlPoint(glm::vec3(0.0f, 0.0f, 0.0f));
	bool HoverIsDirty = false;

	ConstraintPoint HoveringConstraintPoint = ConstraintPoint(glm::vec3(0.0f, 0.0f, 0.0f));

	CurveManagerView View;


	JunctionNode* GetJunctionNodePtr(int ID);

public:

	FeatureCurveManager() { 
		HoveringControlPoint.SetMesh(); 
		HoverIsDirty = true; 

		//HoveringConstraintPoint.CreateMesh();
		HoveringConstraintPoint.SetMesh();
		HoveringConstraintPoint.Cached = true;
		HoveringConstraintPoint.Uploaded = true;
	}
	~FeatureCurveManager() {}

	void AddFeatureCurve();

	int GetCount() const { return static_cast<int>(FeatureCurves.size()); }
	int GetRemainder(FeatureCurve* Curve) const;

	int CreateCurveID() { return NextCurveID++; }
	FeatureCurve* GetFeatureCurve(int id);

	void SelectCurve(int id) { SelectedCurveID = id; }
	int GetSelectedCurveID() const { return SelectedCurveID; }

	void Click(const glm::vec3& Pos, InputButton Button, InputMode Mode);
	void RightClick();

	void UploadBuffers(BufferManager& BufferMgr);
	void UploadBufferJunctionNode(BufferManager& BufferMgr);

	std::vector<FeatureCurve>& GetCurves() { return FeatureCurves; }

	const glm::vec3 AppliedTangentPos(const glm::vec3 P0, const glm::vec3& Pos, int Tangent) const;

	void PendControlPoint(const glm::vec3& Pos);
	void UploadPendedBuffer(BufferManager& BufferMgr);
	std::optional<FC::ControlPoint>& GetPendedControlPoint() { return Pended; }

	bool SegmentComplete(int Count) const{ return (Count >= 4) && ((Count - 1) % 3 == 0); }

	void UpdateAllBoundingBoxes() { for (auto& Curve : FeatureCurves) Curve.UpdateBoundingBox(); }

	PickResult Pick(const glm::vec3& Pos);
	PickResult PickControlPoint(const glm::vec3& Pos);
	PickResult PickConstraintPoint(const glm::vec3& Pos);
	PickResult PickCurve(const glm::vec3& Pos);
	PickResult PickJunctionNode(const glm::vec3& Pos);


	Decision Decide(InputButton Button, InputMode Mode, EditCurveState State, const PickResult& Picked);
	void Execute(Decision DecidedResult, const PickResult& Picked, const glm::vec3& Pos);

	void SelectCurve(const PickResult& Picked);
	void SelectControlPoint(const PickResult& Picked);
	void SelectConstraintPoint(const PickResult& Picked);
	void SelectJunctionNode(const PickResult& Picked);
	void ExtendCurve(const PickResult& Picked);
	void AddControlPoint(const glm::vec3& Pos);

	void PrintState() const;
	void PrintDecision(const Decision& Decision) const;

	void DeselectCurve();
	void DeselectControlPoint();
	void DeselectConstraintPoint();
	void DeselectAll();
	void Cancel();

	void HoverPressedCtrl(const glm::vec3& Pos);
	bool GetHoveringControlPointDirty() { return HoverIsDirty; }
	void SetHoveringControlPointDirty(bool Dirty) { HoverIsDirty = Dirty; }
	const FC::ControlPoint& GetHoveringControlPoint() const { return HoveringControlPoint; }

	void HoverPressedShift(const glm::vec3& Pos);
	bool GetHoveringConstraintPointDirty() { return HoveringConstraintPoint.Uploaded; }
	void SetHoveringConstraintPointDirty(bool Dirty) { HoveringConstraintPoint.Uploaded = Dirty; }
	const ConstraintPoint& GetHoveringConstraintPoint() const { return HoveringConstraintPoint; }		

	int FeatureCurveManager::FindNearestCurvePointInSelecting(const glm::vec3& Pos);

	void AddConstraintPoint(const glm::vec3& Pos);

	CurveManagerView& GetCurveManagerView() { 
		View.SelectedCurveID = SelectedCurveID;
		View.SelectedControlPointID = SelectedControlPointID;
		View.SelectedConstraintPointID = SelectedConstraintPointID;
		View.SelectedJunctionNodeID = SelectedJunctionNodeID;
		return View; 
	};

	void UpdateConstraintPointPosByU(int CurveID, int ConstraintPointID); // ConstraintPoint의 u값에 따라 위치를 업데이트
	void UpdateLastConstraint(int CurveID); // 마지막 ConstraintPoint의 u값에 따라 위치를 업데이트

	void UpdateConstraintPoints(int CurveID); // 모든 ConstraintPoint의 u값에 따라 위치를 업데이트

	void PrintPickResult(const PickResult& Picked) const;

	const std::vector<CurveData> ExtractCurveData();

	SaveData ExtractSaveData();
	void ImportSaveData(const SaveData& Data);
	void ClearFeatureCurveManager() {
		FeatureCurves.clear();
		SelectedCurveID = -1;
		SelectedControlPointID = -1;
		SelectedConstraintPointID = -1;
		SelectedJunctionNodeID = -1;
		NextCurveID = 0;
		State = EditCurveState::None;
		Pended.reset();
		View = CurveManagerView();
	}

	void DeleteSelectedCurve();

	void MoveSelectedControlPoint(const glm::vec3& Pos);
	void MoveSelectedJunctionNode(const glm::vec3& Pos);

	const std::vector<JunctionData> FindJunctions();

	bool IntersectLine2D(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& q0, const glm::vec2& q1, float& OutAlpha, float& OutBeta, glm::vec2& OutPos, const float EPS = 1e-7f);

	int FindMergeableJunction(const std::vector<JunctionData>& Junctions, const glm::vec2& Pos, float MergeEPS2);

	void MergeJunctions(std::vector<JunctionLinked>& Junctions, const JunctionLinked& New);

	void Weld();

	std::vector<JunctionNode>& GetJunctionNodes() { return JunctionNodes; }

	JunctionNode& GetJunctionNode(int ID);

	void ApplyJunctionConstraint(int JunctionID);
};