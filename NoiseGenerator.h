#pragma once
class NoiseGenerator
{



	int Rows;
	int Cols;

	
public:

	inline float perlinSmooth(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

};

