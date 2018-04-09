#define X_MAX 640
#define Y_MAX 480

#include <iostream>
#include "rutinas.cpp"

typedef long double ldd;
using namespace std;
using namespace cv;

struct rMoments{
	// ordinary
	ldd m00;
	ldd m10, m01, m11;
	ldd m20, m02;

	// centralized
	ldd u00;
	ldd u10, u01, u11;
	ldd u20, u02;

	// normalized
	ldd n20, n02;
	ldd n11;

	// invariant Hu
	ldd phi1, phi2;

	// Thetha
	ldd theta;
};

/*
	Input:
		Vector to store Points
*/
void generateSeeds(vector<Point>&);


/*
	Inputs:
		Binarized Image
		N number of seeds 

*/
vector<vector<Point> > mycontours(Mat binImage, unsigned int n_seeds);


/*
	Input:
		Mat origin
		uint8 Theshold value
	Output:
		Mat result

*/
void gray2threshold(const Mat&, Mat&, uint8_t );


/*
	Print regions
*/
void printRegions(vector<vector<Point> > vRegions);