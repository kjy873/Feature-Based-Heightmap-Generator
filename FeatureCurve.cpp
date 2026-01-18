#include "FeatureCurve.h"

void FC::ControlPoint::SetMesh() {

	Mesh->SetHexahedron(Position, 0.005f, glm::vec3(1.0f, 0.0f, 1.0f));

}
	
void FeatureCurve::AddControlPoint(const glm::vec3& pos) {
	
	const int Index = ControlPoints.size();

	FC::ControlPoint cp(pos, Index);

	cp.SetMesh();

	ControlPoints.push_back(std::move(cp));

}

void FeatureCurve::UploadBuffer(BufferManager& BufferMgr) {
	for (const auto& cp : ControlPoints) {
		if (cp.GetIndex() == -1) continue;
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

	if (FeatureCurves.empty() || SelectedID == -1) AddFeatureCurve();

	FeatureCurve* Curve = GetFeatureCurve(SelectedID);

	if (!Curve) return;

	int Remainder = GetRemainder(Curve);

	int CPcount = Curve->GetControlPoints().size();

	if (CPcount == 0) {
		Curve->AddControlPoint(Pos);
	}
	else {
		switch (Remainder) {
		case 0:
			glm::vec3 NewPos = AppliedTangentPos(Curve->GetControlPoints().back().GetPosition(), Pos, Tangent);
			std::cout << "NewPos: " << NewPos.x << ", " << NewPos.y << ", " << NewPos.z << std::endl;
			Curve->AddControlPoint(NewPos);
			break;
		case 1:
			Curve->AddControlPoint(Pos);
			break;
		case 2:
			Curve->AddControlPoint(Pos);
			break;

		default:
			break;
		}
	}


}

void FeatureCurveManager::RightClick() {

	if (SelectedID < 0) return;
	


}

void FeatureCurveManager::UploadBuffers(BufferManager& BufferMgr) {
	for (auto& curve : FeatureCurves) {
		curve.UploadBuffer(BufferMgr);
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