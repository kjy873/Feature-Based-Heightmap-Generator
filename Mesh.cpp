#include "Mesh.h"


void Mesh::SetLine(const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3* c) {
	for (int i = 0; i < 2; i++) {
		Color[i] = glm::vec3(c[i]);
	}
	Position[0] = glm::vec3(vertex1);
	Position[1] = glm::vec3(vertex2);

	IsLine = true;
}

void Mesh::SetLines(std::vector<glm::vec3> vertices, const glm::vec3& c) {

	Position.clear();
	Position = vertices;
	Color.clear();
	Color.resize(vertices.size());

	for (int i = 0; i < Color.size(); i++) {
		Color[i] = glm::vec3(c);
	}

	IsLine = true;

}

void Mesh::SetRectangle(const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3 vertex3, const glm::vec3 vertex4, const glm::vec3* c) {
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

void Mesh::SetHexahedron(std::vector<Mesh>& dst, const glm::vec3 vertices[8], const glm::vec3* c) {

	Mesh temp(4);

	glm::vec3 FrontColor[4] = { c[0], c[1], c[2], c[3] };
	glm::vec3 TopColor[4] = { c[4], c[5], c[1], c[0] };
	glm::vec3 BackColor[4] = { c[6], c[7], c[5], c[4] };
	glm::vec3 BottomColor[4] = { c[3], c[2], c[6], c[7] };
	glm::vec3 LeftColor[4] = { c[4], c[0], c[3], c[7] };
	glm::vec3 RightColor[4] = { c[1], c[5], c[6], c[2] };

	temp.SetRectangle(vertices[0], vertices[1], vertices[2], vertices[3], FrontColor); // front
	dst.push_back(temp);
	temp.SetRectangle(vertices[4], vertices[5], vertices[1], vertices[0], TopColor); // top
	dst.push_back(temp);
	temp.SetRectangle(vertices[6], vertices[7], vertices[5], vertices[4], BackColor); // back
	dst.push_back(temp);
	temp.SetRectangle(vertices[3], vertices[2], vertices[6], vertices[7], BottomColor); // bottom
	dst.push_back(temp);
	temp.SetRectangle(vertices[4], vertices[0], vertices[3], vertices[7], LeftColor); // left
	dst.push_back(temp);
	temp.SetRectangle(vertices[1], vertices[5], vertices[6], vertices[2], RightColor); // right
	dst.push_back(temp);

}

void Mesh::SetHexahedron(const glm::vec3 center, float half, const glm::vec3& c) {

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

void Mesh::SetSurface(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& normals, const glm::vec3& c) {
	Position = vertices;
	Index = indices;
	Normal = normals;
	Color.resize(vertices.size());

	for (int i = 0; i < vertices.size(); i++) {
		Color[i] = c;
	}
}