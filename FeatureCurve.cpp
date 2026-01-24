#include "FeatureCurve.h"

void FC::ControlPoint::SetMesh() {

	Mesh->SetHexahedron(Position, half, glm::vec3(1.0f, 0.0f, 1.0f));

}
	
void FeatureCurve::AddControlPoint(const glm::vec3& pos) {

	FC::ControlPoint cp(pos);

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

void FeatureCurve::AddConstraintPoint(const glm::vec3 Pos) {

	ConstraintPoint cp(Pos);

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
	
	Pended.emplace(Pos);
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

	PickResult Result = PickControlPoint(Pos);
	if (Result.Type != PickType::None) return Result;

	Result = PickCurve(Pos);
	return Result;


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
				Result.ControlPointIndex = i;
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

	if (SelectedID == -1) return -1;
	if (State != EditCurveState::CurveSelected) return -1;

	FeatureCurve* Curve = GetFeatureCurve(SelectedID);
	int Index = Curve->FindNearestCurvePoint(Pos);

	return Index;

}

Decision FeatureCurveManager::Decide(InputButton Button, InputMode Mode, EditCurveState State, const PickResult& Picked) {

	const bool NonePicked = (Picked.Type == PickType::None);
	const bool CurvePicked = (Picked.Type == PickType::Curve);
	const bool ControlPointPicked = (Picked.Type == PickType::ControlPoint);

	std::cout << "Curve Pick(" << CurvePicked << ")" << std::endl;

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

		if (ControlPointPicked) return Decision::SelectControlPoint;
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
			return Decision::None;
		}

		if (Mode == InputMode::Default) {
			if (ControlPointPicked) return Decision::SelectControlPoint;
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
		DeselectCurve();
		break;
	case Decision::DeleteSelectedControlPoint:
	case Decision::DeleteSelectedCurve:
	case Decision::SelectControlPoint:
	case Decision::CommitSegment:
	case Decision::None:
		break;

	default:
		break;
	}

}

void FeatureCurveManager::SelectCurve(const PickResult& Picked) {

	SelectedID = Picked.CurveID;
	GetFeatureCurve(SelectedID)->SetHighlightWeight(1.0f);
	State = EditCurveState::CurveSelected;

	return;

}

void FeatureCurveManager::AddFeatureCurve() {

	int NewId = CreateCurveID();

	FeatureCurves.emplace_back(FeatureCurve(NewId));
	SelectedID = NewId;
	State = EditCurveState::CurveSelected;


}

void FeatureCurveManager::ExtendCurve(const PickResult& Picked) {

	State = EditCurveState::P1;
}

void FeatureCurveManager::AddControlPoint(const glm::vec3& Pos) {

	if (SelectedID == -1) return;

	FeatureCurve* Curve = GetFeatureCurve(SelectedID);

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
		CommittedSegments++;
		Pended.reset();
		Curve->BuildLines();
		UpdateAllBoundingBoxes();
		std::cout << Curve->GetCurveID() << std::endl;
		State = EditCurveState::P1;
		break;

	}
	default:
		break;
	}

}

void FeatureCurveManager::DeselectCurve() {
	Pended.reset();
	GetFeatureCurve(SelectedID)->SetHighlightWeight(0.0f);
	SelectedID = -1;
	State = EditCurveState::None;
}

void FeatureCurveManager::Cancel() {

	if (SelectedID == -1) return;

	int Count = GetFeatureCurve(SelectedID)->GetControlPoints().size();

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
			State = EditCurveState::CurveSelected;
		}
		break;

	case EditCurveState::P3:
		if (CommittedSegments > 0) {
			GetFeatureCurve(SelectedID)->PopBack();
			Pended.reset();
			State = EditCurveState::CurveSelected;
		}
		else {
			FeatureCurves.pop_back();
			Pended.reset();
			State = EditCurveState::CurveSelected;
		}
		break;

	case EditCurveState::P2:
		if (CommittedSegments > 0) {
			GetFeatureCurve(SelectedID)->PopBack();
			Pended.reset();
			State = EditCurveState::CurveSelected;
		}
		else {
			FeatureCurves.pop_back();
			Pended.reset();
			State = EditCurveState::CurveSelected;
		}
		break;

	}

}

void FeatureCurveManager::HoverPressedCtrl(const glm::vec3& Pos) {

	HoveringControlPoint.GetMesh()->Translate(Pos);
	
}

void FeatureCurveManager::HoverPressedShift(const glm::vec3& Pos) {

	if (SelectedID == -1) return;
	if (State != EditCurveState::CurveSelected) return;

	FeatureCurve* Curve = GetFeatureCurve(SelectedID);
	
	int Index = Curve->FindNearestCurvePoint(Pos);

	glm::vec3 NewPos = Curve->GetSamplePoints()[Index].Position;

	HoveringConstraintPoint.GetMesh()->Translate(NewPos);
}

void FeatureCurveManager::AddConstraintPoint(const glm::vec3& Pos) {
	if (SelectedID == -1) return;
	if (State != EditCurveState::CurveSelected) return;

	FeatureCurve* Curve = GetFeatureCurve(SelectedID);

	int Index = Curve->FindNearestCurvePoint(Pos);

	glm::vec3 NewPos = Curve->GetSamplePoints()[Index].Position;

	Curve->AddConstraintPoint(NewPos);


}