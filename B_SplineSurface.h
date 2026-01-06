#pragma once

#include <vector>
#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>

#include <glfw3.h>

struct ControlPoint
{

	std::vector<glm::vec3> Points = 
	{ 
	  glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1 / 3.0f, 0.0f, 0.0f), glm::vec3(2 / 3.0f, 0.0f, 0.0f) , glm::vec3(3 / 3.0f, 0.0f, 0.0f),
	  glm::vec3(0.0f, 0.0f, 1 / 3.0f), glm::vec3(1 / 3.0f, 0.0f, 1 / 3.0f), glm::vec3(2 / 3.0f, 0.0f, 1 / 3.0f) , glm::vec3(3 / 3.0f, 0.0f, 1 / 3.0f),
	  glm::vec3(0.0f, 0.0f, 2 / 3.0f), glm::vec3(1 / 3.0f, 0.0f, 2 / 3.0f), glm::vec3(2 / 3.0f, 0.0f, 2 / 3.0f) , glm::vec3(3 / 3.0f, 0.0f, 2 / 3.0f),
	  glm::vec3(0.0f, 0.0f, 3 / 3.0f), glm::vec3(1 / 3.0f, 0.0f, 3 / 3.0f), glm::vec3(2 / 3.0f, 0.0f, 3 / 3.0f) , glm::vec3(3 / 3.0f, 0.0f, 3 / 3.0f),
	};
	int Rows = 4;
	int Cols = 4;

public:
	glm::vec3& operator()(int Row, int Col) { return Points[Row * Cols + Col]; }

};

class B_SplineSurface
{

private:

	ControlPoint ControlPoints;

	std::vector<float> Height = std::vector<float>(0);
	
	std::vector<float> KnotVectorU;
	std::vector<float> KnotVectorV;
	int U_Degree = 3;
	int V_Degree = 3;


	// Resolution
	int ResU;
	int ResV;



public:

	B_SplineSurface(int ResU, int ResV) : ResU(ResU), ResV(ResV) {};

	void SetResolution(int ResU, int ResV) { this->ResU = ResU; this->ResV = ResV; }
	std::vector<float> initKnotVector(int n, int degree);
	void InitKnotVectors();
	
	void GenerateSurface();

	float BasisFunction(int index, int degree, float t, const std::vector<float>& KnotVector);

	const std::vector<float>& GetHeightMap() const { return Height; }

	const int GetRowsControlPoints() const { return ControlPoints.Rows; }
	const int GetColsControlPoints() const { return ControlPoints.Cols; }

	const glm::vec3& GetControlPoint(int Row, int Col) { return ControlPoints(Row, Col); };
	void SetControlPoint(int Row, int Col, glm::vec3 Point) { ControlPoints(Row, Col) = Point; };


};

