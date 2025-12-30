#pragma once

#include <vector>
#include <algorithm>
#include <iterator>

class HeightMap
{
private:
	std::vector<float> Map;
	int Rows;
	int Cols;

public:

	HeightMap(int Rows, int Cols) : Rows(Rows), Cols(Cols), Map(Rows* Cols) {};

	void SetResolution(int Rows, int Cols);
	int GetResU() const { return Rows; }
	int GetResV() const { return Cols; }

	const std::vector<float>& GetHeightMap() const { return Map; }
	
	float& operator()(int Row, int Col) { return Map[Row * Cols + Col]; }

	void SetHeight(int Row, int Col, float Value) { (*this)(Row, Col) = Value; }
	void SetHeight(const std::vector<float>& Values);
	void SetHeight(const std::vector<std::vector<float>>& Values);

	void AddHeight(int Row, int Col, float Value) { (*this)(Row, Col) += Value; }
	void AddHeight(const std::vector<float>& Values);
	void AddHeight(const std::vector<std::vector<float>>& Values);
};

