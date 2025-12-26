#pragma once

#include <vector>

#include <glew.h>

#include <glm.hpp>

#include <ext.hpp>
#include <gtc/matrix_transform.hpp>

class Mesh
{
protected:
	std::vector<glm::vec3> Position;
	std::vector<glm::vec3> CurrentPosition;
	std::vector<glm::vec3> Color;
	std::vector<unsigned int> Index;
	std::vector<glm::vec3> Normal;

	int vertices = 0;

	bool IsLine = false;

	bool dirty = false;   // 데이터 변경 여부

	glm::mat4 TSR = glm::mat4(1.0f);		// transform matrix, 이후 인스턴싱으로 변경하면 없앰

	int MeshID = -1;

public:

	Mesh(int vertexCount) : vertices(vertexCount) {
		Position.resize(vertices);
		CurrentPosition.resize(vertices);
		Color.resize(vertices);
		Normal.resize(vertices);

		for (int i = 0; i < vertices; i++) {
			Position[i] = glm::vec3(0.0f, 0.0f, 0.0f);
			CurrentPosition[i] = glm::vec3(0.0f, 0.0f, 0.0f);
			Color[i] = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		if (vertices == 4) {
			Index.resize(6);
			Index = std::vector<unsigned int>{ 0, 2, 1, 0, 3, 2 };
		}

		if (vertices == 8) {
			Position.resize(8);
			Color.resize(8);
			Normal.resize(8);
			Index.resize(36);
			Index = std::vector<unsigned int>{// front
								0, 1, 2,  2, 3, 0,
								// right
								1, 5, 6,  6, 2, 1,
								// back
								5, 4, 7,  7, 6, 5,
								// left
								4, 0, 3,  3, 7, 4,
								// top
								4, 5, 1,  1, 0, 4,
								// bottom
								3, 2, 6,  6, 7, 3 };
		}
	}

	void SetMeshID(int id) { MeshID = id; }
	int GetMeshID() const { return MeshID; }

	//void SetSurface(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& normals, const glm::vec3& c);

};

class TerrainMesh : public Mesh {
public:
	TerrainMesh(int vertexCount) : Mesh(vertexCount) {}

	void SetSurface(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices,
		const std::vector<glm::vec3>& normals, const glm::vec3& c);

};

class HexahedronMesh : public Mesh {

public:
	HexahedronMesh(int vertexCount) : Mesh(vertexCount) {}

	//void SetHexahedron(std::vector<Mesh>& dst, const glm::vec3 vertices[8], const glm::vec3* c);

	void SetHexahedron(const glm::vec3 center, float half, const glm::vec3& c);

};

class LineMesh : public Mesh {

public:
	LineMesh(int vertexCount) : Mesh(vertexCount) {}
	void SetLine(const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3* c);
	void SetLines(std::vector<glm::vec3> vertices, const glm::vec3& c);

};

class RectangleMesh : public Mesh {

public:
	RectangleMesh(int vertexCount) : Mesh(vertexCount) {}
	void SetRectangle(const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3 vertex3, const glm::vec3 vertex4, const glm::vec3* c);

};
