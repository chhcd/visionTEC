#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <time.h>
using namespace std;
using namespace cv;

/*

*/
void generateSeeds(vector<Point> &vStrike, unsigned int N_SEEDS){
	// generate random seed
	srand(time(NULL));
	
	for(int i = 0; i < N_SEEDS; i++ ){
		int temp_x = rand()%(X_MAX + 1) + 1;
		int temp_y = rand()%(Y_MAX + 1) + 1; 
		vStrike.push_back(Point(temp_x, temp_y));
	}
}



bool isPosible(int y, int x){
	return (x<X_MAX && x>=0 && y<Y_MAX && y>=0);
}
/*

*/
vector<vector<Point> > mycontours(const Mat binImage, unsigned int n_seeds, Mat &colormat){

	// Vector of Objects
	vector<vector<Point> > vObjects;

	// building the object vector
	vector<Point> tempObject;


	// Color image as helper
	Mat colorImage(binImage.size().height,binImage.size().width,CV_8UC3, Scalar(0,0,0));
	
	// queue to explore
	queue<Point> qPoints;
	
	// vector to store N random seed
	vector<Point> vStrike;
	
	generateSeeds(vStrike,n_seeds);
	//printSeeds(vStrike);

	// strike the image and look for obj
	for (int i = 0; i < n_seeds; i++){
		tempObject.clear();
		qPoints.push(vStrike[i]);

		int colorR = rand()%(255 + 1) + 1;
		int colorG = rand()%(255 + 1) + 1;
		int colorB = rand()%(255 + 1) + 1;

		while(!qPoints.empty()){
			int x = qPoints.front().x;
			int y = qPoints.front().y;
			int val = binImage.at<uint8_t>(y,x);

			// IS REGION
			if(val > 0 && colorImage.at<Vec3b>(y,x)[0] == 0 ){
				// insert points in vector of regions
				tempObject.push_back(Point(x,y));

				// color matrix
				colorImage.at<Vec3b>(y,x)[2] = colorR;
				colorImage.at<Vec3b>(y,x)[1] = colorG;
				colorImage.at<Vec3b>(y,x)[0] = colorB;
				
				if( isPosible(y+1,x))
					if(binImage.at<uint8_t>(y+1,x) && colorImage.at<Vec3b>(y+1,x)[0] == 0 )
						qPoints.push(Point(x,y+1));
				
				if(isPosible(y-1,x))
					if(binImage.at<uint8_t>(y-1,x) && colorImage.at<Vec3b>(y-1,x)[0] == 0 )
						qPoints.push(Point(x,y-1));
							
				if(isPosible(y,x+1))
					if(binImage.at<uint8_t>(y,x+1)   && colorImage.at<Vec3b>(y,x+1)[0] == 0)
					qPoints.push(Point(x+1,y));
				
				if(isPosible(y,x-1))
					if(binImage.at<uint8_t>(y,x-1) && colorImage.at<Vec3b>(y,x-1)[0] == 0 )
						qPoints.push(Point(x-1,y));
				
			}

			
			qPoints.pop();	
		}

		// if found a region list it
		if(tempObject.size() > 50)
			vObjects.push_back(tempObject);

		

	}
	colormat = colorImage;
	// imshow("color mat", colorImage);
	return vObjects;
}

/*

*/
void gray2threshold(const Mat &sourceImage, Mat &binImage, uint8_t threshold_value){
  /* 
  	Last param is type
  	0: Binary
    1: Binary Inverted
    2: Threshold Truncated
    3: Threshold to Zero
    4: Threshold to Zero Inverted
   */

  threshold( sourceImage, binImage, threshold_value, 255,1 );
}

void printRegions(vector<vector<Point> > vRegions){

	for(int i = 0; i < vRegions.size(); i++){
		printf("Objecto # %u \t\t Numero de puntos: %lu\n", i,vRegions[i].size());
		// for(int j = 0; j < vRegions[i].size(); j++ ){
		// 	printf("%u \t %u \n",vRegions[i][j].x,vRegions[i][j].y);
		// }
		printf("\n");
	}
}
