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
	std::vector<int> Index;
	std::vector<glm::vec3> Normal;

	int vertices = 0;

	bool IsLine = false;

	bool dirty = false;   // 데이터 변경 여부

	

	int MeshID = -1;

	GLenum DrawMode = 0;

public:

	glm::mat4 TSR = glm::mat4(1.0f);		// transform matrix, 이후 인스턴싱으로 변경하면 없앰

	Mesh() = default;

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
			Index = std::vector<int>{ 0, 2, 1, 0, 3, 2 };
		}

		if (vertices == 8) {
			Position.resize(8);
			Color.resize(8);
			Normal.resize(8);
			Index.resize(36);
			Index = std::vector<int>{// front
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

	const std::vector<glm::vec3>& GetPosition() const { return Position; }
	const std::vector<glm::vec3>& GetCurrentPosition() const { return CurrentPosition; }
	const std::vector<glm::vec3>& GetColor() const { return Color; }
	const std::vector<int>& GetIndex() const { return Index; }
	const std::vector<glm::vec3>& GetNormal() const { return Normal; }
	bool IsDirty() const { return dirty; }

	const glm::mat4& GetTransformMatrix() const { return TSR; }

	const GLenum GetDrawMode() const { return DrawMode; }
	const int GetVertexCount() const { return vertices; }

	void Translate(const glm::vec3& t) { TSR = glm::translate(glm::mat4(1.0f), t); }


	//void SetSurface(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& normals, const glm::vec3& c);

};

class TerrainMesh : public Mesh {
public:
	TerrainMesh(int vertexCount) : Mesh(vertexCount) { DrawMode = GL_TRIANGLES; }

	void SetSurface(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices,
		const std::vector<glm::vec3>& normals, const glm::vec3& c);
	void SetSurfaceNormalized(const std::vector<float>& HeightMap, const int Rows, const int Cols, const glm::vec3& color);

	

};

class CubeMesh : public Mesh {

public:
	CubeMesh(int vertexCount) : Mesh(vertexCount) { DrawMode = GL_TRIANGLES; }

	//void SetHexahedron(std::vector<Mesh>& dst, const glm::vec3 vertices[8], const glm::vec3* c);

	void SetHexahedron(const glm::vec3 center, float half, const glm::vec3& c);

};

class ControlPointVisualMesh : public CubeMesh {

	

public:
	ControlPointVisualMesh(int vertexCount) : CubeMesh(vertexCount) { DrawMode = GL_TRIANGLES; }

	glm::vec3 LinkedPosition;
	int LinkedRow;
	int LinkedCol;
};

class LineMesh : public Mesh {

public:
	LineMesh(int vertexCount) : Mesh(vertexCount) {
		DrawMode = GL_LINE_STRIP;
	}
	void SetLine(const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3* c);
	void SetLines(std::vector<glm::vec3> vertices, const glm::vec3& c);


};

class RectangleMesh : public Mesh {

public:
	RectangleMesh(int vertexCount) : Mesh(vertexCount) { DrawMode = GL_TRIANGLES; }
	void SetRectangle(const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3 vertex3, const glm::vec3 vertex4, const glm::vec3* c);

};
