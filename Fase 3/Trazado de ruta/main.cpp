
#define PI 3.14159265
#define START_POINT 360,700
#define END_POINT_1 360,70
#define END_POINT_2 360,360
#define N_PIVOTS 150
#define N_NEIGHBORS 5

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <queue>
#include <time.h>
#include <map>
#include <set>
#include <climits>
#include <utility>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
// Our lib
#include "rutinas.h"

using namespace std;
using namespace cv;

Mat img = imread("imagen_obstaculos.png");
// Mat img = imread("test2.png");
Mat colormat;
// Mat grayImage = imread("test2.png",CV_LOAD_IMAGE_GRAYSCALE);
Mat grayImage = imread("imagen_obstaculos.png",CV_LOAD_IMAGE_GRAYSCALE);
Mat drone = imread("parrot.jpg");

vector<Point> vec_pivots;
// Adyacences matrix
vector< vector<long> > mDistances;



void plotDrone(Point coord){
	int drone_size = 50;
	resize(drone, drone, Size(drone_size, drone_size));
	Mat tempShow = img.clone();
	
	// inside limits, not needed when Dieguin lombrin is done
	if(coord.x -  drone_size/2 > 0 && coord.y - drone_size/2 > 0  && coord.x +  drone_size/2 < img.cols &&  coord.y +  drone_size/2 < img.rows)
		drone.copyTo(tempShow(Rect(coord.x -  drone_size/2,coord.y - drone_size/2 ,drone.cols,drone.rows)));
	
	imshow("Drone location", tempShow);
}


void simulatePath(vector<Point> &vPath){
	for(int i = 0; i < vPath.size(); i++){
		plotDrone(vPath[i]);
		printf("%d--------\n",i );
		char c = 'a';
		while(c != 'p'){
			c = waitKey(100);
		}

	}
	
	destroyWindow("Drone location");
}

void computeRoute(Mat img, Mat binImage, int side_selection){

	// Add INITIAL POINT and FINAL POINT
	vec_pivots.push_back(Point(START_POINT)); circle(img, Point(START_POINT), 10 ,Scalar(200,200,0),CV_FILLED,8,0);
	vec_pivots.push_back(Point(END_POINT_1)); circle(img, Point(END_POINT_1), 10 ,Scalar(200,200,0),CV_FILLED,8,0);

	// pivoting the image
	genPathPivots(img,binImage,N_PIVOTS,vec_pivots,side_selection);

	// creating the graph by joining the pivots, draws the graph and reuturns Ady. Matrix
	mDistances = genGraph(img,binImage,vec_pivots, N_NEIGHBORS);

	// Get the path 
	vector<Point> vPath =  my_dijkstra(mDistances,vec_pivots,0);
	
	int i;
	printf("(%d,%d)", vPath[0].x,vPath[0].y);
	for(i = 1; i < vPath.size(); i++){
		printf("->(%d,%d)", vPath[i].x,vPath[i].y);
		line(img, vPath[i], vPath[i-1],  Scalar(0,255,0),3);
	}

	printf("\n");

	// Uncomment this line to simulate the path
	simulatePath(vPath);
}



int main() {
	Mat binImage;	gray2threshold(grayImage,binImage,240);
	imshow("binImage",binImage);


	// Call this dunction to perform the path from initial point to END_POINT_1 or END_POINT_2
	// Last param could be: GOING_LEFT, GOING_RIGHT, GOING_NORMAL
	computeRoute(img,binImage,GOING_LEFT);

	imshow("pivoted image",img);
	// imshow("gray image",grayImage);
	
	// wait to close program, while loop is cuz my vm has a weird bounce
	char c = waitKey(100);
	while(c != 'q'){
		c = waitKey(100);
	}

	destroyAllWindows();
	
	return 0;
}