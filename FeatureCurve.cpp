#include "FeatureCurve.h"

FC::ControlPoint::ControlPoint(glm::vec3 pos, int Index) : Position(pos), Index(Index) {
	Mesh = new ControlPointVisualMesh(8);
}

void FC::ControlPoint::SetMesh() {

	Mesh->SetHexahedron(Position, 0.005f, glm::vec3(1.0f, 0.0f, 1.0f));

}

void FeatureCurve::AddControlPoint(const glm::vec3& pos, int Index) {
	
	FC::ControlPoint cp(pos, Index);

	cp.SetMesh();

	ControlPoints[cp.GetIndex()] = cp;

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