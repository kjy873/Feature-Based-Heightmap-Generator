#include "HeightMap.h"


void HeightMap::SetHeight(const std::vector<float>& Values) {

	if (Values.size() != Map.size()) {
		printf("Value size != Heightmap Size\n");
		return;
	}

	std::copy(Values.begin(), Values.end(), Map.begin());

}
void HeightMap::SetHeight(const std::vector<std::vector<float>>& Values) {

	if (Values.size() != Rows) {
		printf("Row Size Mismatch\n");
		return;
	}

	for (size_t r = 0; r < Rows; r++) {
		if (Values[r].size() != Cols) {
			printf("Column Size Mismatch\n");
			return;
		}
	}

	for (size_t r = 0; r < Rows; r++) {
		const size_t Start = r * Cols;
		std::copy(Values[r].begin(), Values[r].end(), Map.begin() + Start);
	}

}

void HeightMap::AddHeight(const std::vector<float>& Values) {

	if (Values.size() != Map.size()) {
		printf("Value size != Heightmap Size\n");
		return;
	}

	for (size_t i = 0; i < Map.size(); i++) {
		Map[i] += Values[i];
	}

}
void HeightMap::AddHeight(const std::vector<std::vector<float>>& Values) {

	if (Values.size() != Rows) {
		printf("Row Size Mismatch\n");
		return;
	}

	for (size_t r = 0; r < Rows; r++) {
		if (Values[r].size() != Cols) {
			printf("Column Size Mismatch\n");
			return;
		}
	}

	for (size_t r = 0; r < Rows; ++r) {
		const size_t Start = r * Cols;
		for (size_t c = 0; c < Cols; ++c) {
			Map[Start + c] += Values[r][c];
		}
	}

}