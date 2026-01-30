#include "FeatureCurve.h"

void FC::ControlPoint::SetMesh() {

	Mesh->SetHexahedron(Position, half, glm::vec3(1.0f, 0.0f, 1.0f));

}
	
void FeatureCurve::AddControlPoint(const glm::vec3& pos) {

	FC::ControlPoint cp(NextControlPointID++, pos);

	cp.SetMesh();

	ControlPoints.emplace_back(std::move(cp));

}

void FeatureCurve::UploadBuffer(BufferManager& BufferMgr) {
	for (const auto& cp : ControlPoints) {
		if (cp.GetMesh() == nullptr) continue;
		ControlPointVisualMesh* mesh = cp.GetMesh();
		if (mesh) {
			mesh->SetMeshID(BufferMgr.CreateMeshID());
			BufferMgr.CreateBufferData(mesh->GetMeshID(), true);
			BufferMgr.BindVertexBufferObjectByID(mesh->GetMeshID(), mesh->GetPosition().data(), mesh->GetPosition().size(),
				mesh->GetColor().data(), mesh->GetColor().size(), mesh->GetNormal().data(), mesh->GetNormal().size());
			BufferMgr.BindElementBufferObjectByID(mesh->GetMeshID(), mesh->GetIndex().data(), mesh->GetIndex().size());
		}
	}
}

void FeatureCurve::BuildLines() {

	SamplePoints.clear();

	int Count = ControlPoints.size();
	if (Count < 4 || (Count - 1) % 3 != 0) return;

	int SegmentCount = (Count - 1) / 3;

	for (int i = 0; i + 3 < Count; i += 3) {
		const glm::vec3& P0 = ControlPoints[i].GetPosition();
		const glm::vec3& P1 = ControlPoints[i + 1].GetPosition();
		const glm::vec3& P2 = ControlPoints[i + 2].GetPosition();
		const glm::vec3& P3 = ControlPoints[i + 3].GetPosition();

		int SamplingStart = (i == 0) ? 0 : 1;

		int seg = i / 3;

		for (int j = SamplingStart; j <= SamplePerSegment; j++) {
			float t = (float)j / (float)SamplePerSegment;

			glm::vec3 Pos = BezierCubic(P0, P1, P2, P3, t);
			float u = (seg + t) / (float)SegmentCount;

			SamplePoints.emplace_back(Pos, u);
		}
	}

	LineDirty = true;
}

void FeatureCurve::BuildLinesLength() {

	SamplePoints.clear();

	int Count = ControlPoints.size();
	if (Count < 4 || (Count - 1) % 3 != 0) return;

	int SegmentCount = (Count - 1) / 3;

	for (int i = 0; i + 3 < Count; i += 3) {
		const glm::vec3& P0 = ControlPoints[i].GetPosition();
		const glm::vec3& P1 = ControlPoints[i + 1].GetPosition();
		const glm::vec3& P2 = ControlPoints[i + 2].GetPosition();
		const glm::vec3& P3 = ControlPoints[i + 3].GetPosition();

		int SamplingStart = (i == 0) ? 0 : 1;

		for (int j = SamplingStart; j <= SamplePerSegment; j++) {
			float t = (float)j / (float)SamplePerSegment;

			glm::vec3 Pos = BezierCubic(P0, P1, P2, P3, t);

			SamplePoints.emplace_back(Pos, 0.0f);
		}
	}

	if (SamplePoints.size() >= 2) {
		float TotalLength = 0.0f;

		SamplePoints[0].u = 0.0f;

		for (int i = 1; i < SamplePoints.size(); i++) {
			TotalLength += glm::length(SamplePoints[i].Position - SamplePoints[i - 1].Position);

			SamplePoints[i].u = TotalLength;
		}

		if (TotalLength > 0) for (auto& sp : SamplePoints) sp.u /= TotalLength;
		else for (auto& sp : SamplePoints) sp.u = 0.0f;

		SamplePoints.back().u = 1.0f;

	}
	
	LineDirty = true;
}

void FeatureCurve::UploadBufferLine(BufferManager& BufferMgr) {
	if (SamplePoints.empty()) return;
	if (!LineDirty) return;

	if (!Line) {
		Line = std::make_unique<LineMesh>(SamplePoints.size());
		Line->SetMeshID(BufferMgr.CreateMeshID());
		BufferMgr.CreateBufferData(Line->GetMeshID(), false);
	}

	std::vector<glm::vec3> Positions;

	for (const auto& sp : SamplePoints) {
		Positions.emplace_back(sp.Position);
	}

	Line->SetLines(Positions, glm::vec3(1.0f, 1.0f, 1.0f));

	BufferMgr.BindVertexBufferObjectByID(Line->GetMeshID(), Line->GetPosition().data(), Line->GetPosition().size(),
		Line->GetColor().data(), Line->GetColor().size(),
		nullptr, 0);

	LineDirty = false;

}

void FeatureCurve::UploadBufferConstraintPoint(BufferManager& BufferMgr) {
	for (const auto& cp : ConstraintPoints) {
		if (cp.GetMesh() == nullptr) continue;
		ControlPointVisualMesh* mesh = cp.GetMesh();
		if (mesh) {
			mesh->SetMeshID(BufferMgr.CreateMeshID());
			BufferMgr.CreateBufferData(mesh->GetMeshID(), true);
			BufferMgr.BindVertexBufferObjectByID(mesh->GetMeshID(), mesh->GetPosition().data(), mesh->GetPosition().size(),
				mesh->GetColor().data(), mesh->GetColor().size(), mesh->GetNormal().data(), mesh->GetNormal().size());
			BufferMgr.BindElementBufferObjectByID(mesh->GetMeshID(), mesh->GetIndex().data(), mesh->GetIndex().size());
		}

	}
}

void FeatureCurve::UpdateBoundingBox() {

	BoundingBox.Valid = false;

	if (ControlPoints.empty()) return;

	float MinX{ 0.0f }, MinZ{ 0.0f }, MaxX{ 0.0f }, MaxZ{ 0.0f };
	
	for (const auto& cp : ControlPoints) {
		const glm::vec3 p = cp.GetPosition();

		MinX = std::min(MinX, p.x);
		MinZ = std::min(MinZ, p.z);
		MaxX = std::max(MaxX, p.x);
		MaxZ = std::max(MaxZ, p.z);
	}

	BoundingBox.Min = glm::vec2(MinX, MinZ);
	BoundingBox.Max = glm::vec2(MaxX, MaxZ);

	BoundingBox.Valid = true;

}

void FeatureCurve::UpdateBoundingBox(const glm::vec3 Point) {
	if (!BoundingBox.Valid) {
		BoundingBox.Min = glm::vec2(Point.x, Point.z);
		BoundingBox.Max = glm::vec2(Point.x, Point.z);
		BoundingBox.Valid = true;
		return;
	}
	BoundingBox.Min.x = std::min(BoundingBox.Min.x, Point.x);
	BoundingBox.Min.y = std::min(BoundingBox.Min.y, Point.z);
	BoundingBox.Max.x = std::max(BoundingBox.Max.x, Point.x);
	BoundingBox.Max.y = std::max(BoundingBox.Max.y, Point.z);
}

float FeatureCurve::DistancePointToLineSq(const glm::vec3 Point, const glm::vec3 LineStart, const glm::vec3 LineEnd) const {

	glm::vec3 AB = LineEnd - LineStart;

	glm::vec3 AP = Point - LineStart;

	float t = glm::dot(AP, AB) / glm::dot(AB, AB);

	t = glm::clamp(t, 0.0f, 1.0f);

	glm::vec3 ClosestPoint = LineStart + t * AB;

	return glm::dot(Point - ClosestPoint, Point - ClosestPoint);
	
}

float FeatureCurve::NearestDistanceSq(const glm::vec3 Point) {

	if(SamplePoints.size() < 2) return std::numeric_limits<float>::infinity();

	float Nearest = std::numeric_limits<float>::max();

	for (int i = 0; i < SamplePoints.size() - 1; i++) {
		float Distance = DistancePointToLineSq(Point, SamplePoints[i].Position, SamplePoints[i + 1].Position);
		if (Distance < Nearest) {
			Nearest = Distance;
		}
	}

	return Nearest;

}

void FeatureCurve::AddConstraintPoint(const glm::vec3 Pos, const float u) {

	ConstraintPoint cp(NextConstraintPointID++, Pos, u);

	cp.SetMesh();
		
	ConstraintPoints.emplace_back(std::move(cp));
	
}

int FeatureCurve::FindNearestCurvePoint(const glm::vec3 Pos) {

	int BestIndex = -1;
	float NearestDistSq = std::numeric_limits<float>::max();

	for (int i = 0; i < SamplePoints.size(); i++) {
		glm::vec3 a = SamplePoints[i].Position;
		glm::vec3 b = Pos;
		float DistSq = glm::dot(a - b, a - b);

		if (DistSq < NearestDistSq) {
			NearestDistSq = DistSq;
			BestIndex = i;
		}
	}

	return BestIndex;

}

FC::ControlPoint* FeatureCurve::GetControlPointPtr(int ID) {
	auto it = std::find_if(ControlPoints.begin(), ControlPoints.end(), [ID](const FC::ControlPoint& cp) { return cp.GetID() == ID; });
	return (it == ControlPoints.end()) ? nullptr : &(*it);
}

ConstraintPoint* FeatureCurve::GetConstraintPointPtr(int ID) {
	auto it = std::find_if(ConstraintPoints.begin(), ConstraintPoints.end(), [ID](const ConstraintPoint& cp) { return cp.GetID() == ID; });
	return (it == ConstraintPoints.end()) ? nullptr : &(*it);
}

FC::ControlPoint& FeatureCurve::GetControlPoint(int id) {
	FC::ControlPoint* cp = GetControlPointPtr(id);
	if (!cp) throw std::runtime_error("ControlPoint ID not found");
	return *cp;
}

ConstraintPoint& FeatureCurve::GetConstraintPoint(int id) {
	ConstraintPoint* cp = GetConstraintPointPtr(id);
	if (!cp) throw std::runtime_error("ConstraintPoint ID not found");
	return *cp;
}

int FeatureCurve::FindConstraintPointByU(float u) const {

	auto it = std::find_if(ConstraintPoints.begin(), ConstraintPoints.end(), [u](const ConstraintPoint& cp) { return glm::abs(cp.Data.u - u) < 1e-6f; }); // 또는 a == b

	return (it == ConstraintPoints.end()) ? -1 : it->Data.u;

}

void FeatureCurveManager::PrintState() const {
	
	switch (State) {
	case EditCurveState::P0:
		std::cout << "State: P0" << std::endl;
		break;
	case EditCurveState::P1:
		std::cout << "State: P1" << std::endl;
		break;
	case EditCurveState::P2:
		std::cout << "State: P2" << std::endl;
		break;
	case EditCurveState::P3:
		std::cout << "State: P3" << std::endl;
		break;
	case EditCurveState::CurveSelected:
		std::cout << "State: CurveSelected" << std::endl;
		break;
	case EditCurveState::ControlPointSelected:
		std::cout << "State: ControlPointSelected" << std::endl;
		break;
	case EditCurveState::ConstraintPointSelected:
		std::cout << "State: ConstraintPointSelected" << std::endl;
		break;
	case EditCurveState::None:
		std::cout << "State: None" << std::endl;
		break;
	}
}

void FeatureCurveManager::PrintDecision(const Decision& Decision) const {

	switch (Decision) {
	case Decision::SelectCurve:
		std::cout << "Decision: SelectCurve" << std::endl;
		break;
	case Decision::SelectControlPoint:
		std::cout << "Decision: SelectControlPoint" << std::endl;
		break;
	case Decision::Deselect:
		std::cout << "Decision: Deselect" << std::endl;
		break;
	case Decision::AddCurve:
		std::cout << "Decision: AddCurve" << std::endl;
		break;
	case Decision::ExtendCurve:
		std::cout << "Decision: ExtendCurve" << std::endl;
		break;
	case Decision::AddControlPoint:
		std::cout << "Decision: AddControlPoint" << std::endl;
		break;
	case Decision::CommitSegment:
		std::cout << "Decision: CommitSegment" << std::endl;
		break;
	case Decision::Cancel:
		std::cout << "Decision: Cancel" << std::endl;
		break;
	case Decision::DeleteSelectedControlPoint:
		std::cout << "Decision: DeleteSelectedControlPoint" << std::endl;
		break;
	case Decision::DeleteSelectedCurve:
		std::cout << "Decision: DeleteSelectedCurve" << std::endl;
		break;
	case Decision::AddConstraintPoint:
		std::cout << "Decision: AddConstraintPoint" << std::endl;
		break;
	case Decision::SelectConstraintPoint:
		std::cout << "Decision: SelectConstraintPoint" << std::endl;
		break;
	case Decision::None:
		std::cout << "Decision: None" << std::endl;
		break;
	default:
		break;
	}


}

int FeatureCurveManager::GetRemainder(FeatureCurve* Curve) const {
	const int n = static_cast<int>(Curve->GetControlPoints().size());
	return (n == 0) ? 0 : (n - 1) % 3;
}

// 반환 후 컨테이너를 수정하지 말 것
FeatureCurve* FeatureCurveManager::GetFeatureCurve(int id) {

	auto it = std::find_if(FeatureCurves.begin(), FeatureCurves.end(), [id](const FeatureCurve& c) {return c.GetCurveID() == id; });
	

	return (it == FeatureCurves.end()) ? nullptr : &(*it);

}

void FeatureCurveManager::Click(const glm::vec3& Pos, InputButton Button, InputMode Mode) {

	std::cout << "Previous State: ";
	PrintState();
	std::cout << std::endl;

	PickResult Picked = Pick(Pos);

	PrintPickResult(Picked);

	/*if (Picked.Type == PickType::ControlPoint) std::cout << "Pick ControlPoint" << std::endl;
	else if (Picked.Type == PickType::Curve) std::cout << "Pick Curve" << std::endl;
	else std::cout << "Pick None" << std::endl;*/

	Decision Dec = Decide(Button, Mode, State, Picked);

	std::cout << "Decided Result: ";
	PrintDecision(Dec);
	std::cout << std::endl;

	Execute(Dec, Picked, Pos);

	std::cout << "Next State: ";
	PrintState();
	std::cout << std::endl;
	

}

void FeatureCurveManager::UploadBuffers(BufferManager& BufferMgr) {
	for (auto& curve : FeatureCurves) {
		curve.UploadBuffer(BufferMgr);
		curve.UploadBufferLine(BufferMgr);
		curve.UploadBufferConstraintPoint(BufferMgr);
	}

}

const glm::vec3 FeatureCurveManager::AppliedTangentPos(const glm::vec3 P0, const glm::vec3& Pos, int Tangent) const {
	
	glm::vec3 v = glm::vec3(Pos.x - P0.x, 0.0f, Pos.z - P0.z);	

	float Length = glm::length(v);

	if (Length < 1e-6f) return P0;
	
	float Rad = glm::radians((float)Tangent);
	Rad = glm::clamp(Rad, glm::radians(-80.0f), glm::radians(80.0f));

	float yPos = Length * tanf(Rad);

	glm::vec3 P1 = P0 + v;
	P1.y = P0.y + yPos;

	return P1;

}

void FeatureCurveManager::PendControlPoint(const glm::vec3& Pos) {

	if (SelectedCurveID == -1) {
		std::cout << "PendControlPoint Error: No curve selected." << std::endl;
		return;
	}

	Pended.emplace(GetFeatureCurve(SelectedCurveID)->CreateControlPointID(), Pos);
	Pended->SetMesh();

}

void FeatureCurveManager::UploadPendedBuffer(BufferManager& BufferMgr) {

	if (!Pended.has_value()) return;

	ControlPointVisualMesh* mesh = Pended->GetMesh();
	if (mesh) {
		mesh->SetMeshID(BufferMgr.CreateMeshID());
		BufferMgr.CreateBufferData(mesh->GetMeshID(), true);
		BufferMgr.BindVertexBufferObjectByID(mesh->GetMeshID(), mesh->GetPosition().data(), mesh->GetPosition().size(),
			mesh->GetColor().data(), mesh->GetColor().size(), mesh->GetNormal().data(), mesh->GetNormal().size());
		BufferMgr.BindElementBufferObjectByID(mesh->GetMeshID(), mesh->GetIndex().data(), mesh->GetIndex().size());
	}


}

void FeatureCurveManager::RightClick() {

	
}

PickResult FeatureCurveManager::Pick(const glm::vec3& Pos) {

	PickResult Result;

	if (State == EditCurveState::ControlPointSelected) {
		Result = PickConstraintPoint(Pos);
		if (Result.Type != PickType::None) return Result;
	}

	Result = PickControlPoint(Pos);
	if (Result.Type != PickType::None) return Result;

	Result = PickConstraintPoint(Pos);
	if (Result.Type != PickType::None) return Result;

	Result = PickCurve(Pos);

	return Result;

}

void FeatureCurveManager::PrintPickResult(const PickResult& Result) const {

	switch (Result.Type) {
	case PickType::None:
		std::cout << "PickResult: None" << std::endl;
		break;
	case PickType::Curve:
		std::cout << "PickResult: Curve, CurveID = " << Result.CurveID << std::endl;
		break;
	case PickType::ControlPoint:
		std::cout << "PickResult: ControlPoint = " << Result.CurveID << ", ControlPointID = " << Result.ControlPointID << std::endl;
		break;
	case PickType::ConstraintPoint:
		std::cout << "PickResult: ConstraintPoint, CurveID = " << Result.CurveID << ", ConstraintPointID = " << Result.ConstraintPointID << std::endl;
		break;
	default:
		break;
	}

}

PickResult FeatureCurveManager::PickControlPoint(const glm::vec3& Pos) {

	PickResult Result;

	for (const auto& Curve : FeatureCurves) {
		const auto& CPs = Curve.GetControlPoints();
		for (int i = 0; i < CPs.size(); i++) {
			glm::vec3 PosCP = CPs[i].GetPosition();
			if (Pos.x >= PosCP.x - CPs[i].GetHalf() && Pos.x <= PosCP.x + CPs[i].GetHalf() && Pos.z >= PosCP.z - CPs[i].GetHalf() && Pos.z <= PosCP.z + CPs[i].GetHalf()) {
				Result.Type = PickType::ControlPoint;
				Result.CurveID = Curve.GetCurveID();
				Result.ControlPointID = CPs[i].GetID();
				return Result;
			}
		}
	}
	return Result;
}

PickResult FeatureCurveManager::PickConstraintPoint(const glm::vec3& Pos) {
	PickResult Result;
	for (const auto& Curve : FeatureCurves) {
		const auto& CPs = Curve.GetConstraintPoints();
		for (int i = 0; i < CPs.size(); i++) {
			glm::vec3 PosCP = CPs[i].GetPosition();
			if (Pos.x >= PosCP.x - CPs[i].GetHalf() && Pos.x <= PosCP.x + CPs[i].GetHalf() && Pos.z >= PosCP.z - CPs[i].GetHalf() && Pos.z <= PosCP.z + CPs[i].GetHalf()) {
				Result.Type = PickType::ConstraintPoint;
				Result.CurveID = Curve.GetCurveID();
				Result.ConstraintPointID = CPs[i].GetID();
				return Result;
			}
		}
	}
	return Result;
}

PickResult FeatureCurveManager::PickCurve(const glm::vec3& Pos) {

	PickResult Result;

	int NearestCurveID = -1;
	float NearestDistance = std::numeric_limits<float>::max();

	for (auto& Curve : FeatureCurves) {
		if (!Curve.PointInBoundingBox(Pos)) continue;

		float Distance = Curve.NearestDistanceSq(Pos);

		if (Distance < NearestDistance) {
			NearestDistance = Distance;
			NearestCurveID = Curve.GetCurveID();
		}

	}
	if ((NearestDistance <= r * r) && (NearestCurveID != -1)) {

		Result.Type = PickType::Curve;
		Result.CurveID = NearestCurveID;
		return Result;
	}

	return Result;

}

int FeatureCurveManager::FindNearestCurvePointInSelecting(const glm::vec3& Pos) {

	if (SelectedCurveID == -1) return -1;
	if (State != EditCurveState::CurveSelected) return -1;

	FeatureCurve* Curve = GetFeatureCurve(SelectedCurveID);
	int Index = Curve->FindNearestCurvePoint(Pos);

	return Index;

}

Decision FeatureCurveManager::Decide(InputButton Button, InputMode Mode, EditCurveState State, const PickResult& Picked) {

	const bool NonePicked = (Picked.Type == PickType::None);
	const bool CurvePicked = (Picked.Type == PickType::Curve);
	const bool ControlPointPicked = (Picked.Type == PickType::ControlPoint);
	const bool ConstraintPointPicked = (Picked.Type == PickType::ConstraintPoint);

	//std::cout << "Curve Pick(" << CurvePicked << ")" << std::endl;

	switch (State) {
	case EditCurveState::P0:
	case EditCurveState::P1:
	case EditCurveState::P2:
	case EditCurveState::P3:
		if (Button == InputButton::Left) {
			if (Mode != InputMode::Ctrl) return Decision::Cancel;
			return Decision::AddControlPoint;
		}
		if (Button == InputButton::Right) return Decision::Cancel;
		return Decision::None;
		break;

	case EditCurveState::CurveSelected:
		if (Button == InputButton::Right) return Decision::Deselect;
		if (Button != InputButton::Left) return Decision::None;

		if (Mode == InputMode::Ctrl) return Decision::ExtendCurve;

		if (Mode == InputMode::Shift) {
			std::cout << "Add Constraint Point" << std::endl;
			return Decision::AddConstraintPoint;
		}
		if (ControlPointPicked) return Decision::SelectControlPoint;
		if (ConstraintPointPicked) return Decision::SelectConstraintPoint;
		if (CurvePicked) return Decision::SelectCurve;
		if (NonePicked) return Decision::Deselect;

		return Decision::None;

		break;

	case EditCurveState::ControlPointSelected:
		if (Button != InputButton::Left) return Decision::None;

		if (Mode == InputMode::Ctrl) return Decision::None;
		if (Mode == InputMode::Shift) return Decision::None;

		if (ControlPointPicked) return Decision::SelectControlPoint;
		if (ConstraintPointPicked) return Decision::SelectConstraintPoint;
		if (CurvePicked) return Decision::SelectCurve;
		if (NonePicked) return Decision::Deselect;

		return Decision::None;

		break;

	case EditCurveState::ConstraintPointSelected:
		if (Button != InputButton::Left) return Decision::None;

		if (Mode == InputMode::Ctrl) return Decision::None;
		if (Mode == InputMode::Shift) return Decision::None;

		if (ControlPointPicked) return Decision::SelectControlPoint;
		if (ConstraintPointPicked) return Decision::SelectConstraintPoint;
		if (CurvePicked) return Decision::SelectCurve;
		if (NonePicked) return Decision::Deselect;

		return Decision::None;
		break;

	case EditCurveState::None:

		if (Button != InputButton::Left) return Decision::None;

		if (Mode == InputMode::Ctrl) {
			if (NonePicked) return Decision::AddCurve;
			if (CurvePicked) return Decision::ExtendCurve;
			if (ControlPointPicked) return Decision::SelectControlPoint; // 또는 ExtendCurve로 바꿔도 됨
			if (ConstraintPointPicked) return Decision::SelectConstraintPoint;
			return Decision::None;
		}

		if (Mode == InputMode::Default) {
			if (ControlPointPicked) return Decision::SelectControlPoint;
			if (ConstraintPointPicked) return Decision::SelectConstraintPoint;
			if (CurvePicked) return Decision::SelectCurve;
			return Decision::None;
		}

		return Decision::None;

		break;
	default:
		break;
	}


	return Decision::None;

}

void FeatureCurveManager::Execute(Decision DecidedResult, const PickResult& Picked, const glm::vec3& Pos) {
	
	switch (DecidedResult)
	{
	case Decision::SelectCurve:
		SelectCurve(Picked);
		break;
	case Decision::AddCurve:
		AddFeatureCurve();
		AddControlPoint(Pos);
		break;
	case Decision::ExtendCurve:
		ExtendCurve(Picked);
		AddControlPoint(Pos);
		break;
	case Decision::AddControlPoint:
		AddControlPoint(Pos);
		break;

	case Decision::Cancel:
		Cancel();
		break;
	case Decision::Deselect:
		//Cancel();
		DeselectAll();
		break;
	case Decision::AddConstraintPoint:
		AddConstraintPoint(Pos);
		break;
	case Decision::DeleteSelectedControlPoint:
	case Decision::DeleteSelectedCurve:
	case Decision::SelectControlPoint:
		SelectControlPoint(Picked);
		break;
	case Decision::SelectConstraintPoint:
		SelectConstraintPoint(Picked);
		break;
	case Decision::CommitSegment:
	case Decision::None:
		break;

	default:
		break;
	}

}

void FeatureCurveManager::SelectCurve(const PickResult& Picked) {

	DeselectAll();

	SelectedCurveID = Picked.CurveID;
	GetFeatureCurve(SelectedCurveID)->SetHighlightWeight(1.0f);
	State = EditCurveState::CurveSelected;

	return;

}

void FeatureCurveManager::SelectControlPoint(const PickResult& Picked) {

	DeselectAll();

	SelectedCurveID = Picked.CurveID;

	SelectedConstraintPointID = -1;

	SelectedControlPointID = Picked.ControlPointID;

	GetFeatureCurve(SelectedCurveID)->GetControlPoint(SelectedControlPointID).SetHighlightWeight(1.0f);
	
	State = EditCurveState::ControlPointSelected;

	return;
}

void FeatureCurveManager::SelectConstraintPoint(const PickResult& Picked) {

	DeselectAll();

	SelectedCurveID = Picked.CurveID;
	SelectedControlPointID = -1;
	SelectedConstraintPointID = Picked.ConstraintPointID;

	GetFeatureCurve(SelectedCurveID)->GetConstraintPoint(SelectedConstraintPointID).SetHighlightWeight(1.0f);

	State = EditCurveState::ConstraintPointSelected;

	View.ConstraintPointPanelOpen = true;

	return;
}

void FeatureCurveManager::AddFeatureCurve() {

	int NewId = CreateCurveID();

	FeatureCurves.emplace_back(FeatureCurve(NewId));
	SelectedCurveID = NewId;
	State = EditCurveState::CurveSelected;


}

void FeatureCurveManager::ExtendCurve(const PickResult& Picked) {

	State = EditCurveState::P1;
}

void FeatureCurveManager::AddControlPoint(const glm::vec3& Pos) {

	if (SelectedCurveID == -1) return;

	FeatureCurve* Curve = GetFeatureCurve(SelectedCurveID);

	if (!Curve) return;

	switch (State) {
	case EditCurveState::CurveSelected: {
		Curve->AddControlPoint(Pos);
		std::cout << "P0 Pos: " << Pos.x << ", " << Pos.y << ", " << Pos.z << std::endl;
		State = EditCurveState::P1;
		break;
	}
	case EditCurveState::P1: {
		Curve->AddControlPoint(Pos);
		std::cout << "P1 Pos: " << Pos.x << ", " << Pos.y << ", " << Pos.z << std::endl;
		State = EditCurveState::P3;
		break;
	}
	case EditCurveState::P3: {
		PendControlPoint(Pos);
		std::cout << "P3 Pos: " << Pos.x << ", " << Pos.y << ", " << Pos.z << std::endl;
		State = EditCurveState::P2;
		break;
	}
	case EditCurveState::P2: {
		Curve->AddControlPoint(Pos);
		std::cout << "P2 Pos: " << Pos.x << ", " << Pos.y << ", " << Pos.z << std::endl;
		Curve->AddControlPoint(std::move(*Pended));

		Pended.reset();
		Curve->BuildLinesLength();
		UpdateAllBoundingBoxes();

		if (CommittedSegments < 1) {
			Curve->AddConstraintPoint(Curve->GetControlPoints().front().GetPosition(), 0.0f);
			Curve->AddConstraintPoint(Pended->GetPosition(), 1.0f);
		}
		else {
			UpdateLastConstraint(Curve->GetCurveID());
		}

		CommittedSegments++;



		std::cout << Curve->GetCurveID() << std::endl;
		State = EditCurveState::P1;
		break;

	}
	default:
		break;
	}

}

void FeatureCurveManager::DeselectCurve() {

	if (SelectedCurveID == -1) return;

	Pended.reset();
	GetFeatureCurve(SelectedCurveID)->SetHighlightWeight(0.0f);
	SelectedCurveID = -1;
	State = EditCurveState::None;
}

void FeatureCurveManager::DeselectControlPoint() {

	if (SelectedCurveID == -1 || SelectedControlPointID == -1) return;

	GetFeatureCurve(SelectedCurveID)->GetControlPoint(SelectedControlPointID).SetHighlightWeight(0.0f);
	SelectedControlPointID = -1;
	State = EditCurveState::None;

}

void FeatureCurveManager::DeselectConstraintPoint() {

	if (SelectedCurveID == -1 || SelectedConstraintPointID == -1) return;

	GetFeatureCurve(SelectedCurveID)->GetConstraintPoint(SelectedConstraintPointID).SetHighlightWeight(0.0f);
	SelectedConstraintPointID = -1;
	State = EditCurveState::None;

	View.ConstraintPointPanelOpen = false;

}

void FeatureCurveManager::DeselectAll() {
	if (SelectedCurveID == -1) return;

	FeatureCurve* Curve = GetFeatureCurve(SelectedCurveID);
	if (!Curve) {
		std::cout << "DeselectAll(): Curve not found!" << std::endl;
		return;
	}
	FC::ControlPoint* ControlPoint = Curve->GetControlPointPtr(SelectedControlPointID);
	ConstraintPoint* ConstraintPoint = Curve->GetConstraintPointPtr(SelectedConstraintPointID);

	if (Curve) Curve->SetHighlightWeight(0.0f);
	if (ControlPoint) ControlPoint->SetHighlightWeight(0.0f);
	if (ConstraintPoint) ConstraintPoint->SetHighlightWeight(0.0f);

	SelectedCurveID = -1;
	SelectedControlPointID = -1;
	SelectedConstraintPointID = -1;

	State = EditCurveState::None;

	View.ConstraintPointPanelOpen = false;
}

void FeatureCurveManager::Cancel() {

	if (SelectedCurveID == -1) return;

	int Count = GetFeatureCurve(SelectedCurveID)->GetControlPoints().size();

	switch (State) {
	case EditCurveState::P0:
		break;
	case EditCurveState::P1:
		if (CommittedSegments > 0) {
			Pended.reset();
			State = EditCurveState::CurveSelected;
		}
		else {
			FeatureCurves.pop_back();
			Pended.reset();
			State = EditCurveState::None;
		}
		break;

	case EditCurveState::P3:
		if (CommittedSegments > 0) {
			GetFeatureCurve(SelectedCurveID)->PopBack();
			Pended.reset();
			State = EditCurveState::CurveSelected;
		}
		else {
			FeatureCurves.pop_back();
			Pended.reset();
			State = EditCurveState::None;
		}
		break;

	case EditCurveState::P2:
		if (CommittedSegments > 0) {
			GetFeatureCurve(SelectedCurveID)->PopBack();
			Pended.reset();
			State = EditCurveState::CurveSelected;
		}
		else {
			FeatureCurves.pop_back();
			Pended.reset();
			State = EditCurveState::None;
		}
		break;

	}

}

void FeatureCurveManager::HoverPressedCtrl(const glm::vec3& Pos) {

	HoveringControlPoint.GetMesh()->Translate(Pos);
	
}

void FeatureCurveManager::HoverPressedShift(const glm::vec3& Pos) {

	if (SelectedCurveID == -1) return;
	if (State != EditCurveState::CurveSelected) return;

	FeatureCurve* Curve = GetFeatureCurve(SelectedCurveID);
	
	int Index = Curve->FindNearestCurvePoint(Pos);

	glm::vec3 NewPos = Curve->GetSamplePoints()[Index].Position;

	HoveringConstraintPoint.GetMesh()->Translate(NewPos);


}

void FeatureCurveManager::AddConstraintPoint(const glm::vec3& Pos) {
	if (SelectedCurveID == -1) {
		std::cout << "AddConstraintPoint(): No curve selected!" << std::endl;
		return;
	}
	if ((State != EditCurveState::CurveSelected) && (State != EditCurveState::P2)) {
		std::cout << "AddConstraintPoint(): Invalid state!" << std::endl;
		return;
	}
	FeatureCurve* Curve = GetFeatureCurve(SelectedCurveID);

	int Index = Curve->FindNearestCurvePoint(Pos);

	glm::vec3 NewPos = Curve->GetSamplePoints()[Index].Position;
	float u = Curve->GetSamplePoints()[Index].u;

	Curve->AddConstraintPoint(NewPos, u);


}

void FeatureCurveManager::UpdateConstraintPointPosByU(int CurveID, int ConstraintPointID){

	FeatureCurve* Curve = GetFeatureCurve(CurveID);

	if (!Curve) return;

	//glm::vec3 NewPos = Curve->GetSamplePoints().back().Position; // 이거 아님, u에 따라 바꿔야함
	
	//Curve->GetConstraintPoint(ConstraintPointID).CachedPos = NewPos;
	//Curve->GetConstraintPoint(ConstraintPointID).Cached = true;
	//Curve->GetConstraintPoint(ConstraintPointID).Uploaded = true;
	
	//Curve->GetConstraintPoint(ConstraintPointID).SetMesh();


}

void FeatureCurveManager::UpdateLastConstraint(int CurveID) {

	FeatureCurve* Curve = GetFeatureCurve(CurveID);
	if (!Curve) return;

	int id = Curve->FindConstraintPointByU(1.0f);
	if (id == -1) return;

	glm::vec3 NewPos = Curve->GetSamplePoints().back().Position;

	std::cout << NewPos.x << ", " << NewPos.y << ", " << NewPos.z << std::endl;

	Curve->GetConstraintPoint(id).CachedPos = NewPos;
	Curve->GetConstraintPoint(id).Cached = true;
	Curve->GetConstraintPoint(id).Uploaded = true;
	Curve->GetConstraintPoint(id).SetMesh();

}

void FeatureCurveManager::UpdateConstraintPoints(int CurveID) {

	FeatureCurve* Curve = GetFeatureCurve(CurveID);
	if (!Curve) return;
	
	// Todo
	/*SamplePoint or ConstraintPoint가 바뀌는 순간
	= ControlPoint가 이동하거나 Segment가 추가되는 순간에
	SamplePoint와 ConstraintPoint의 Pos, u값을 업데이트해야함*/



	// u에 따라 위치 업데이트
}

const std::vector<CurveData> FeatureCurveManager::ExtractCurveData() {

	std::vector<CurveData> Datas;

	for (const auto& curve : FeatureCurves) {
		CurveData Data;
		for (const auto& control : curve.GetControlPoints()) {
			Data.ControlPoints.emplace_back(control.GetPosition());
		}
		for(const auto& constraint : curve.GetConstraintPoints()) {
			Data.ConstraintPoints.emplace_back(constraint.Data);
		}
		std::sort(Data.ConstraintPoints.begin(), Data.ConstraintPoints.end(), [](const Constraints& a, const Constraints& b) {
			return a.u < b.u;
			});
		Datas.emplace_back(std::move(Data));
	}

	return Datas;

}