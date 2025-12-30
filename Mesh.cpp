#include "Mesh.h"


void LineMesh::SetLine(const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3* c) {
	for (int i = 0; i < 2; i++) {
		Color[i] = glm::vec3(c[i]);
	}
	Position[0] = glm::vec3(vertex1);
	Position[1] = glm::vec3(vertex2);

	IsLine = true;
}

void LineMesh::SetLines(std::vector<glm::vec3> vertices, const glm::vec3& c) {

	Position.clear();
	Position = vertices;
	Color.clear();
	Color.resize(vertices.size());

	for (int i = 0; i < Color.size(); i++) {
		Color[i] = glm::vec3(c);
	}

	IsLine = true;

}

void RectangleMesh::SetRectangle(const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3 vertex3, const glm::vec3 vertex4, const glm::vec3* c) {
	Position[0] = glm::vec3(vertex1);
	Position[1] = glm::vec3(vertex2);
	Position[2] = glm::vec3(vertex3);
	Position[3] = glm::vec3(vertex4);

	glm::vec3 normal = glm::normalize(glm::cross(vertex2 - vertex1, vertex4 - vertex1));
	for (int i = 0; i < 4; i++) {
		Normal[i] = normal;
	}

	for (int i = 0; i < 4; i++) {
		Color[i] = glm::vec3(c[i]);
	}

}

//void HexahedronMesh::SetHexahedron(std::vector<Mesh>& dst, const glm::vec3 vertices[8], const glm::vec3* c) {
//
//	Mesh temp(4);
//
//	glm::vec3 FrontColor[4] = { c[0], c[1], c[2], c[3] };
//	glm::vec3 TopColor[4] = { c[4], c[5], c[1], c[0] };
//	glm::vec3 BackColor[4] = { c[6], c[7], c[5], c[4] };
//	glm::vec3 BottomColor[4] = { c[3], c[2], c[6], c[7] };
//	glm::vec3 LeftColor[4] = { c[4], c[0], c[3], c[7] };
//	glm::vec3 RightColor[4] = { c[1], c[5], c[6], c[2] };
//
//	temp.SetRectangle(vertices[0], vertices[1], vertices[2], vertices[3], FrontColor); // front
//	dst.push_back(temp);
//	temp.SetRectangle(vertices[4], vertices[5], vertices[1], vertices[0], TopColor); // top
//	dst.push_back(temp);
//	temp.SetRectangle(vertices[6], vertices[7], vertices[5], vertices[4], BackColor); // back
//	dst.push_back(temp);
//	temp.SetRectangle(vertices[3], vertices[2], vertices[6], vertices[7], BottomColor); // bottom
//	dst.push_back(temp);
//	temp.SetRectangle(vertices[4], vertices[0], vertices[3], vertices[7], LeftColor); // left
//	dst.push_back(temp);
//	temp.SetRectangle(vertices[1], vertices[5], vertices[6], vertices[2], RightColor); // right
//	dst.push_back(temp);
//
//}

void CubeMesh::SetHexahedron(const glm::vec3 center, float half, const glm::vec3& c) {

	glm::vec3 vertices[8] = {
	   center + glm::vec3(-half,  half,  half),
	   center + glm::vec3(half,  half,  half),
	   center + glm::vec3(half, -half,  half),
	   center + glm::vec3(-half, -half,  half),
	   center + glm::vec3(-half,  half, -half),
	   center + glm::vec3(half,  half, -half),
	   center + glm::vec3(half, -half, -half),
	   center + glm::vec3(-half, -half, -half)
	};

	for (int i = 0; i < 8; ++i) {
		Position[i] = vertices[i];
		Color[i] = c;
	}

	std::vector<glm::vec3> normalSum(8, glm::vec3(0.0f));
	std::vector<int> normalCount(8, 0);
	for (int i = 0; i < 36; i += 3) {
		int i0 = Index[i];
		int i1 = Index[i + 1];
		int i2 = Index[i + 2];

		glm::vec3& p0 = Position[i0];
		glm::vec3& p1 = Position[i1];
		glm::vec3& p2 = Position[i2];

		glm::vec3 faceNormal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

		normalSum[i0] += faceNormal;
		normalSum[i1] += faceNormal;
		normalSum[i2] += faceNormal;

		normalCount[i0]++;
		normalCount[i1]++;
		normalCount[i2]++;
	}

	for (int i = 0; i < 8; ++i) {
		if (normalCount[i] > 0)
			Normal[i] = glm::normalize(normalSum[i] / (float)normalCount[i]);
		else
			Normal[i] = glm::vec3(0.0f, 0.0f, 1.0f);
	}
}

void TerrainMesh::SetSurface(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices, const std::vector<glm::vec3>& normals, const glm::vec3& c) {
	Position = vertices;
	Index = indices;
	Normal = normals;
	Color.resize(vertices.size());

	for (int i = 0; i < vertices.size(); i++) {
		Color[i] = c;
	}
}

void TerrainMesh::SetSurfaceNormalized(const std::vector<float>& HeightMap, const int Rows, const int Cols, const glm::vec3& color) {

	if ((int)HeightMap.size() != Rows * Cols) {
		printf("HeightMap size mismatch\n");
		return;
	}


	Position.resize(Rows * Cols);

	for (int i = 0; i < Rows * Cols; i++) {
		int r = i / Cols;
		int c = i % Cols;

		float x = (Rows == 1) ? 0.0f : (float)r / (float)(Rows - 1);
		float y = HeightMap[i];
		float z = (Cols == 1) ? 0.0f : (float)c / (float)(Cols - 1);

		Position[i] = glm::vec3(x, y, z);

	}

	int SizeU2 = Rows;
	int SizeV2 = Cols;

	Index.clear();
	Index.reserve((Rows - 1) * (Cols - 1) * 6);
	for (int i = 0; i < SizeU2 - 1; i++) {
		for (int j = 0; j < SizeV2 - 1; j++) {
			// 사각형 정점 인덱스 중복 저장을 방지하기 위해 EBO 사용
			int index = i * SizeV2 + j;
			Index.push_back(index);
			Index.push_back(index + SizeV2);
			Index.push_back(index + SizeV2 + 1);
			Index.push_back(index + SizeV2 + 1);
			Index.push_back(index + 1);
			Index.push_back(index);
		}
	}

	// 이전 방식에서는 같은 위치의 정점이라도 별개로 취급했으므로 각 면마다 노멀을 계산했음, 하지만 정점을 재사용하면 다른 노멀 계산이 필요함
// face normal -> vertex normal
/*1.모든 정점에 매핑 가능한 float3(vec3) 배열을 만든다
2. 원래 가지고 있던 모든 정점의 배열과 각 삼각형을 정의하는 인덱스를 활용해서 노멀 계산
	(인덱스를 3씩 늘리면서 한 번 반복 index, index + 1, index + 2로 이루어진 노멀 계산)
3. 그리고 그렇게 계산한 노멀을 각 계산의 반복(각 삼각형의 반복)마다 해당 3개 점에 대응하는 1.에서 만든 배열의 인덱스에 누산 +=
4. 정점배열[n]에 대응하는 노멀배열[n]이 만들어진다, 정규화 필요*/
	Normal.resize(Position.size(), glm::vec3(0.0f, 0.0f, 0.0f));
	for (int i = 0; i < Index.size(); i += 3) {
		glm::vec3 p0 = Position[Index[i]];
		glm::vec3 p1 = Position[Index[i + 1]];
		glm::vec3 p2 = Position[Index[i + 2]];

		glm::vec3 normal = normalize(cross(glm::vec3(p1 - p0), glm::vec3(p2 - p0)));
		// 삼각형의 면적에 따른 normal에 가중치를 부여할 수 있음
		//glm::vec3 face = cross(p1 - p0, p2 - p0);
		//float area = length(face) * 0.5f;
		//glm::vec3 normal = normalize(face) * area;  == face(면)노멀 x 면적
		Normal[Index[i]] += normal;
		Normal[Index[i + 1]] += normal;
		Normal[Index[i + 2]] += normal;
	}

	for (auto& normal : Normal) {
		if (glm::dot(normal, normal) > 0.0f) normal = glm::normalize(normal);
	}

	Color.resize(Position.size());

	for (int i = 0; i < Position.size(); i++) {
		Color[i] = color;
	}

}