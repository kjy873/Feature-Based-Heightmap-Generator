#include "B_SplineSurface.h"


std::vector<float> B_SplineSurface::initKnotVector(int ControlPointCount, int degree) {
	int length = ControlPointCount + degree + 1;
	std::vector<float> KnotVector(length);

	int segment = ControlPointCount - degree;

	for (int i = 0; i < length; i++) {
		if (i <= degree) {
			KnotVector[i] = 0.0f;
		}
		else if (i >= ControlPointCount + 1) {
			KnotVector[i] = 1.0f;
		}
		else {
			KnotVector[i] = (float)(i - degree) / (float)segment;
		}
	}

	return KnotVector;
}

void B_SplineSurface::InitKnotVectors() {

	KnotVectorU = initKnotVector(ControlPoints.Rows, U_Degree);
	KnotVectorV = initKnotVector(ControlPoints.Cols, V_Degree);

}

void B_SplineSurface::GenerateSurface() {

	InitKnotVectors();

	Height.clear();
	Height.reserve(ResU * ResV);

	std::vector<std::vector<float>> CachedBasisU;
	std::vector<std::vector<float>> CachedBasisV;
	CachedBasisU.resize(ResU, std::vector<float>(ControlPoints.Rows));
	CachedBasisV.resize(ResV, std::vector<float>(ControlPoints.Cols));
	
	for (int i = 0; i < ResU; i++) {
		float u = (float)i / (float)(ResU - 1);
		for (int j = 0; j < ControlPoints.Rows; j++) {
			CachedBasisU[i][j] = BasisFunction(j, U_Degree, u, KnotVectorU);
		}
	}
	
	for (int i = 0; i < ResV; i++) {
		float v = (float)i / (float)(ResV - 1);
		for (int j = 0; j < ControlPoints.Cols; j++) {
			CachedBasisV[i][j] = BasisFunction(j, V_Degree, v, KnotVectorV);
		}
	}


	for (int p = 0; p < ResU; p++) {
		float u = p * 1.0 / (float)(ResU - 1);
		for (int q = 0; q < ResV; q++) {
			float v = q * 1.0 / (float)(ResV - 1);
			glm::vec3 CurrentPoint(0.0f, 0.0f, 0.0f);
			for (int i = 0; i < ControlPoints.Rows; i++) {
				float PointU = CachedBasisU[p][i];
				for (int j = 0; j < ControlPoints.Cols; j++) {
					float PointV = CachedBasisV[q][j];
					CurrentPoint += ControlPoints(i, j) * PointU * PointV;
				}
			}
			Height.push_back(CurrentPoint.y);
		}
	}


}

float B_SplineSurface::BasisFunction(int index, int degree, float t, const std::vector<float>& KnotVector) {
	if (degree == 0) {
		if (t >= KnotVector[index] && t < KnotVector[index + 1])return 1.0f;
		else return 0.0f;
	}

	float left{ 0.0f };
	float right{ 0.0f };

	if (KnotVector[index + degree] != KnotVector[index]) {
		left = (t - KnotVector[index]) / (KnotVector[index + degree] - KnotVector[index]) * BasisFunction(index, degree - 1, t, KnotVector);
	}
	if (KnotVector[index + degree + 1] != KnotVector[index + 1]) {
		right = (KnotVector[index + degree + 1] - t) / (KnotVector[index + degree + 1] - KnotVector[index + 1]) * BasisFunction(index + 1, degree - 1, t, KnotVector);
	}

	if (t == KnotVector.back() && index == KnotVector.size() - degree - 2)
		return 1.0f;

	return left + right;

}

void B_SplineSurface::PrintControlPoints() {
	for (int i = 0; i < ControlPoints.Rows; i++) {
		for (int j = 0; j < ControlPoints.Cols; j++) {
			glm::vec3 pt = ControlPoints(i, j);
			std::cout << "Control Point (" << i << ", " << j << "): " << pt.x << ", " << pt.y << ", " << pt.z << std::endl;
		}
	}
}

void B_SplineSurface::ResetControlPoints() {
	ControlPoints = ControlPoint();
}