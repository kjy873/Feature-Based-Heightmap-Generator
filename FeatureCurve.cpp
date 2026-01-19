#include "FeatureCurve.h"

void FC::ControlPoint::SetMesh() {

	Mesh->SetHexahedron(Position, 0.005f, glm::vec3(1.0f, 0.0f, 1.0f));

}
	
void FeatureCurve::AddControlPoint(const glm::vec3& pos) {
	
	const int Index = ControlPoints.size();

	FC::ControlPoint cp(pos);

	cp.SetMesh();

	ControlPoints.push_back(std::move(cp));

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

	Vertices.clear();

	int Count = ControlPoints.size();
	if (Count < 4 || (Count - 1) % 3 != 0) return;

	for (int i = 0; i + 3 < Count; i += 3) {
		const glm::vec3& P0 = ControlPoints[i].GetPosition();
		const glm::vec3& P1 = ControlPoints[i + 1].GetPosition();
		const glm::vec3& P2 = ControlPoints[i + 2].GetPosition();
		const glm::vec3& P3 = ControlPoints[i + 3].GetPosition();

		int SamplingStart = (i == 0) ? 0 : 1;

		for (int j = SamplingStart; j <= SamplePerSegment; j++) {
			float t = (float)j / (float)SamplePerSegment;

			Vertices.push_back(BezierCubic(P0, P1, P2, P3, t));
		}
	}

	LineDirty = true;
}

void FeatureCurve::UploadBufferLine(BufferManager& BufferMgr) {
	if (Vertices.empty()) return;
	if (!LineDirty) return;

	if (!Line) {
		Line = std::make_unique<LineMesh>(Vertices.size());
		Line->SetMeshID(BufferMgr.CreateMeshID());
		BufferMgr.CreateBufferData(Line->GetMeshID(), false);
	}

	Line->SetLines(Vertices, glm::vec3(1.0f, 1.0f, 1.0f));

	BufferMgr.BindVertexBufferObjectByID(Line->GetMeshID(), Line->GetPosition().data(), Line->GetPosition().size(),
		Line->GetColor().data(), Line->GetColor().size(),
		nullptr, 0);

	LineDirty = false;

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

float FeatureCurve::DistancePointToLine(const glm::vec3 Point, const glm::vec3 LineStart, const glm::vec3 LineEnd) const {

	glm::vec3 AB = LineEnd - LineStart;

	glm::vec3 AP = Point - LineStart;

	float t = glm::dot(AP, AB) / glm::dot(AB, AB);

	t = glm::clamp(t, 0.0f, 1.0f);

	glm::vec3 ClosestPoint = LineStart + t * AB;

	return glm::dot(Point - ClosestPoint, Point - ClosestPoint);
	
}

float FeatureCurve::NearestDistance(const glm::vec3 Point) {

	if(Vertices.size() < 2) return std::numeric_limits<float>::infinity();

	float Nearest = std::numeric_limits<float>::max();

	for (int i = 0; i < Vertices.size() - 1; i++) {
		float Distance = DistancePointToLine(Point, Vertices[i], Vertices[i + 1]);
		if (Distance < Nearest) {
			Nearest = Distance;
		}
	}

	return Nearest;

}

void FeatureCurveManager::AddFeatureCurve() {

	int NewId = CreateCurveID();
	
	FeatureCurves.emplace_back(FeatureCurve(NewId));
	SelectedID = NewId;


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

void FeatureCurveManager::LeftClick(const glm::vec3& Pos, int Tangent) {

	if (SelectedID == -1) {

		int NearestCurveID = -1;
		float NearestDistance = std::numeric_limits<float>::max();

		for (auto& Curve : FeatureCurves) {
			if (!Curve.PointInBoundingBox(Pos)) continue;

			float Distance = Curve.NearestDistance(Pos);

			if (Distance < NearestDistance) {
				NearestDistance = Distance;
				NearestCurveID = Curve.GetCurveID();
			}
				
		}
		if ((NearestDistance <= 0.05 * 0.05f) && (NearestCurveID != -1)) {
			SelectedID = NearestCurveID;
			State = EditCurveState::Selected;
			std::cout << "Selected Curve ID: " << SelectedID << std::endl;
			return;
		}
	}

	if (FeatureCurves.empty() || SelectedID == -1) AddFeatureCurve();

	FeatureCurve* Curve = GetFeatureCurve(SelectedID);

	if (!Curve) return;

	int Remainder = GetRemainder(Curve);

	switch (State) {
	case EditCurveState::P0: {
		Curve->AddControlPoint(Pos);
		std::cout << "P0 Pos: " << Pos.x << ", " << Pos.y << ", " << Pos.z << std::endl;
		State = EditCurveState::P1;
		break;
	}
	case EditCurveState::P1: {
		/*glm::vec3 NewPos = AppliedTangentPos(Curve->GetControlPoints().back().GetPosition(), Pos, Tangent);
		std::cout << "P1 Pos: " << NewPos.x << ", " << NewPos.y << ", " << NewPos.z << std::endl;*/
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
		/*glm::vec3 NewPos = AppliedTangentPos(Pended->GetPosition(), Pos, Tangent);
		std::cout << "P2 Pos: " << NewPos.x << ", " << NewPos.y << ", " << NewPos.z << std::endl;*/
		Curve->AddControlPoint(Pos);
		std::cout << "P2 Pos: " << Pos.x << ", " << Pos.y << ", " << Pos.z << std::endl;
		Curve->AddControlPoint(std::move(*Pended));
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

	if (SelectedID == -1) return;

	int Count = GetFeatureCurve(SelectedID)->GetControlPoints().size();


	switch (State) {
	case EditCurveState::P0:
		break;
	case EditCurveState::P1:
		if (Count >= 4) {
			Pended.reset();
			SelectedID = -1;
			State = EditCurveState::P0;
		}
		else {
			FeatureCurves.pop_back();
			Pended.reset();
			SelectedID = -1;
			State = EditCurveState::P0;
		}
		break;

	case EditCurveState::P3:
		if (Count >= 4) {
			GetFeatureCurve(SelectedID)->PopBack();
			Pended.reset();
			SelectedID = -1;
			State = EditCurveState::P0;
		}
		else {
			FeatureCurves.pop_back();
			Pended.reset();
			SelectedID = -1;
			State = EditCurveState::P0;
		}
		break;

	case EditCurveState::P2:
		if (Count >= 4) {
			GetFeatureCurve(SelectedID)->PopBack();
			Pended.reset();
			SelectedID = -1;
			State = EditCurveState::P0;
		}
		else {
			FeatureCurves.pop_back();
			Pended.reset();
			SelectedID = -1;
			State = EditCurveState::P0;
		}
		break;

	}
}

bool FeatureCurveManager::FindCurvesByPoint(const glm::vec3 Pos) {
	
}